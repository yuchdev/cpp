# Functions in C++: Advanced Facts, Pitfalls, and Evolution (C++98 → C++20)

> Functions are the fundamental abstraction boundary in C++.
> They combine language rules, ABI conventions, optimizer behavior, and C legacy.

## 1. What a function is (and is not)

* A function is a named callable entity with:
  * a type (`R(Args...)`)
  * linkage
  * calling convention
* A function is not:
  * an object
  * a closure (unless wrapped)
  * a constructor
* Constructors are not functions, as they have no return type and special lookup/dispatch rules.

###  1.1 Function size and performance myths

* Function call cost is negligible compared to:
  * cache misses
  * branches
  * memory access
* Inlining is an optimization hint, not a command
* Readability beats micro-optimization
* Modern compilers inlines aggressively - *even without* `inline`

### 1.2 C inheritance: the foundation

C++ function rules are built on C:

* Call by value
* Pointer decay
* No runtime type information for parameters
* ABI compatibility with C when `extern "C"` is used

```cpp
extern "C" void c_api(int);
```

### 1.3 Key C-inherited facts

* Top-level `const` on parameters is ignored
* Arrays decay to pointers
* Function names decay to function pointers

```cpp
void f(int);
// same function type
void f(const int);
```

### 1.4 Function identity, ABI, and calling conventions

A function is not only defined by its *signature* (`R(Args...)`) but also by its **ABI contract**.

#### 1.4.1 Calling convention is part of the function type (ABI level)

On platforms where calling conventions exist (notably Windows):

| Convention     | Stack cleanup | Typical use     |
| -------------- | ------------- | --------------- |
| `__cdecl`      | caller        | default (C/C++) |
| `__stdcall`    | callee        | WinAPI          |
| `__fastcall`   | registers     | performance     |
| `__vectorcall` | registers     | SIMD-heavy code |

```cpp
void __cdecl f1(int);
void __stdcall f2(int);
```

These are **different functions** at the ABI level.

#### Function pointers must match the calling convention exactly

```cpp
using CdeclFn = void(__cdecl*)(int);
using StdFn   = void(__stdcall*)(int);

void __stdcall win_api(int);

CdeclFn p = win_api; // ❌ undefined behavior (ABI mismatch)
```

> **Key insight**
> Calling convention mismatches compile cleanly on MSVC if not carefully typed - the failure happens at runtime.

#### GCC / Clang note (non-Windows)

On most Unix ABIs (SysV AMD64, AArch64):

* Calling convention is fixed by the ABI
* `__attribute__((cdecl))` etc. usually do nothing
* ABI mismatches are *rare*, but still possible with:

  * vector registers
  * variadic functions
  * `extern "C"`

---

### 1.5 `extern "C"`: more than name mangling

#### What `extern "C"` actually does

```cpp
extern "C" int f(int);
```

Effects:

* disables C++ name mangling
* enforces **C ABI**
* forbids overloading
* changes how parameters are passed (ABI-defined)

#### Subtle but critical facts

* `extern "C"` **does not** mean "callable from C" if:

  * parameter types are not C-compatible
  * exceptions escape
  * references are used

```cpp
extern "C" void f(std::string); // ❌ ABI mismatch, UB
```

#### Exception safety rule

> **Never let a C++ exception cross a C ABI boundary**

```cpp
extern "C" void api() {
    try {
        cpp_code();
    } catch (...) {
        std::abort(); // or translate error
    }
}
```

### 1.6. Compiler-specific details worth knowing

#### GCC / Clang

* aggressively inline across TUs with LTO
* treat `noexcept` as optimizer hint
* diagnose ABI mismatches poorly

#### MSVC

* calling convention mismatches are common footguns
* `/GR-` affects RTTI but not function ABI
* `__declspec(dllexport)` affects function identity

---

## 2. Function types and declarations

### 2.1 Prefix vs the suffix return type

```cpp
std::string to_string(int);
auto to_string(int) -> std::string;
```

Equivalent, but:

* Suffix return type is essential when:

  * return type depends on template parameters
  * using `decltype`

```cpp
template<class T, class U>
auto add(T t, U u) -> decltype(t + u);
```

### 2.2 Function overload

* Functions can be overloaded by
  * number of parameters
  * parameter types
  * `const`-qualification on member functions

* Top-level const ignored in function types
```cpp
void f(int);
void f(const int); // same as above
```

### 2.3 Function types are not objects - but decay constantly

#### 2.3.1 Function-to-pointer decay

```cpp
void f(int);

void (*p)(int) = f;     // decay
auto q = &f;            // explicit pointer
```

But:

```cpp
decltype(f)   // function type: void(int)
decltype(&f)  // pointer type: void(*)(int)
```

#### Why this matters

* `decltype(f)` is **not assignable**
* template deduction differs
* overload resolution differs

```cpp
template<class T>
void g(T);

g(f);   // T = void(*)(int), NOT void(int)
```

---

### 2.4 Function types with qualifiers (`noexcept`)

#### 2.4.1 `noexcept` is part of the function type (since C++17)

```cpp
void f();
void g() noexcept;
```

These are **different types**.

```cpp
using Fn1 = void(*)();
using Fn2 = void(*)() noexcept;

Fn1 a = g; // ❌ ill-formed
Fn2 b = g; // OK
```

#### Template implications

```cpp
template<class F>
void call(F f);

call(f); // F differs based on noexcept
```

#### Overload resolution and `noexcept`

