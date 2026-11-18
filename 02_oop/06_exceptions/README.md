# 06. Exception Handling

Exceptions are the only standard C++ mechanism for reporting failure out of a constructor, and one of the few places where the language actively pulls apart the object model: partially-constructed objects, unwinding, resource ownership, and program termination all converge here. This section is less about `try`/`catch` syntax — which is small — and more about the design discipline around it: what your exception hierarchy should look like, which guarantees your operations owe their callers, how `noexcept` changes codegen and contracts, and what else exists besides exceptions (`error_code`, `quick_exit`) for reporting failure when exceptions are the wrong tool.

The examples in this directory are:

1. [`01_basic_exceptions/ex.h`](01_basic_exceptions/ex.h), [`main.cpp`](01_basic_exceptions/main.cpp) — throwing by base type, catching by pointer, catching a standard exception (`std::out_of_range`) thrown by the standard library itself.
2. [`02_simple_exception/triangle.h`](02_simple_exception/triangle.h), [`triangle.cpp`](02_simple_exception/triangle.cpp), [`main.cpp`](02_simple_exception/main.cpp) — a small exception hierarchy nested inside the class that throws it; ordering catch clauses from derived to base; catching a private, unnamed derived type only through its public base.
3. [`03_complex_exception/main.cpp`](03_complex_exception/main.cpp) — exceptions from constructors under multiple/virtual inheritance, private exception inheritance.
4. [`04_controlled_section/controlled.h`](04_controlled_section/controlled.h), [`controlled.cpp`](04_controlled_section/controlled.cpp), [`main.cpp`](04_controlled_section/main.cpp) — the "controlled constructor" (function-try-block) pattern.
5. [`05_noexcept/main.cpp`](05_noexcept/main.cpp) — `noexcept` specifier/operator, conditional `noexcept` exercised on both its true and false branch, a `finally`-style RAII helper, a custom `terminate` handler — including a forked child process that actually hits a `noexcept` violation, to show it is `SIGABRT`/`std::terminate()`, not a catchable exception.
6. [`06_quick_exit/main.cpp`](06_quick_exit/main.cpp) — `quick_exit` / `at_quick_exit` as a deliberately destructor-skipping shutdown path, with two `at_quick_exit` callbacks to show they run in reverse registration order.
7. [`07_system_error/system_err.cpp`](07_system_error/system_err.cpp) — `exception_ptr`/`current_exception` propagation across threads (proving the exception's *original* dynamic type survives the trip, not just `std::exception`), `<system_error>`, a custom HTTP-status `error_category` constructed **and** exercised end to end (`error_code` conversion, `message()`, `default_error_condition()` equivalence with `std::errc`).
8. [`08_expected_unexpected/main.cpp`](08_expected_unexpected/main.cpp) — `std::expected`/`std::unexpected` (C++23): a non-exception error channel, `std::bad_expected_access`, and the `transform`/`and_then`/`or_else` monadic operations exercised on both their success and their error/short-circuit paths.

**Build status of these eight leaves, stated once here:** `01_basic_exceptions` through `05_noexcept` and `07_system_error` build and run as part of this repo's CMake tree. Two remain unbuildable on this specific machine, for reasons that are toolchain gaps rather than bugs in the examples: `06_quick_exit` is standards-conformant C++11 but fails to *compile* on macOS (`error: reference to unresolved using declaration`), because Apple's libc++ declares `quick_exit`/`at_quick_exit` as `using_if_exists` and the platform's own C library doesn't provide them, so the names are declared but never actually resolve; `08_expected_unexpected` is standards-conformant C++23, but this machine's Xcode 14 libc++ doesn't ship `<expected>` at all yet (confirmed: the header itself is missing, not merely some API surface of it), so it could not be compile-verified here even though it has its own `CXX_STANDARD 23` property set correctly. Read all eight for the ideas; on a newer libc++/libstdc++, expect six of them to run, and all eight once a platform ships `quick_exit`/`<expected>`.

`01_basic_exceptions` and `02_simple_exception` were, until recently, excluded from the build for reasons worth recording here even though they're fixed now: `01_basic_exceptions/main.cpp` used to `#include` six headers that either belonged to other leaves, were Windows-SEH-specific content already dropped from this chapter, or were lost entirely — the surviving, in-scope material (built-in-type exceptions, `std::out_of_range`) is what remains, now self-contained in `ex.h`. `02_simple_exception/main.cpp` called into a `triangle` class that had never been checked in; `triangle.h`/`triangle.cpp` reconstruct it from how `main.cpp` already used it.

A useful first map is:

| Facility | Since | Main question |
|---|---:|---|
| `throw` / `try` / `catch` | C++98 | How does control transfer when something goes wrong? |
| exception safety guarantees (basic/strong/nothrow) | C++98 (named later) | What does an operation still owe its caller if it throws? |
| function-try-block constructor | C++98 | How do I observe an exception from the init-list without claiming I can clean up from it? |
| `std::exception_ptr` / `current_exception` / `rethrow_exception` | C++11 | How does an exception cross a thread or a deferred call boundary? |
| `<system_error>` | C++11 | How do I report a system-origin failure without forcing exceptions on every caller? |
| `noexcept` specifier / operator | C++11 | Does this function promise not to throw, and can I ask? |
| `quick_exit` / `at_quick_exit` | C++11 | How do I abandon a process without running destructors, deliberately? |
| `std::uncaught_exceptions()` | C++17 | How many exceptions are currently unwinding, right now? |
| `noexcept` as part of the function type | C++17 | Can two otherwise-identical function types differ only by `noexcept`? |
| `std::exception_ptr_cast` | C++26 | Can I inspect a captured exception's type without rethrowing it? |

---

## 1. Exception basics and hierarchy design

### Throw by value, catch by reference

Catch clauses are matched in order, top to bottom, so derived-to-base ordering matters — a `catch(std::exception&)` placed first will swallow everything beneath it:

```cpp
catch (triangle::wrong_param& w) { /* most specific first */ }
catch (triangle::non_triangle_exception& w) { /* ... */ }
catch (triangle::triangle_exception& w) { /* base, catches all derived too */ }
```

(`02_simple_exception/main.cpp`.) Throw the exception object itself, not a pointer (`throw new int(1)` leaks, and nobody is obliged to delete it), and catch by `const&` to avoid a copy and avoid slicing a derived exception down to its base.

### You can throw anything; don't

`throw int(1); throw double(0.);` is legal C++ (`01_basic_exceptions/main.cpp`), and a catch-all `catch (const void* p)` can intercept anything thrown by pointer. Neither is good practice. Reserve exceptions for exceptional conditions — a failed precondition, an unrecoverable I/O error, a broken invariant — not for routine control flow. Just having `try`/`catch` in a call path costs a small but real amount (roughly 5-10% depending on ABI and optimizer); actually unwinding through it costs orders of magnitude more. Use plain control structures when they suffice.

### A single library-wide base class, derived by design

In a hierarchy of any size, derive every exception type your library throws from one project-specific base (itself derived from `std::exception`), so callers have one catch point:

```cpp
catch (std::exception& e) { /* catches everything you meant to throw */ }
```

`03_complex_exception/main.cpp` demonstrates the inverse trap — **privately** inheriting from `std::exception`:

```cpp
class my_exception : std::exception // private inheritance
{
public:
    my_exception() : std::exception("my_exception") {}
};
```

A `catch (const std::exception&)` cannot bind to a `my_exception` at all here, even though the inheritance relationship exists — private inheritance is not an is-a relationship as far as catch-matching and `what()` access are concerned. Some libraries use this deliberately in reverse: certain internal control-flow types (Boost's `thread_interrupted` is the usual example) are intentionally kept *out* of the `std::exception` hierarchy specifically so a broad `catch (const std::exception&)` or `catch (...)` elsewhere in the program cannot accidentally eat them.

### The constructor-failure problem

An object's lifetime does not begin until its constructor completes; a constructor that exits via an exception never produced an object, so its destructor is never called:

```cpp
class finalizer : public derived1, public derived2
{
public:
    internal _i;
    finalizer() { throw std::runtime_error("Exception from constructor"); }
    ~finalizer() { cout << "~finalizer()\n"; }   // never runs
};
```

(`03_complex_exception/main.cpp`.) What *does* run is destructors for every fully-constructed base and member subobject, in reverse order of construction, which is why this is safe even under multiple and virtual inheritance — `internal::~internal()` and the base destructors still fire; only `finalizer::~finalizer()` is skipped, because `finalizer` itself never finished coming into existence. This is also the reason a thrown exception is the *only* standard way to report constructor failure: there is no returned value and no valid object to query afterward.

### The standard exception hierarchy

Almost everything the standard library itself throws derives from `std::exception`:

* `bad_alloc`, `bad_cast`, `bad_typeid`, `ios_base::failure` derive directly from `exception`.
* `logic_error` derives from `exception`; `domain_error`, `invalid_argument`, `length_error`, and `out_of_range` derive from `logic_error`. These are, in principle, detectable before the program runs, or by validating arguments up front.
* `runtime_error` derives from `exception`; `range_error`, `overflow_error`, and `underflow_error` derive from `runtime_error`. These are everything that can only be discovered at runtime.

Know this hierarchy so you can decide, deliberately, whether your own types should reuse it (so generic `catch (std::exception&)` code keeps working) or stand apart from it (for the rare cases, as above, where you want broad catches to miss them).

---

## 2. Exception safety levels

### "Exception safety" needs a level attached

The phrase by itself is underspecified — the standard and most authors use it loosely. Two useful, distinct ideas:

* **Implementation security**: no resource leak and no undefined behavior results from the throw, regardless of what the object's documented contract says afterward. This is usually the easier bar and RAII gets you there almost for free.
* **Interface security**: the object's own documented public guarantees still hold after the throw — not just "nothing leaked" but "the object is still usable the way its interface promises."

The standard library states three concrete levels for its own operations, from weakest to strongest:

* **Basic guarantee** — invariants are preserved, no resources leak, the object is left in *some* valid (if unspecified) state.
* **Strong guarantee** — the operation either completes fully or has no observable effect at all, as if it were never called.
* **Nothrow guarantee** (`noexcept`) — the operation is guaranteed not to throw.

Treat "basic" as the floor for anything you write; add "strong" and "nothrow" where the cost of providing them is reasonable. Most STL containers actually provide the strong guarantee for most operations, with well-known exceptions: inserting into the middle of a `vector`, inserting multiple elements into an associative container in one call, several `string` operations, and most generic `<algorithm>` operations (the `uninitialized_*` family is the exception to the exception — those are written to be exception-safe on purpose, because they run during container reallocation).

### How to actually build an exception-safe operation

The standard pattern underlying most strong-guarantee STL operations: build the new state fully, *then* discard the old one — never destroy the old representation before the new one exists and is known-good. Combine that with RAII (so a thrown exception midway through construction unwinds through sub-object destructors automatically) and you get the strong guarantee for free in most container-like code. Validate preconditions and throw *before* you start mutating state, not partway through — with one carve-out: inside a constructor, "before mutating state" doesn't quite apply the same way, since the object isn't fully alive yet regardless.

### Requirements this places on element types

A container offering the basic or strong guarantee is implicitly asking something of the types it holds: comparison, equality, and `swap` must not leave an element internally inconsistent; `swap` itself and iterator copy/assignment are expected not to throw at all; predicates, comparators, `char_traits`, locale facets, and most algorithm callables are assumed not to throw either (`qsort`/`bsearch` are the odd exception — they can only throw through a user-supplied callback, since the functions themselves are C). Violate these assumptions and the container's own exception-safety guarantees quietly stop applying to it, even though nothing looks wrong at the call site.

---

## 3. The controlled constructor (function-try-block)

> **A gap this chapter's examples had to fix to actually test both halves.** `controlled`'s constructor originally threw *unconditionally* — meaning a `controlled` object could never actually finish constructing, which meant `controlled_method()` (the other function-try-block this section is about) could never be reached by any program at all. Both now take a `should_fail` parameter (defaulting to `true`, preserving the original throwing behavior for a default-constructed `controlled`), so `04_controlled_section/main.cpp` can exercise, and `assert`, all four combinations: constructor throws, constructor succeeds, method throws (and does *not* propagate — see below), method succeeds.

### Intercepting exceptions from the initializer list

A constructor's member-initializer list runs before the constructor body, and an exception thrown there is otherwise invisible to the constructor itself. A **function-try-block constructor** ("controlled constructor") wraps both:

```cpp
controlled::controlled() try : _vect(1000)
{
    for (int i = 0; i < 1000; i++)
        _vect.push_back(1);

    throw std::runtime_error("constructor exception");
}
catch (const exception& e) {
    cout << e.what() << endl;
}
catch (...) {
    cout << "Unknown exception" << endl;
}
```

(`04_controlled_section/controlled.cpp`, one of the three leaves that actually build and run.) This lets you log or observe what went wrong in construction. It does **not** let you recover the object or safely release its subresources from the handler: by the time you're in that catch block you cannot assume which members finished initializing, so attempting targeted cleanup there is guesswork, not RAII. Treat it as a diagnostic hook only. The standard also makes a `return` statement inside such a handler for a constructor ill-formed — you cannot use it to manufacture a fallback return value; the exception is always rethrown once the handler finishes, implicitly or explicitly.

### Destructors must not let exceptions escape

```cpp
controlled::~controlled() throw() {}
```

The `throw()` here is the old-style empty dynamic exception specification (this code predates `noexcept`; see Standards timeline below) asserting the destructor will not throw. The underlying rule hasn't changed: if a destructor throws while the stack is already unwinding because of another exception, `std::terminate()` is called immediately — there is no way to have two exceptions propagating at once. Practically: don't throw from destructors, or if some internal operation might, catch it inside the destructor and swallow or log it there.

### The pattern generalizes past constructors

`controlled::controlled_method()` in the same file applies the identical function-try-block shape to an ordinary member function — it's a general C++ construct, not constructor-specific — but constructors are where it earns its keep, since they're the one place an object genuinely has no other channel to report failure.

---

## 4. `noexcept`: specifying and querying

### The specifier is a promise, not an enforced contract

`05_noexcept/main.cpp` (builds and runs) shows the gap directly:

```cpp
void check_noexcept2() noexcept
{
    throw std::runtime_error("boom"); // compiles; terminates at runtime
}
```

The compiler does not, in general, verify that a `noexcept` function's body can't throw. If it does throw anyway, `std::terminate()` is called at the point the exception would otherwise propagate out — the stack is not even guaranteed to unwind first. This is a deliberate trade: in exchange for giving up that compile-time guarantee, the compiler can skip generating unwind tables and cleanup paths for the `noexcept` function, which is where the optimization benefit comes from. Mark a function `noexcept` only when you're confident of it, not as decoration.

### Conditional `noexcept` and the `noexcept` operator

The specifier can take a compile-time boolean, and the `noexcept` *operator* answers "does the compiler believe this expression cannot throw":

```cpp
template<typename T>
void copy_function(T& x) noexcept(std::is_trivially_copyable_v<T>)
{
    T tmp(x);
}

template <typename T>
void call_func(std::vector<T>& v) noexcept(noexcept(func(v[0])))
{
    for (auto x : v) func(x);
}
```

(`std::is_trivially_copyable_v` here, not `std::is_pod` — `is_pod` was deprecated in C++20; the direct modern replacement for "is a raw, memcpy-style copy of this type safe and non-throwing" is `is_trivially_copyable`, not the old POD category. `05_noexcept/main.cpp` exercises both branches of this conditional — a trivial type, where it's `noexcept`, and a type holding a `std::string`, where it genuinely isn't — rather than only ever instantiating the function with a type that takes the `true` branch.)

This composition — a function's own `noexcept`-ness depending on whether some expression it uses is itself `noexcept` — is exactly how generic standard-library operations (container `swap`, move constructors, etc.) propagate "doesn't throw" guarantees through template code without the author hand-writing it for every instantiation.

### `noexcept` and templates/PImpl

Generic code is where `noexcept` guarantees get fragile: a template function generally cannot assume that constructing, copying, moving, or passing around its type parameter `T` won't throw, because `T` is supplied by the caller. Common mitigations: accept `T` only by pointer or reference (sidestepping the question), wrap the whole operation in `catch(...)`, translate everything into your own exception type at the boundary, lean on the library's documented nothrow-move guarantees where they exist, or commit to being exception-*transparent* (never catch, just let whatever `T` throws propagate unchanged). The same uncertainty applies to PImpl: a facade class generally doesn't know, and shouldn't assume, what exceptions its hidden implementation can throw.

### `finally`-style cleanup without a language feature

C++ has no `finally` keyword; the idiomatic replacement is RAII with a lambda-holding guard object:

```cpp
template<typename F>
struct final_action
{
    final_action(F f) : clean { f } {}
    ~final_action() { clean(); }
    F clean;
};

template<class F>
final_action<F> finally(F f) { return final_action<F>(f); }
```

used as `auto guard = cpp::finally([&]{ delete[] i; });` — the cleanup runs on every exit path, exception or not, because it's riding on normal destructor semantics, not a dedicated exception feature.

### `set_terminate` and controlling abandonment

```cpp
[[noreturn]] void my_handler() { exit(1); }
terminate_handler old_handler = ::set_terminate(&my_handler);
```

`std::terminate()` is invoked in several specific situations worth knowing by name: no matching handler is found for a thrown exception; a `noexcept` function tries to let an exception escape; a destructor throws while already unwinding; the exception-handling machinery itself fails; a bare `throw;` executes with no exception currently being handled; a static or thread-local object's constructor or destructor throws out of that context; or an `atexit`-registered function throws. The default terminate handler calls `std::abort()`, which does **not** run destructors or flush buffers. Installing your own handler — as above, calling `std::exit()` instead — lets you run global/static destructors on the way out, at the cost of running on a program state that may be arbitrarily broken.

`05_noexcept/main.cpp` proves both halves of that last sentence rather than just asserting them: it `fork()`s a child process specifically to hit the `check_noexcept2()` violation above, once with the *default* terminate handler installed (the child dies from `SIGABRT`, confirmed via `waitpid`/`WIFSIGNALED`) and once with `my_handler` installed (the child now exits cleanly with status `1`, confirmed via `WIFEXITED`/`WEXITSTATUS`). The fork is necessary, not decorative: hitting this violation directly in the test process would terminate the test binary along with it, which is exactly why the rest of this chapter's examples can `assert()` their way through every other exception path but this one needs a disposable child process instead.

---

## 5. `quick_exit` and deliberately skipping cleanup

### A second, more abrupt way to stop

`06_quick_exit/main.cpp` demonstrates `std::quick_exit()` and `std::at_quick_exit()`:

```cpp
void my_cleanup() { std::cout << "Custom at_quick_exit() cleanup\n"; }

std::at_quick_exit(&my_cleanup);
std::quick_exit(0);
```

`quick_exit` sits between `exit()` and `abort()`: like `exit()`, it returns a status code to the OS cleanly, but like `abort()`, it does not run automatic-storage destructors or `atexit`-registered functions. Only callbacks registered specifically via `at_quick_exit` run, **in reverse registration order** — the same LIFO rule as `atexit`. `06_quick_exit/main.cpp` registers two callbacks, not one, specifically to exercise that ordering rather than merely state it; a single callback can't distinguish "runs in registration order" from "runs in reverse order." It exists for situations where normal static-destructor order is itself risky or meaningless (multi-process shutdown, corrupted global state, certain embedded/real-time shutdown paths) but you still want a chance to flush something specific before the process disappears.

### A genuine toolchain gap, not a code defect

This example is standards-conformant C++11 and builds fine on, e.g., recent GCC/Clang-on-Linux or MSVC. On this repo's macOS/Xcode toolchain specifically, it currently fails to *compile* (`error: reference to unresolved using declaration`, not a link error): Apple's libc++ declares `quick_exit`/`at_quick_exit` with the `using_if_exists` attribute because macOS's own C library doesn't actually provide them, so the names are declared in `<cstdlib>` but never resolve to anything real. That is a platform-library gap between the C++ standard and Apple's C runtime, not a defect in the example — worth knowing in general, because "my code didn't build" and "my toolchain doesn't implement this part of the standard yet" are different problems that call for different fixes. Because this leaf can't be compiled here, its ordering claim above is checked against the standard and cppreference, not against an actual run on this machine — noted directly in the file too.

---

## 6. `<system_error>` and dual-channel error reporting

### Propagating an exception across a boundary that isn't a call stack

`07_system_error/system_err.cpp` (builds and runs) shows `std::exception_ptr` carrying a live exception across a boundary where ordinary stack unwinding can't reach — specifically, across threads:

```cpp
std::exception_ptr ex_ptr;
try {
    std::string s;
    const char c = s.at(1); // throws
}
catch (...) {
    ex_ptr = std::current_exception();
}
// ... later, possibly on another thread ...
std::rethrow_exception(ex_ptr);
```

`std::exception_ptr` behaves like a smart pointer to a captured exception object; it keeps the exception alive independent of the handler that caught it. This is exactly the mechanism `std::promise`/`std::future` and `std::packaged_task` use internally: a worker thread that hits an exception calls `promise.set_exception(current_exception())` instead of `set_value(...)`, and the exception resurfaces on `future::get()` on whichever thread is waiting — `show_multithreading_propagation()` and `show_packaged_task_propagation()` in the same file exercise both paths.

### `<system_error>` as the library's answer to OS-origin failures

The standard library's own error reporting is, frankly, inconsistent across facilities — `errno`-style globals, `end()`-as-sentinel, stream state flags, exceptions, and `<system_error>` all coexist in different corners of the library. `<system_error>` is specifically aimed at errors that ultimately come from the operating system or from code outside the standard library's own control:

* **`error_code`** — a pair of (integer value, `error_category`), representing one specific, usually platform-specific, error.
* **`error_category`** — a base class identifying where an error code's values and meanings come from; passed by reference, stored by pointer, never copied — it's meant to be inherited from and singleton-like.
* **`system_error`** — a `runtime_error` that wraps an `error_code`, for when you do want an exception.
* **`error_condition`** — a portable counterpart to `error_code`, for comparisons that should hold regardless of platform.

```cpp
FILE* pf = fopen("myfile.txt", "r");
if (!pf)
    throw std::system_error(EIO, std::system_category());
```

The whole point of having both `error_code` and `error_condition` is that `EEXIST` on POSIX and `ERROR_ALREADY_EXISTS` on Windows are different integers from different categories, but both ought to compare equal to the portable `std::errc::file_exists`:

```cpp
if (ec == std::errc::file_exists) { /* true on either platform */ }
```

— this only works because the library (or you) defined that equivalence via `default_error_condition()`. If you find yourself calling `error_code::value()` directly and branching on the raw integer, you've thrown away the portability `error_condition` exists to provide.

### Defining your own category

`system_err.cpp` builds a small HTTP-status `error_category` as a worked example: define an enum of your codes, derive a class from `std::error_category` overriding `name()` and `message()`, provide a free function returning a reference to a single static instance of it, provide `make_error_code()`, and specialize `std::is_error_code_enum<YourEnum>` to `true_type` so the standard machinery recognizes your enum as error-code-shaped. This is the same registration mechanism the standard library's own `std::errc` uses internally — nothing about it is special-cased for the standard library's own types.

`show_custom_http_error_category()` is the function that actually exercises all of this: a previous version of this file built the entire `http_category_impl`/`http_error` machinery above and never once constructed an `error_code` from it anywhere in `main()` — fully worked-out theory with no code behind it. It now converts `http_error::forbidden` to an `error_code` through the implicit conversion `is_error_code_enum` enables, checks `category().name()`/`message()` against the category's own overrides, and asserts the `default_error_condition()` equivalence with `std::errc::permission_denied` actually holds — plus the negative case, `http_error::gateway_timeout`, which has no explicit mapping and so falls through to `default_error_condition()`'s fallback branch instead of matching any `std::errc`.

### Why have both exceptions and error codes at all

Be pragmatic rather than dogmatic about which to use where. Rough guidance: "logic errors" are the kind that could in principle be caught by testing arguments up front or by better testing — these are reasonable to turn into exceptions or assertions, since they generally indicate a bug. True runtime/environmental failures (a missing file, a dropped connection) are a better fit for `error_code` when the caller is expected to handle them routinely, and for exceptions when they're genuinely exceptional for that call site. It's good practice to have *some* catch-all at a sufficiently high level of the program either way — a `catch (std::exception&)` near `main()`, or an equivalent error-code check — so a failure mode nobody anticipated doesn't silently vanish.

---

## 7. `std::expected`/`std::unexpected`: a value-carrying alternative to both

`error_code` (§6) solves the "I don't want the cost/control-flow of an exception" problem but at the price of a bare integer-plus-category — no payload, and nothing stops a caller from ignoring it entirely. `std::expected<T, E>` (C++23, `<expected>`) is the library's newer answer: a type that holds *either* a `T` on success or an `E` describing the failure, with the compiler forcing the caller to pick one before using it.

### A function that can fail returns `expected<T, E>` instead of throwing

```cpp
enum class parse_error { empty, not_a_number, out_of_range };

std::expected<int, parse_error> parse_int(const std::string& s)
{
    if (s.empty())
        return std::unexpected(parse_error::empty);
    // ... on success:
    return 42;
}
```

`std::unexpected(e)` wraps an error value so it converts into the error state of an `expected<T, E>`; a plain `return value;` becomes the success state. There is no implicit conversion between the two — you cannot accidentally return a `T` where an error was meant, or vice versa, the way an `errno` check can be skipped by mistake. [`08_expected_unexpected/main.cpp`](08_expected_unexpected/main.cpp) builds exactly this `parse_int()`.

### Consuming one: check first, same as `std::optional`

```cpp
auto r = parse_int(s);
if (r.has_value())
    use(r.value());
else
    handle(r.error());

int n = r.value_or(-1); // fallback instead of branching
```

`has_value()`/`value()`/`value_or()` are deliberately the same shape as `std::optional<T>`'s — the difference is that the failure case carries an `E`, not nothing.

### Calling `value()` on an error is the one place this still throws

```cpp
try {
    int v = bad.value();
}
catch (const std::bad_expected_access<parse_error>& e) {
    handle(e.error()); // the error value travels with the exception
}
```

This is a safety net for code that skipped the `has_value()` check, not the intended way to consume an `expected` — reaching for `value()` unconditionally defeats the entire point of using `expected` instead of an exception in the first place.

### `transform`/`and_then`/`or_else` chain fallible steps without manual branching

```cpp
auto doubled = parse_int("21").transform([](int v) { return v * 2; });

auto chained = parse_int("5").and_then([](int v) -> std::expected<int, parse_error> {
    return v == 0 ? std::unexpected(parse_error::out_of_range)
                   : std::expected<int, parse_error>(100 / v);
});

auto recovered = parse_int("nope").or_else([](parse_error) -> std::expected<int, parse_error> {
    return 0; // recover with a default instead of propagating
});
```

`transform` runs only on success and leaves an existing error untouched; `and_then` chains another fallible step, short-circuiting at the first failure exactly like a hand-written sequence of early-return error checks would; `or_else` is the error-path mirror of `and_then`, letting a later step recover from (or translate) an earlier failure. All three compose the way `std::optional`'s monadic operations do — `expected` is, in effect, `optional` with a reason attached to the empty case.

`08_expected_unexpected/main.cpp` asserts both branches of each of the three, not just the success path shown above: `transform` on an already-failed `expected` (the function never runs, the original error passes through unchanged), `and_then` short-circuiting on an already-failed input (so a parse failure can never be misread as, say, "parsed as `0`, then divided by zero"), and `or_else` on an already-successful `expected` (the recovery function never runs). As noted at the top of this file, this leaf's `<expected>` header isn't available on this machine's toolchain, so these assertions are checked by hand against the standard's specified behavior rather than by an actual compiler and test run here.

### Where this fits next to exceptions and `error_code`

`std::expected` is not a replacement for either — it's a third point on the same spectrum. Reach for an exception when failure is genuinely exceptional and you want automatic propagation up an arbitrary number of stack frames (§1). Reach for `error_code` (§6) when you need a lightweight, ABI-stable, ignorable-but-shouldn't-be-ignored signal, often crossing a C-compatible or OS boundary. Reach for `std::expected` when failure is a routine, expected outcome of a specific call — parsing, validation, a lookup that might miss — and you want the type system to force the caller to handle both outcomes without paying for stack unwinding on the failure path.

---

## Rules worth keeping in working memory

The thread running through this section is that an exception is not just a `goto` with extra syntax — it interacts with object lifetime, with what "this operation succeeded" is allowed to mean, and with how a program is allowed to end. Treating `throw`/`catch` as a free-standing language feature, independent of RAII and of the guarantee each operation owes its caller, is where most exception-safety bugs come from.

> An exception is a promise about what state the program is in when control resumes, not merely a mechanism for jumping there.

### Core rules

1. **Use exceptions for exceptional conditions, not control flow** — the overhead and the design cost are both real.
2. **Build on RAII, not on a cleanup step you remember to call** — a constructor-plus-`init()` split adds states to validate and a call that's easy to forget.
3. **Document which exceptions you throw and when** — your exception types are part of your public interface, not an implementation detail.
4. **Never let an exception leave a destructor** — it either terminates the program outright, or masks the exception that was already unwinding.
5. **Default to the basic exception-safety guarantee; reach for strong and nothrow where the cost is reasonable.**

### Pitfalls at a glance

| Pitfall | What happens | Do instead |
|---|---|---|
| Catching base before derived | The derived handler is unreachable; only the base one ever runs | Order `catch` clauses most-derived first |
| Treating `noexcept` as compiler-checked | A throw inside a `noexcept` function calls `std::terminate()` immediately | Mark `noexcept` only where you've actually verified it |
| Cleaning up inside a function-try-block constructor handler | You don't reliably know which subobjects finished initializing | Use it for diagnostics only; let RAII in sub-objects do the releasing |
| Calling `error_code::value()` and branching on the raw integer | Silently platform-specific; breaks on another OS | Compare against a portable `std::error_condition` / `std::errc` instead |
| Privately inheriting from `std::exception` | `catch (const std::exception&)` cannot see it at all | Inherit publicly unless hiding it from broad catches is the explicit goal |

### Guarantee / implementation / application mental model

| Question | Language/library guarantee | Implementation detail | Application responsibility |
|---|---|---|---|
| What happens if this throws mid-operation? | Basic, strong, or nothrow — whichever the operation documents | How the compiler lays out unwind tables and cleanup | Know which guarantee each call you rely on actually provides |
| Does `noexcept` get enforced? | No — violating it calls `std::terminate()`, not a compile error | Codegen omits unwind paths when `noexcept` is asserted | Only assert `noexcept` where you've actually checked it holds |
| How does an OS-origin failure surface? | `<system_error>` gives a portable comparison path via `error_condition` | The raw `error_code` value is platform-specific | Compare against `errc`/`error_condition`, never the raw integer |

### Review checklist

- [ ] Are `catch` clauses ordered most-derived to least-derived?
- [ ] Does every type thrown ultimately derive from `std::exception` (unless deliberately excluded from broad catches)?
- [ ] Can any destructor in this change let an exception escape its own scope?
- [ ] Is every `noexcept` on this change actually verified, not just assumed?
- [ ] Does each new operation document (even just in a comment) which exception-safety guarantee it provides?
- [ ] If this reports OS/environment errors, does it compare against `error_condition`/`errc` rather than a raw platform-specific code?

---

## Diagnostics, useful compiler settings and extensions

### Warnings

| Compiler | Flags | What they reveal |
|---|---|---|
| GCC | `-Wall -Wextra -Wnoexcept` | Functions that could be `noexcept` but aren't, and vice versa |
| Clang | `-Wall -Wextra -Wexceptions` | Exceptions that can't propagate through `noexcept`/`extern "C"` boundaries |
| MSVC | `/W4 /EHsc` and the historical `C4297` | `C4297`: "function assumed not to throw an exception but does" — exactly the `check_noexcept2()` case in `05_noexcept/main.cpp` |

### Sanitizers and runtime checks

| Tool | Setting | Best use |
|---|---|---|
| AddressSanitizer | `-fsanitize=address` | Use-after-free of an object destroyed mid-unwind, double deletes on the exception path |
| UndefinedBehaviorSanitizer | `-fsanitize=undefined` | Catches some, not all, UB reachable only via an exception path |
| `-fno-exceptions` (GCC/Clang) build | Compile the example tree with exceptions disabled | Surfaces exactly which code paths silently assume exceptions are available |

### Semantics-changing options

| Option | Effect | Recommendation |
|---|---|---|
| `/EHs` vs `/EHa` (MSVC) | `/EHa` additionally lets `catch(...)` catch asynchronous structured exceptions (SEH, e.g. access violations); `/EHs` is C++-exceptions-only | Prefer `/EHs` unless you specifically need SEH translation; `/EHa` makes `catch(...)` catch things you likely didn't intend |
| `-fno-rtti` (GCC/Clang) | Disables `dynamic_cast`/`typeid`; exception matching by type still works, as it doesn't depend on RTTI the same way | Know the difference before assuming RTTI-off breaks exception handling |
| `_set_se_translator` (MSVC) | Translates a Windows SEH exception into a real, catchable C++ exception type | Needed only when you must let `catch` clauses see SEH at all; SEH unwinding otherwise does not run C++ destructors |

### Compiler extensions

| Extension | Compilers | Use | Portable approach |
|---|---|---|---|
| `std::exception(const char*)` constructor | GCC, MSVC | A convenience constructor `03_complex_exception/main.cpp`'s `my_exception` used to lean on | Not in the standard (libc++ lacks it) — store the message yourself and override `what()`, as `my_exception` does now |
| SEH (`__try`/`__except`) | MSVC | Catching OS-level hardware/async exceptions | Not portable at all; isolate behind a translator if you need it to interoperate with C++ exceptions |

### Libraries and tooling beyond the standard

| Tool | Purpose | When useful |
|---|---|---|
| clang-tidy (`bugprone-exception-escape`, `misc-noexcept-*`) | Static analysis for exceptions escaping destructors/`noexcept` functions | Catching exactly the class of bug this section is about, automatically |
| Catch2 / GoogleTest `EXPECT_THROW`/`ASSERT_THROW` | Exercising the exception path in tests | Confirming the *documented* exception actually fires, not just that something is thrown |
| Compiler Explorer | Inspect generated unwind tables | Checking whether `noexcept` actually changed codegen the way you expect |

---

## Standards timeline

### C++98/03: the original model

`throw`/`try`/`catch`, the standard exception hierarchy (`exception`, `logic_error`, `runtime_error`, and their descendants), and dynamic exception specifications (`void f() throw(std::bad_alloc);`) were all present from the start. The dynamic-exception-specification mechanism — stating in the signature exactly which exceptions a function may throw — proved to cause more problems (runtime checking cost, `std::unexpected()` semantics, poor composability with templates) than it solved, which sets up everything C++11 and C++17 later did to it.

### C++11: the modern exception-handling vocabulary

C++11 introduced, as a connected set: the `noexcept` specifier and `noexcept` operator (replacing the old dynamic exception specifications, which it simultaneously deprecated); `std::exception_ptr`, `std::current_exception()`, and `std::rethrow_exception()` for carrying a live exception across contexts a normal catch block can't reach (this is also what makes exception propagation through `std::future`/`std::promise` possible); the entire `<system_error>` facility (`error_code`, `error_category`, `error_condition`, `system_error`); and `std::quick_exit`/`std::at_quick_exit` as a third process-termination path alongside `exit()` and `abort()`.

### C++17: `noexcept` becomes part of the type system, old specifications are gone

Two changes land together: the dynamic exception specification `throw()` is redefined to be an exact synonym for `noexcept(true)` (and anything other than an empty one is removed outright), and `noexcept` itself becomes part of a function's *type*, not just an annotation on its declaration — meaning two function pointers differing only in their `noexcept`-ness are genuinely different types, and (as a consequence) you can no longer overload a function on exception specification alone. The same release adds `std::uncaught_exceptions()` (plural, returning a count), deprecating the old `std::uncaught_exception()` (singular, returning a bool) — the plural form is what makes it possible to tell how many unwinds are stacked up, which matters to things like scope-guard libraries that need to know whether they're running during a throw or during ordinary destruction.

### C++20: cleanup of the deprecated path

C++20 removes `std::uncaught_exception()` (singular) outright, finishing what C++17 started — `std::uncaught_exceptions()` is now the only option. The dynamic exception specification for non-empty throw lists, already an error since C++17's redefinition, is gone from the grammar entirely.

### C++23: `std::expected`/`std::unexpected` join the error-reporting toolkit

The exception-handling core itself (`noexcept`, the exception hierarchy, `exception_ptr`, `<system_error>`) was a stable carryover, but `<expected>` (P0323) is a genuinely new facility that belongs in this section: `std::expected<T, E>` and `std::unexpected<E>` give code a third point on the exceptions/`error_code` spectrum — a value-carrying, type-checked "either a result or a reason it failed" that composes with `transform`/`and_then`/`or_else`, without exception-handling overhead on the failure path. §7 and [`08_expected_unexpected/main.cpp`](08_expected_unexpected/main.cpp) cover it. (C++23 also adds `std::stacktrace`, adjacent to error reporting but a separate facility from anything demonstrated in this directory.)

This machine's toolchain (Xcode 14, AppleClang 14) does not ship `<expected>` at all, so `08_expected_unexpected` could not be compile-verified here — see its build-status note above. The `std::expected` API itself has been stable since the paper was adopted, so this is purely a "libc++ hasn't caught up on this specific machine yet" gap, not an open design question.

### C++26

`std::exception_ptr_cast<E>(ptr)` is a confirmed C++26 addition (feature-test macro `__cpp_lib_exception_ptr_cast`): given an `exception_ptr`, it lets you check whether the captured exception matches a given type `E` and get a `std::optional<const E&>` to it — without rethrowing it first. This directly improves on the pattern in `07_system_error/system_err.cpp`, where inspecting a captured exception's concrete type currently requires `rethrow_exception()` followed by a `catch`.

---

## Migration note for Java / Python / C# developers

The `try`/`catch`/`throw` keywords look identical across these languages, which makes the actual differences easy to miss: object lifetime, how strongly the type system tracks "does this throw," and whether exceptions are the *only* error-reporting channel all differ in ways that bite specifically when they're assumed away.

### Where intuition transfers and where it breaks

| Concept | Java | Python | C# | C++ reality |
|---|---|---|---|---|
| Exception object lifetime | GC-managed; lives as long as something references it | GC-managed (refcount + cycle collector) | GC-managed | No GC — the exception object's lifetime is managed by the exception-handling runtime itself; `exception_ptr` exists precisely because there's no GC to keep an exception alive across a thread boundary for you |
| Catching by value vs. reference | Irrelevant — everything is a reference | Irrelevant — everything is a reference | Irrelevant — everything is a reference | Catching by value **slices** a derived exception down to its base type, silently losing the derived `what()`/data; always catch by `const&` |
| Checked vs. unchecked | `checked` exceptions must appear in a `throws` clause and are enforced by the compiler | No checking; any exception can propagate from any call | Unchecked, like C++ | Fully unchecked; the now-removed dynamic exception specification was C++'s closest historical analogue to Java's `throws`, and it was deprecated/removed specifically because it didn't work well |
| "Does this call throw?" as a compiler fact | Only for checked exceptions | Never tracked | Never tracked | `noexcept` is a type-level, ABI-visible promise — but unlike Java's `throws`, violating it is **not** a compile error, it's a runtime call to `std::terminate()` |
| Dual error-reporting channel | Essentially none — exceptions are the one true channel | Essentially none, aside from occasional sentinel returns | Essentially none (`TryParse`-style patterns exist but aren't systematized the way `<system_error>` is) | `<system_error>` coexists deliberately alongside exceptions for OS/environment-origin errors, precisely so a hot path can check an `error_code` without paying exception-handling overhead |

### Common wrong assumptions

Assuming the exception object is safe to hold onto indefinitely the way a Java/C#/Python exception reference is: in C++, the moment you're past the `catch` block the original exception object may be gone unless you've explicitly extended its life with `std::exception_ptr`. Assuming a function without a `throws`-equivalent annotation can't throw: in C++, *silence* about exceptions means nothing — only an explicit `noexcept` is a claim, and even that claim isn't verified. Assuming `noexcept` behaves like Java's `throws` — i.e., that violating it is caught and reportable: it isn't; it's `std::terminate()`, immediately, usually without unwinding. Assuming "the standard library throws when something goes wrong" uniformly: large parts of the ecosystem (anything touching `<system_error>`, much of the filesystem library's non-throwing overloads) deliberately return error codes instead, and mixing the two styles at an API boundary is something C++ expects you to design, not something the language picks for you. None of Java, Python, or C# has a built-in "value-or-reason-it-failed" return type comparable to `std::expected` (§7) — Java/Python/C# developers reaching for it for the first time are not translating a habit, they're picking up a genuinely new tool, closer in spirit to Rust's `Result<T, E>` than to anything idiomatic in those three languages.

### Idiomatic C++ replacement

| Habit from managed languages | Idiomatic C++ |
|---|---|
| `finally { cleanup(); }` | RAII — a destructor, or the `finally`-via-lambda-guard pattern in `05_noexcept/main.cpp` |
| Catch, inspect by type, maybe rethrow | Catch by `const&`, most-derived first; use `std::exception_ptr`/`std::current_exception` to move the exception elsewhere instead of holding a live stack frame open |
| `throws IOException` on a method signature | A comment/doc string stating what you throw and when — it's convention, not compiler-enforced, so write it down |
| A single exception hierarchy for everything | Decide deliberately: derive from `std::exception` for anything meant to be caught broadly; keep control-flow-only signals (rare) outside that hierarchy on purpose |
| "Just catch Exception at the top" | Have one high-level catch-all, yes — but prefer `catch (const std::exception&)` over bare `catch (...)`, since `catch(...)` can also intercept platform SEH or types intentionally excluded from the hierarchy, and gives you no message to log |

---

## Further reading

* [language/exceptions](https://en.cppreference.com/w/cpp/language/exceptions)
* [language/noexcept_spec](https://en.cppreference.com/w/cpp/language/noexcept_spec)
* [language/noexcept](https://en.cppreference.com/w/cpp/language/noexcept)
* [language/throw](https://en.cppreference.com/w/cpp/language/throw)
* [language/function-try-block](https://en.cppreference.com/w/cpp/language/function-try-block)
* [error/exception](https://en.cppreference.com/w/cpp/error/exception)
* [error/exception_ptr](https://en.cppreference.com/w/cpp/error/exception_ptr)
* [error/current_exception](https://en.cppreference.com/w/cpp/error/current_exception)
* [error/uncaught_exception](https://en.cppreference.com/w/cpp/error/uncaught_exception)
* [error/error_code](https://en.cppreference.com/w/cpp/error/error_code)
* [utility/expected](https://en.cppreference.com/w/cpp/utility/expected) — `std::expected`, `std::unexpected`, `std::bad_expected_access`
* [error/error_category](https://en.cppreference.com/w/cpp/error/error_category)
* [error/error_condition](https://en.cppreference.com/w/cpp/error/error_condition)
* [error/system_error](https://en.cppreference.com/w/cpp/error/system_error)
* [error/terminate](https://en.cppreference.com/w/cpp/error/terminate)
* [error/set_terminate](https://en.cppreference.com/w/cpp/error/set_terminate)
* [utility/program/quick_exit](https://en.cppreference.com/w/cpp/utility/program/quick_exit)
* [utility/program/at_quick_exit](https://en.cppreference.com/w/cpp/utility/program/at_quick_exit)