```cpp
void h() noexcept;
void h();

h(); // prefers noexcept overload
```

> **Advanced rule**
> `noexcept` participates in overload resolution, template specialization, and type identity.

---

### 2.5 `noexcept(expr)` and conditional noexcept

```cpp
template<class T>
void swap(T& a, T& b) noexcept(noexcept(T(a)));
```

Key points:

* `noexcept(expr)` is evaluated at compile time
* affects:

  * inlining
  * optimizer assumptions
  * standard library behavior

#### STL consequence

Many containers choose move vs copy based on `noexcept`:

```cpp
std::vector<T> v;
v.push_back(x); // may copy if move is not noexcept
```

> **Real-world impact**
> Incorrect `noexcept` specifications can silently degrade performance.

---

### 2.6 Advanced function rules checklist

* Calling convention is part of the ABI
* `noexcept` is part of the type
* Function names decay to pointers aggressively
* `extern "C"` changes ABI, not semantics
* Member function pointers are not addresses
* Variadic functions are ABI traps
* `constexpr` affects linkage and ODR
* Return type deduction can silently change APIs

---

## 3. `auto` and return type deduction

### 3.1 Evolution

| Standard | Feature               |
|----------|-----------------------|
| C++11    | trailing return types |
| C++14    | return type deduction |
| C++20    | constrained auto      |

```cpp
auto f() { return 42; }        // int
auto g() { return 3.14; }      // double
```

❌ Mixing return types is ill-formed.


### 3.2 Return type deduction corner cases

#### `auto` return type is deduced *per function*, not per return

```cpp
auto f(bool b) {
    if (b) return 1;
    else   return 2;    // OK
}
```

But:

```cpp
auto g(bool b) {
    if (b) return 1;
    else   return 2.0;  // ❌ different deduced types
}
```

#### `decltype(auto)` preserves references

```cpp
int x;

auto f() { return x; }          // returns int
decltype(auto) g() { return x; } // returns int&
```

This distinction is **critical** in forwarding APIs.

---

---

## 4. `constexpr`, `consteval`, `constinit`

### 4.1 `constexpr` functions

* Can run at compile-time or runtime
* Cannot have observable side effects
* May contain loops (since C++14)

```cpp
constexpr int square(int x) { return x*x; }
```

### 4.2 `consteval` (C++20)

* Must be evaluated at compile time
* Runtime call is ill-formed

```cpp
consteval int id(int x) { return x; }
```

### 4.3 `constexpr` vs `inline` vs ODR

#### `constexpr` implies `inline`

```cpp
constexpr int f() { return 42; }
```

Effects:

* implicitly inline
* multiple definitions allowed across TUs
* participates in constant evaluation

#### ODR pitfalls

```cpp
constexpr int f(); // declaration
constexpr int f() { return 1; }
```

OK - but mixing with non-constexpr definitions is UB.

---

---

## 5. Function pointers

### 5.1 Definition

```cpp
using Fn = int(*)(double);
```

Facts:

* Carry no state
* Cannot capture
* Reflect linkage and calling convention
* Can be `noexcept`

```cpp
void f() noexcept;
void (*p)() noexcept = f;
```

❌ `noexcept` and linkage cannot appear in type aliases.

### 5.2 Function pointers vs member function pointers

```cpp
struct X {
    void f();
};

void (X::*pmf)() = &X::f;
```

Facts:

* member function pointers are **not addresses**
* representation is ABI-specific
* may be larger than `void*`
* require object to invoke

```cpp
X x;
(x.*pmf)();
```

#### Why this matters

* cannot cast to `void*`
* cannot interoperate with C
* layout varies across compilers

---

### 5.3 Variadic functions (`...`) - legacy ABI landmines

```cpp
int printf(const char*, ...);
```

Facts:

* no type safety
* default argument promotions apply
* ABI-dependent register passing
* breaks `noexcept` reasoning

#### Never forward variadic arguments manually

```cpp
void f(...) 
{
    g(...); // ❌ impossible
}
```

Use `va_list` correctly or avoid entirely.

---

## 7. Attributes (`[[...]]`)

Attributes annotate functions for tools and optimizers.

> Attributes do not change semantics - they convey intent.

Attributes look like:

```cpp
[[something]]
[[something("optional message")]]
[[namespace::vendor_attr]]
```

They attach *metadata* to declarations/statements. Most commonly they:

- trigger or suppress diagnostics (warnings),
- express intent (fallthrough, nodiscard),
- give optimization hints (likely/unlikely),
- enable ABI/layout optimizations (no_unique_address).

Unlike macros, attributes are part of the C++ grammar and are visible to tooling.

---

### 1. Timeline: what arrived when?

#### C++11
- Attributes introduced (standard syntax), but with very few standardized attributes early on.

#### C++14
- `[[deprecated]]`
- `[[deprecated("message")]]`

#### C++17
- `[[nodiscard]]`
- `[[maybe_unused]]`
- `[[fallthrough]]`

#### C++20
- `[[likely]]`, `[[unlikely]]`
- `[[no_unique_address]]`

> Some compilers also support vendor attributes (e.g. `[[gnu::always_inline]]`) for much longer.

---

### 2. Where attributes can be applied (targets)

Attributes can appear on many "targets":

- functions and function parameters
- types (class/struct/enum)
- variables and data members
- namespaces
- statements (notably `[[fallthrough]]`, `[[likely]]/[[unlikely]]`)
- enumerators

**Important:** placement affects what entity the attribute applies to.

---

### 3. `[[deprecated]]` (C++14)

#### Purpose
Mark an API as obsolete while keeping it available.

#### Example
```cpp
[[deprecated("use new_function() instead")]]
void old_function();
```

#### Notes
- Typically a warning when referenced.
- Can be used on:
  - functions
  - classes/types
  - variables
  - namespaces
  - enum members (compiler support varies)

#### Best practice
- Provide a migration path in the message.
- Keep deprecation warnings actionable and stable.
- Often used with versioning policy:
  - deprecated in v1.2
  - removed in v2.0

---

### 4. `[[nodiscard]]` (C++17)

#### Purpose
Prevent accidentally ignoring important return values.

#### Example
```cpp
[[nodiscard]] Result parse();
parse(); // warning (typically)
```

#### Common uses
- error/result types (`Result`, `Status`, `expected<T, E>`)
- resource handles
- security-sensitive decisions

#### Notes and pitfalls
- `[[nodiscard]]` is *diagnostic intent*; it doesn't enforce runtime checks.
- Don't mark everything nodiscard; you'll create warning fatigue.
- Consider marking **types** nodiscard if *any* function returning that type should be checked.

---

### 5. `[[maybe_unused]]` (C++17)

#### Purpose
Silence "unused" warnings when something is intentionally unused.

#### Example
```cpp
[[maybe_unused]] int debug_value = 42;

auto f = []([[maybe_unused]] int unused_param) { };
```

#### Legitimate uses
- debug-only variables (compiled but not used in release)
- parameters required by interface but unused in an implementation
- conditional compilation where some builds don't use the symbol

#### Pitfall
It can hide real bugs. Prefer refactoring first (remove unused) and apply `[[maybe_unused]]` only when intent is clear.

---

### 6. `[[fallthrough]]` (C++17)

#### Purpose
Make intentional `switch` fallthrough explicit (and keep compilers quiet).

#### Example
```cpp
switch (x) {
  case 1:
    do_something();
    [[fallthrough]];
  case 2:
    do_other();
}
```

#### Notes
- Must be on a statement position (usually as a standalone statement).
- Different compilers have slightly different strictness; keep it simple.

---

### 7. `[[likely]]` / `[[unlikely]]` (C++20)

#### Purpose
Provide branch prediction hints for optimizers.

#### Example
```cpp
if (ok) [[likely]] { fast_path(); }
else [[unlikely]] { slow_path(); }
```

#### Reality check
- The compiler may ignore it.
- It can help in hot loops, but only after profiling.

#### Best practice
- Use sparingly, only in performance-critical code.
- Confirm with benchmarks and generated assembly if you care.

---

### 8. `[[no_unique_address]]` (C++20)

#### Purpose
Allow empty data members to occupy no storage (EBO-like optimization for members).

#### Example
```cpp
struct Empty {};
struct S {
  int x;
  [[no_unique_address]] Empty e;
};
```

#### When it matters
- policy-based design
- allocators and empty functors stored in containers/wrappers
- small value types where every byte matters

#### Notes
- Size/layout results depend on ABI/compiler; measure `sizeof`.

---

### 9. Vendor attributes: `[[gnu::...]]`, `[[clang::...]]`, `[[msvc::...]]`

These are **non-standard** but often useful, e.g.:

- `[[gnu::always_inline]]`
- `[[gnu::cold]]`
- `[[clang::no_sanitize("undefined")]]`
- MSVC equivalents via `[[msvc::...]]` (varies by version)

#### Best practice
Wrap them in feature checks or compiler guards:

```cpp
#if defined(__GNUC__) || defined(__clang__)
  [[gnu::always_inline]] inline int f();
#else
  inline int f();
#endif
```

---

### 10. Portability: `__has_cpp_attribute`

Use feature detection to support older compilers:

```cpp
#if defined(__has_cpp_attribute)
#  if __has_cpp_attribute(nodiscard)
#    define NODISCARD [[nodiscard]]
#  else
#    define NODISCARD
#  endif
#else
#  define NODISCARD
#endif
```

Then:

```cpp
NODISCARD int important();
```

This pattern is common in libraries that support multiple compilers/standards.

---

### 11. Attributes vs other specifiers

Attributes are not the same as:

- `inline`, `constexpr`, `consteval`
- `noexcept`
- `alignas(...)` (alignment specifier)
- `[[...]]` is not a macro: it's in the grammar and parsed by the compiler.

Some attributes have "compiler-attribute equivalents" in older codebases, e.g.
`__attribute__((...))` in GCC/Clang or `__declspec(...)` in MSVC.

---

### 13. Practical rules of thumb

- Use `[[nodiscard]]` on results that must be checked.
- Use `[[deprecated]]` for phased API removal with good messages.
- Use `[[maybe_unused]]` only when intent is obvious and documented.
- Use `[[fallthrough]]` every time fallthrough is intentional.
- Use `[[likely]]/[[unlikely]]` only after profiling confirms value.
- Use `[[no_unique_address]]` in tight value types / policy objects.

If you want, I can add a follow-up file showing:
- `[[nodiscard("message")]]` (supported by many compilers though standardized later),
- richer patterns for `cold/hot` functions (vendor attributes),
- attribute placement corner cases (declarations vs declarators),
- how attributes interact with templates and overload sets.

---

## 8. Static local variables

```cpp
void f() {
    static int x = init();
}
```

Facts:

* Initialized once
* Thread-safe since C++11
* Recursive initialization is undefined behavior

---

## 9. Parameter passing and decay

### Arrays decay to pointers

```cpp
void f(int*);
void f(int[]);
void f(int[1024]);   // all identical
```

* Array size is lost
* Use `std::span<T>` (C++20) or containers instead

---

## 10. Variadic interfaces

### Three approaches

1. Variadic templates (type-safe)
2. `std::initializer_list<T>`
3. C-style `...` (unsafe)

```cpp
void f(std::initializer_list<int>);
```

### Priority rule

> If overload ambiguity exists, `initializer_list` wins.

---

## 11. Default arguments

* Only for trailing parameters
* Bound at call site
* Can be function calls

```cpp
void log(int level = get_default());
```

---

## 13. Inline functions

### Meaning of `inline`

* Allows multiple definitions (ODR)
* Suggests inlining (not guaranteed)

### When `inline` matters

* Header-defined functions
* Template definitions
* Explicitly specifying inline makes sense in two cases:
  * if the definition is in `*.cpp` file
  * in template functions (`std::max`)
  
### When inlining is impossible

* Recursion
* Virtual dispatch
* Taking function address
* `inline` is canceled by recursion, loops, virtual (if a method), and dereferencing.
* If you put the inline function's definition into a `*.cpp` file, and if it is called from another `*.cpp` file, you'll get an "unresolved external" error from the linker
---

## 14. Overload resolution (deep dive)

Resolution order:

1. Exact match
2. Promotions
3. Standard conversions
4. User-defined conversions
5. Ellipsis `...`

### Important facts

* Return type is ignored
* Overload resolution is context-independent
* `const` on parameters affects resolution

---

## 15. Initializer lists `{}`

### Deduction rules

```cpp
f({1,2,3});   // OK if f(initializer_list<int>)
f({});        // error: type unknown
```

### Three models

1. Constructor arguments
2. Aggregate initialization
3. `std::initializer_list`

### Priority

> `initializer_list` overloads are preferred.

---

## 16. C++ Callables & Lambdas

This is a detailed reference on "callables" in C++ and the evolution of lambdas:
- C++98 function objects ("functors")
- early functional adapters and Boost-era lambda libraries
- C++11 lambdas and the key design decisions
- improvements in C++14/17/20 (generic, init-capture, `[*this]`, constexpr, templated lambdas)
- other callable primitives (`std::function`, `std::bind`, `std::invoke`)

---

### 1. What is a "callable" in C++?

#### Definition

A callable is anything that can be *invoked* with `()` (or via `std::invoke`) like:

- free function: `int f(int)`
- function pointer: `int(*)(int)`
- member function pointer: `int (T::*)(int)`
- function object (functor): a type with `operator()`
- lambda closure: compiler-generated functor type
- `std::function<R(Args...)>`: type-erased wrapper
- bind expression: result of `std::bind(...)`

Since C++17, you can test callability with:
- `std::is_invocable_v<F, Args...>`
- `std::invoke(f, args...)` to call uniformly (including member pointers)

#### Functions vs callables: why it matters

Functions:

* have fixed ABI
* no state
* no captures
* ideal for:

  * APIs
  * C boundaries
  * plugin interfaces

Callables (lambdas/functors):

* inlineable
* stateful
* templatable
* zero-cost abstractions

> **Design rule**
> Use **functions** for boundaries.
> Use **callables** for composition.

---

### 2. C++98: Functors and the STL style

#### 2.1 Functor basics

In C++98, you wrote small objects:

```cpp
struct Multiply 
{
  int factor;
  explicit Multiply(int f): factor(f) {}
  int operator()(int x) const { return x * factor; }
};
```

Used in algorithms:

```cpp
std::transform(v.begin(), v.end(), out.begin(), Multiply(2));
```

#### 2.2 Why functors mattered

Compared to function pointers, functors can:
- store state (`factor`)
- be inlined and optimized aggressively
- be templated and customized

STL shipped many functors/adapters in `<functional>` (historically):
- `std::plus<T>`, `std::less<T>`, etc.
- adaptors like `std::bind1st`, `std::bind2nd` (deprecated/removed later)

---

### 3. Pre-C++11: "Lambda" emulation (Boost)

Before real lambdas, libraries tried to make expression templates "feel" like lambdas.

#### 3.1 Boost.Lambda (historical)
Typical style:

```cpp
using namespace boost::lambda;
std::for_each(v.begin(), v.end(), std::cout << _1 << "\n");
```

Pros:
- expressiveness when C++ lacked lambdas

Cons:
- heavy template machinery
- confusing errors
- readability costs

Modern code generally uses real lambdas instead.

(Boost is described here for historical context; the demo code does not depend on it.)

---

### 4. C++11: Lambda arrives

#### 4.1 Basic syntax

```cpp
[capture](params) -> return_type { body }
```

Return type is usually deduced:

```cpp
auto twice = [](int x) { return x * 2; };
```

#### 4.2 Lambdas are closure objects

A lambda is essentially a "syntax sugar" unnamed functor with compiler-generated members for captures.

* Every lambda has a unique, unnamed type
* No two lambdas share a type
* Can be stored using
  * `auto`
  * `std::function<R(Args...)>`

Example:
```cpp
int inc = 5;
auto f = [inc](int x) { return x + inc; };
```

Conceptually similar to:

```cpp
struct __Lambda 
{
  int inc;
  int operator()(int x) const { return x + inc; }
};
```

#### 4.3 Conversion to a function pointer (only if no captures)

```cpp
auto nc = [](int x) { return x + 1; };
int (*fp)(int) = nc; // ok

int a = 1;
auto cap = [a](int x) { return x + a; };
// cannot convert to function pointer (needs object state)
```

---

### 5. Capture lists: the full story

Captures are the main "power" (and danger) of lambdas.

#### 5.1 Explicit captures

- by value: `[x]`
- by reference: `[&x]`

```cpp
int x = 1;
auto v = [x]() { return x; };   // copy
auto r = [&x]() { return x; };  // reference
```

If `x` changes later:
- value-capture stays old
- reference-capture observes new

#### 5.2 Default captures

- `[=]` capture by value (everything used)
- `[&]` capture by reference (everything used)

```cpp
auto f1 = [=]() { return a + b; };
auto f2 = [&]() { return a + b; };
```

#### 5.3 Mixed captures

You can mix default with explicit:

```cpp
auto f = [&, x]() { return x + y; }; // x by value, y by reference
```

#### 5.4 `mutable` lambdas

By default, value-captured members are `const` inside `operator()`.  
`mutable` allows modifying the captured copies:

```cpp
int counter = 0;
auto f = [counter]() mutable { counter++; return counter; };
```

This does not change the outer `counter`.

#### 5.5 Capture pitfalls: lifetimes

Never return a lambda capturing references to locals:

```cpp
std::function<int()> bad()
{
  int x = 1;
  return [&]() { return x; }; // dangling reference after return => UB
}
```

Fix:
```cpp
return [x]() { return x; };
```

* Note that namespace global variables don't need capture.

#### 5.6 `this` capture

`[this]` captures the `this` pointer by value (pointer copy):

```cpp
auto f = [this]() { return member; };
```

* Beware of dangling `this` if the lambda outlives the object.
* Use `[*this]` (C++17) to capture a copy of the object.

---

### 6. C++14: Generic lambdas and init-capture

#### 6.1 Generic lambdas (auto parameters)

```cpp
auto add = [](auto a, auto b) { return a + b; };
```

Useful for:
- generic algorithms
- avoiding templates at call site
- composing functions

#### 6.2 Init-capture (a.k.a. generalized capture)

Allows capturing an *expression* and/or renaming:

```cpp
int x = 7;
auto f = [y = x + 10]() { return y; };
```

Also enables move-capture:

```cpp
auto p = std::make_unique<int>(123);
auto owns = [ptr = std::move(p)] { return *ptr; };
```

This is one of the most important improvements for real-world C++ code.

---

### 7. C++17: `[*this]`, better constexpr use, visitors pattern

#### 7.1 Capturing `this` vs capturing `[*this]` - a copy of the object

`[this]` captures the pointer; the lambda sees current object state:

```cpp
auto f = [this] { return value; };
```

`[*this]` captures a copy of the object at creation time:

```cpp
auto g = [*this] { return value; };
```

This is often used to avoid lifetime issues when lambdas outlive the object.

#### 7.2 Overloaded lambdas (visitor)

A common pattern to create a visitor for `std::variant`:

```cpp
auto visitor = Overloaded
{
  [](int){},
  [](std::string const&){}
};

std::visit(visitor, var);
```

---

### 8. C++20: Templated lambdas and more

#### 8.1 Explicit template parameter list

```cpp
auto id = []<class T>(T v) { return v; };
```

This allows:
- explicit type parameters
- constraints (with concepts, not shown here)
- clearer intent in some generic contexts

#### 8.2 `[[likely]]/[[unlikely]]` and lambdas

These are attributes on statements/branches, not lambda-specific, but often used inside lambdas in hot code.

---

### 9. `std::function`: when you need runtime polymorphism

`std::function<R(Args...)>` is a type-erased wrapper:

- can store any callable with matching signature
- type-erased
- can be empty
- copyable by design

Trade-offs:

- may allocate (small buffer optimization varies)
- adds indirection cost
- loses concrete type (can't inline as well)
- Generally slower than direct lambda calls

Use it when:

- you need to store heterogeneous callables in one container
- you need ABI-stable callback interfaces
- you need to accept "any callable" across translation units cleanly

Prefer templates (`auto`, `F&&`) when:

- performance matters
- you can keep the callable type visible to the compiler

---

### 10. `std::bind`: mostly legacy

`std::bind` arrived in C++, and still can do partial application:

```cpp
auto add10 = std::bind(add_free, 10, _1);
```

But lambdas are usually clearer:

```cpp
auto add10 = [](int x){ return add_free(10, x); };
```

`std::bind` pitfalls:
- placeholder indexing is hard to read
- value vs reference capture surprises (use `std::ref`)
- error messages can be noisy

---

### 11. "Callable design" best practices

- Prefer lambdas for local, readable behavior.
- Prefer named functors when:
  - behavior is reused in multiple places
  - you want explicit types and semantics
  - you need template configuration/policies
- Avoid returning lambdas capturing references to locals.
- Prefer `[*this]` if your lambda may outlive the object.
- Prefer `F&&` templates over `std::function` for performance-critical generic code:
  ```cpp
  template<class F>
  void run(F&& f) { std::forward<F>(f)(); }
  ```
- Use `std::function` when you need type erasure / runtime polymorphism.
- Prefer `std::invoke` for uniform calling of any callable, especially member pointers.

---

### 12. Modern lambdas (C++20+ mindset)

This section summarizes "how people *actually* use lambdas today" in modern C++ (C++20-era), and the extra topics you asked to include.

#### 12.1 Constraining callables with concepts (`std::invocable`, `std::predicate`)

When you accept a callable as a template parameter, concepts let you state requirements explicitly:

```cpp
#include <concepts>
#include <utility>

template <class F, class... Args>
requires std::invocable<F, Args...>
decltype(auto) call(F&& f, Args&&... args) {
  return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
}
```

For algorithm-style parameters that return `bool`, `std::predicate` expresses intent:

```cpp
#include <concepts>
#include <vector>
#include <algorithm>

template <std::predicate<int> Pred>
int count_if_ints(const std::vector<int>& v, Pred p) 
{
  return (int)std::count_if(v.begin(), v.end(), p);
}
```

Practical impact:
- better error messages than "SFINAE explosions"
- clearer APIs (what the callable must do)
- makes generic code safer and more maintainable

---

#### 12.2 Coroutine + lambda interactions (what to know)

Key points:
- a coroutine is a function whose execution can suspend and resume; it has a frame (state).
- lambdas can be coroutines **if** their `operator()` is a coroutine (i.e., it contains `co_await/co_yield/co_return`) and the return type supports coroutines.

Typical patterns:
- lambdas as small helpers *inside* coroutine bodies (common)
- lambdas that **capture** coroutine-related state - be careful about lifetime
- "generator" lambdas returning a coroutine type (possible but more advanced; APIs must expose the promise type / return object)

Rule of thumb:
- keep coroutine-returning lambdas **named** (using `auto f = [](...) -> Task { ... }`) and avoid overly clever inline coroutine lambdas in hot code paths or public APIs, because debugging and error messages can get rough.

---

#### 12.3 Multithreading: capture pitfalls and safe patterns

The most common lambda/thread bugs are **lifetime** and **shared mutability** issues.

##### Pitfall A: capturing references into asynchronous work

```cpp
int x = 123;
std::thread t([&]{ /* uses x */ }); // BAD if x goes out of scope early
```

Safer patterns:
- capture by value when the work may outlive the scope:
  ```cpp
  std::thread t([x]{ /* uses x copy */ });
  ```
- move-capture owning resources:
  ```cpp
  auto p = std::make_unique<Data>();
  std::thread t([ptr = std::move(p)]{ /* owns ptr */ });
  ```

##### Pitfall B: data races on shared state

If multiple threads mutate captured data, you need synchronization (mutex/atomics) or confinement.

Patterns:
- capture `std::shared_ptr<T>` by value to share ownership + protect access with a mutex inside `T`
- capture `std::atomic<T>*` or `std::shared_ptr<std::atomic<T>>` for lock-free counters
- capture an immutable snapshot (copy) to avoid sharing entirely

---

#### 12.4 Performance notes: inlining, `std::function` SBO, and capture sizes

##### Inlining & zero-overhead
- A lambda's closure type is concrete. When passed as a template parameter (`auto` / `F&&`), compilers can inline it aggressively.
- `std::function` is **type-erased**: it usually introduces an indirection, and may block inlining across that boundary.

##### Small Buffer Optimization (SBO)
Many `std::function` implementations store small callables inline (no heap allocation), but:
- the SBO size is implementation-dependent
- larger captures or non-trivially-movable callables may allocate

Practical advice:
- prefer templates for hot-path callables
- use `std::function` for API boundaries, storage, heterogeneous callables, or stable ABI needs
- keep captures small; prefer capturing pointers/references to stable objects over copying huge objects

##### Capturing large objects
If you capture a large `std::vector` by value, you likely copy it. Prefer:
- capture by reference with careful lifetime control, or
- move-capture (`[v = std::move(vec)]`), or
- capture a `shared_ptr` / `unique_ptr` to share/own without copying

---

#### 12.5 Stateful algorithm objects vs lambdas (C++20 ranges style)

Modern code uses both; pick based on clarity and reuse.

##### Lambdas shine when:
- behavior is local to the call site
- capture is simple and obvious
- you want minimal ceremony

Example (ranges-style predicate):
```cpp
auto is_even = [](int x){ return x % 2 == 0; };
```

##### Named function objects shine when:
- you reuse the same behavior in many places
- you want a named type (debugging, profiling, readability)
- you want configuration via constructor/state
- you want to expose it as part of an API

Example:
```cpp
struct IsMultipleOf {
  int k;
  bool operator()(int x) const { return x % k == 0; }
};
```

Ranges often ship function objects for composability and naming (e.g., projections and predicates), while lambdas remain the most ergonomic glue at call sites.

---

#### 12.6 Practical "modern lambda" checklist

- Prefer **init-capture** for ownership (`[p = std::move(p)]`) and renaming.
- Avoid `[&]` defaults in async/multithread contexts; be explicit.
- For object methods, prefer `[*this]` when the callable may outlive `this`.
- Prefer templates (`F&&`) for performance-critical generic APIs.
- Use concepts (`std::invocable`, `std::predicate`) to state callable requirements.
- Keep captures small and lifetimes obvious.

If you want, I can add:
- concepts-based constraints (`std::invocable`, `std::predicate`) and examples,
- coroutine/lambda interactions,
- lambda capture in multithreading pitfalls and patterns,
- performance notes: inlining, SBO in `std::function`, capturing large objects,
- "stateful algorithm objects" vs lambdas in C++20 ranges.

---

## 18. Macros vs functions

### 1. What the preprocessor is (and is not)

The C/C++ preprocessor runs **before compilation** and mainly performs:

- `#include` file inclusion (pure text inclusion)
- `#define` macro definitions
- `#if/#elif/#else/#endif` conditional inclusion/exclusion of text
- some directives like `#line`, `#pragma` (compiler-specific behavior)

It does **not** understand types, scopes, overload resolution, templates, or evaluation rules.  
Macros are expanded as **tokens/text**, not as language constructs.

#### Inspecting preprocessor output

When debugging expansions, view the "preprocessed" output:

- GCC/Clang:
  - `g++ -E file.cpp > out.i`
- MSVC:
  - `/P` (preprocess to a file) or `/E` (to stdout)

This is often the fastest way to answer "what did this macro actually become?"

---

### 2. Predefined macros (standard & common)

Common standard-ish predefined macros:

- `__FILE__`, `__LINE__` – current file/line
- `__DATE__`, `__TIME__` – build time (not runtime)
- `__cplusplus` – C++ mode indicator (e.g., 201703, 202002)

Compiler/platform macros (non-standard but widely used):

- GCC/Clang family: `__GNUC__`, `__clang__`
- MSVC: `_MSC_VER`
- OS: `_WIN32`, `__linux__`, `__APPLE__`

**Rule of thumb:** prefer *feature detection* over "guessing" by OS/compiler.  
Example: `__has_include(<optional>)` instead of `#ifdef __linux__`.

---

### 3. Core macro mechanics

#### 3.1 Object-like macros (replacement constants)

```cpp
#define PI_APPROX 3.14159265358979323846
```

Pros:
- can be used in `#if` conditions (unlike `constexpr`)
- zero dependency

Cons:
- no type, no scope
- can collide with identifiers
- can break tooling (debuggers / refactors)

Modern alternative for regular code:
```cpp
constexpr double pi = 3.14159265358979323846;
```

---

#### 3.2 Function-like macros

```cpp
#define SQUARE(x) ((x) * (x))
```

Pros:
- can accept expressions and "inline" into code

Cons:
- **multiple evaluation** pitfalls (`SQUARE(i++)`)
- operator precedence pitfalls if you forget parentheses
- no overloads, no templates, no type checks

Prefer:
```cpp
template<class T>
constexpr T square(T x) { return x * x; }
```

---

### 4. Classic macro use cases (legacy + still seen)

#### 4.1 Include guards (still valid)

Classic header guard:

```cpp
#ifndef MY_HEADER_H
#define MY_HEADER_H
// ...
#endif
```

Or `#pragma once` (non-standard, but widely supported):

```cpp
#pragma once
```

**Still necessary** in headers.

---

#### 4.2 Conditional compilation

Typical patterns:

- platform:
  ```cpp
  #if defined(_WIN32)
  // ...
  #endif
  ```

- build toggle:
  ```cpp
  #ifndef APP_ENABLE_TRACE
  #define APP_ENABLE_TRACE 0
  #endif
  ```

- debug-only:
  ```cpp
  #ifndef NDEBUG
  // debug code
  #endif
  ```

**Keep conditional blocks small**; too much `#if` makes code untestable.

---

#### 4.3 Debug logging macros

Still one of the best uses of macros: they can capture `__FILE__`, `__LINE__`, and function signature.

Modern-ish pattern:

- Use a macro to capture call-site metadata,
- forward to a *real function* that does formatting/logging.

---

### 5. Macro "power tools": `#` and `###`

#### 5.1 Stringizing (`#`)

Turns tokens into string literals.

```cpp
#define STR(x) #x
```

But if you want macro arguments to expand first, use **2-level** stringizing:

```cpp
#define STR_IMPL(x) #x
#define STR(x) STR_IMPL(x)
```

Then:
- `STR(PI_APPROX)` becomes `"3.14159265358979323846"` (after expansion)

---

#### 5.2 Token pasting (`###`) / concatenation

Joins tokens:

```cpp
#define CAT(a,b) a###b
```

Also commonly written with a 2-level helper to ensure expansion.

Use cases:
- generating unique identifiers (`var_1`, `var_2`, …)
- building platform-specific symbol names
- some metaprogramming tricks

**Beware:** token pasting can produce invalid tokens silently, and errors can be hard to read.

---

### 6. Variadic macros

```cpp
#define LOGF(fmt, ...) /* ... __VA_ARGS__ ... */
```

Use cases:
- printf-style logging
- compile-time controlled verbosity
- automatic metadata injection (`__FILE__`, `__LINE__`)

Pitfalls:
- formatting mismatches are runtime bugs (unless you use compiler format checking)
- can be awkward when `__VA_ARGS__` is empty (workarounds exist)

---

### 7. X-macros: a "good" macro pattern

An **X-macro** keeps a list in one place and reuses it to generate code.

Example list:

```cpp
#define COLOR_LIST(X) \
    X(Red)            \
    X(Green)          \
    X(Blue)
```

Then generate:
- enum values
- string conversion
- tables, registries

Pros:
- "single source of truth"
- less repetition
- keeps data-like lists close to code generation

Cons:
- unfamiliar to many devs
- still macro magic; keep it documented

---

### 8. Macro traps (the classics)

#### 8.1 Multiple evaluation

```cpp
#define MAX(a,b) ((a)>(b)?(a):(b))
MAX(i++, j++); // increments may happen more than once
```

Fix:
- use templates or inline functions

---

#### 8.2 Operator precedence

```cpp
#define ADD(a,b) a + b
ADD(1,2)*3 // expands to 1 + 2*3
```

Fix:
- parentheses, or better, no macro

---

#### 8.3 `if/else` binding bugs

Bad:

```cpp
#define SET_FLAG(flag) if (flag) foo(); else bar();
```

This breaks when used in another `if`.

Correct pattern:

```cpp
#define SET_FLAG(flag) do { if (flag) foo(); else bar(); } while(0)
```

---

#### 8.4 Name capture / collisions

Macros don't respect scopes:

```cpp
#define min(a,b) ...
```

This can break code, especially with Windows headers.

Fix:
- avoid "common word" macro names
- `#undef` after use if unavoidable
- isolate third-party headers
- prefer `constexpr`, `enum class`, functions

---

### 9. Modern, necessary macro use cases (today)

These are cases where macros are still often the right tool:

#### 9.1 Export/import symbols (shared libraries)

```cpp
#if defined(_WIN32)
#  ifdef MYLIB_BUILD
#    define MYLIB_API __declspec(dllexport)
#  else
#    define MYLIB_API __declspec(dllimport)
#  endif
#else
#  define MYLIB_API
#endif
```

#### 9.2 Feature detection helpers

- `__has_include(<header>)` – optional includes
- `__has_cpp_attribute(nodiscard)` – attribute availability
- `__has_builtin(...)` – compiler builtins availability (Clang/GCC-ish)

#### 9.3 Attributes portability

Wrap attributes so the code compiles across compilers:

```cpp
#if __has_cpp_attribute(nodiscard)
#  define NODISCARD [[nodiscard]]
#else
#  define NODISCARD
#endif
```

#### 9.4 Compile-time config points

Small compile-time toggles can be appropriate, especially in embedded, or when you *must* remove code from the binary:

- `-DENABLE_TRACE=1`
- `-DUSE_FAST_MATH=1`

---

### 10. Best practices checklist (battle-tested)

- Prefer **constexpr / inline / templates** over macros for computations.
- Keep macros **local** and **documented**.
- Use **ALL_CAPS** names to make macro usage obvious.
- Avoid redefining language/common identifiers (`min`, `max`, `ERROR`, `small`…).
- Use `do { ... } while(0)` for statement macros.
- Use 2-level helpers for `#` and `###` when expansion must happen first.
- Consider `#undef` after a macro if it's intended for a short scope.
- Don't hide control flow in macros (especially `return`, `goto`, `break`).
- If macros get complex, you're likely building a mini-language - stop and reconsider.

---

### 11. What the companion `.cpp` demonstrates

`preprocessor_macros_demo.cpp` includes demos of:

- predefined macros
- conditional compilation
- object-like macros vs `constexpr`
- function-like macros and the safe statement macro pattern
- stringizing (`#`) and token pasting (`###`)
- variadic macros for logging
- X-macros generating enum + string conversion
- modern feature checks (`__has_include`, `__has_cpp_attribute`)
- classic traps (optional via `-DDEMO_TRAPS=1`)
- safer alternatives with templates/constexpr

Build example:

```bash
g++ -std=c++20 -O2 -Wall -Wextra -pedantic preprocessor_macros_demo.cpp -o demo
./demo
```

Enable trap demos:

```bash
g++ -std=c++20 -DDEMO_TRAPS=1 preprocessor_macros_demo.cpp -o demo
./demo
```

---

### 12. Suggested exercises (quick)

1. Preprocess-only output:
  - Run `-E` and find where `TRACE_LINE` expanded.
2. Turn `APP_ENABLE_TRACE` on:
  - `-DAPP_ENABLE_TRACE=1`
3. Add a new item to the X-macro list and observe enum + to_string updates.
4. Intentionally break `CAT(a,b)` with invalid token paste and inspect the error.

---

If you want, I can extend this with:
- portable `LIKELY/UNLIKELY` macro wrappers,
- "unique name" macros using `__COUNTER__`,
- a clean header-only `LOG()` that routes to a real logger,
- or a "how to safely interact with Windows headers" section.

### Prefer functions unless:

* Conditional compilation
* Compile-time code generation
* Platform detection
* If used, give the macros long, ugly names

---

## 19. Function evolution timeline

| Standard | Features                                         |
|----------|--------------------------------------------------|
| C++98    | Basic functions, pointers                        |
| C++11    | `auto`, lambdas, `constexpr`, variadics          |
| C++14    | return type deduction, init-capture              |
| C++17    | attributes, `constexpr` expanded                 |
| C++20    | `consteval`, concepts, `std::span`, `[[likely]]` |

---

## 20. Migration note for Java / Python / C# developers

In Java, Python, and similar languages, functions are:

* objects
* heap-allocated
* runtime entities
* reflection-friendly
* garbage-collected

In C++, functions are:

* compile-time entities
* ABI-bound
* optimized aggressively
* invisible at runtime
* governed by the abstract machine

Key differences:

* No runtime safety net
* No universal callable object
* No guaranteed type erasure
* No default closures with lifetime tracking

> C++ functions trade safety for absolute control and zero-cost abstraction.

---

## Final guidance

* Prefer clear function signatures
* Minimize overload ambiguity
* Avoid C-style variadics
* Use lambdas for local behavior
* Use `constexpr` and attributes to express intent
* Treat macros as a last resort

