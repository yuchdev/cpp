# 02. Constness Concept in Modern C++

Constness in C++ is not one feature. It is a family of related mechanisms that answer different questions:

* may this object be modified through this expression?
* may this value be used in a constant expression?
* must this function call happen during constant evaluation?
* must this static or thread-local variable be initialized statically?
* is the current invocation being constant-evaluated?
* should a template branch even exist for this specialization?
* does a class expose the same operation differently for mutable, const, lvalue, and rvalue objects?

Confusing those questions is the source of many subtle bugs and misleading rules of thumb.

The examples in this directory are:

1. [`01_runtime_constness/runtime_constness.cpp`](01_runtime_constness/runtime_constness.cpp) — type-level `const`, references, pointers, `mutable`, `const_cast`, shallow constness, and deduction.
2. [`02_compile_time_constness/compile_time_constness.cpp`](02_compile_time_constness/compile_time_constness.cpp) — `constexpr`, `consteval`, `constinit`, literal types, constant-expression contexts, and templates.
3. [`03_constness_and_classes/constness_and_classes.cpp`](03_constness_and_classes/constness_and_classes.cpp) — class interfaces, const member functions, iterator constness, ref-qualified overloads, and logical constness.
4. [`04_const_expr_eval/const_expr_eval.cpp`](04_const_expr_eval/const_expr_eval.cpp) — constant-evaluation mode, immediate functions, `std::is_constant_evaluated()`, and C++23 `if consteval`.

A useful first map is:

| Facility                       | Since | Main question                                                            |
|--------------------------------|------:|--------------------------------------------------------------------------|
| `const`                        | C++98 | Can this object be modified through this type/expression?                |
| `constexpr`                    | C++11 | Can this variable/function participate in constant evaluation?           |
| `if constexpr`                 | C++17 | Should this template branch exist for this specialization?               |
| `consteval`                    | C++20 | Must this function call be constant-evaluated?                           |
| `constinit`                    | C++20 | Must this static/thread-local variable have static initialization?       |
| `std::is_constant_evaluated()` | C++20 | Is this expression currently being evaluated during constant evaluation? |
| `if consteval`                 | C++23 | Select a branch specifically for constant evaluation.                    |

The shortest reliable mental model is:

> `const` is primarily a type-system property. Constant evaluation is an evaluation-mode property.

The rest of the chapter expands that distinction.

---

## 1. Runtime vs. compile-time constness

### `const` is not a promise of compile-time evaluation

A `const` object may be initialized entirely at runtime:

```cpp
int read_from_socket();

const int value = read_from_socket();
```

After initialization, `value` cannot be modified through that name, but its value was not known during translation.

This is therefore wrong as a general rule:

> `const` means compile-time constant.

A better rule is:

> `const` means that the type system does not permit mutation through that cv-qualified access path.

Some `const` integral or enumeration objects initialized with constant expressions can themselves be usable in integral constant-expression contexts:

```cpp
const int n = 4;
static_assert(n * n == 16);
```

But that is a consequence of the initializer satisfying constant-expression rules, not a general property of `const`.

### Constant expression describes an expression, not merely a declaration keyword

A **constant expression** is an expression that satisfies the language rules for compile-time evaluation.

Contexts that require constant expressions include, among others:

```cpp
static_assert(expr);

template<int N>
struct buffer {};

buffer<expr> b;

switch (x) {
case expr:
    break;
}

std::array<int, expr> a;
```

The interesting distinction is:

```cpp
const int a = runtime_value(); // read-only, not a constant expression
constexpr int b = 42;          // must be initialized by a constant expression
```

`const` affects the declared type.

`constexpr` imposes a constant-expression requirement on a variable initializer and enables constant evaluation for functions.

### `constexpr` does not mean “always compile time”

A `constexpr` function may be evaluated either at compile time or runtime:

```cpp
constexpr int square(int x)
{
    return x * x;
}

constexpr int a = square(4); // constant evaluation
static_assert(a == 16);

int n = read_from_socket();
int b = square(n);           // ordinary runtime call
```

The function is *eligible* for constant evaluation when the arguments and evaluation path satisfy the rules.

This distinction is central:

> `constexpr` expands where a function may execute; `consteval` restricts where it may execute.

### `consteval` means immediate function

C++20 introduced `consteval`:

```cpp
consteval int checked_power_of_two(unsigned bit)
{
    if (bit >= 31)
        throw "bit index too large";

    return 1 << bit;
}
```

Every potentially evaluated call that is not inside an immediate-function context must produce a constant expression:

```cpp
constexpr int mask = checked_power_of_two(5); // OK

int bit = read_from_socket();
// int mask2 = checked_power_of_two(bit);      // error
```

A `consteval` function is also a `constexpr` function in the language sense, but it has the stronger immediate-call restriction.

Use `consteval` when runtime fallback would violate the API contract, not merely because compile-time execution might be faster.

### `constinit` is about initialization phase, not immutability

C++20 also introduced `constinit`:

```cpp
constinit int request_count = 0;
```

The object is mutable:

```cpp
++request_count;
```

What `constinit` requires is static initialization for a variable with static or thread storage duration.

It is useful when you want to rule out dynamic initialization:

```cpp
constexpr int initial_capacity()
{
    return 64;
}

constinit int capacity = initial_capacity();
```

This helps avoid initialization-order problems across translation units.

`constinit` does not mean:

* read-only;
* `constexpr`;
* evaluated on every use at compile time;
* no runtime destruction.

It cannot be combined with `constexpr` in the same declaration.

### `std::is_constant_evaluated()` asks about the current evaluation

C++20 added:

```cpp
std::is_constant_evaluated()
```

which reports whether the call occurs while evaluating a manifestly constant-evaluated expression or conversion.

```cpp
constexpr int algorithm(int x)
{
    if (std::is_constant_evaluated())
        return x * x;          // compile-time-friendly path

    return optimized_runtime_square(x);
}
```

The same function may therefore use different implementations at compile time and runtime.

The result is contextual:

```cpp
constexpr int a = algorithm(4); // constant-evaluated path
int b = algorithm(4);           // normally runtime path
```

### C++23 `if consteval` is clearer than testing a boolean

C++23 adds:

```cpp
if consteval {
    // constant-evaluation path
} else {
    // runtime path
}
```

For example:

```cpp
consteval int compile_time_only(int x)
{
    return x * x;
}

constexpr int square_dispatch(int x)
{
    if consteval {
        return compile_time_only(x);
    } else {
        return x * x;
    }
}
```

The `if consteval` branch is an immediate-function context, which makes it especially useful when a `constexpr` API needs to call a `consteval` helper only during constant evaluation.

It is not the same construct as `if constexpr`.

### `if constexpr` answers a template question, not an evaluation-mode question

Since C++17:

```cpp
template<class T>
constexpr auto magnitude(T value)
{
    if constexpr (std::is_signed_v<T>) {
        return value < 0 ? -value : value;
    } else {
        return value;
    }
}
```

The condition is resolved during template instantiation, and the discarded branch is not instantiated in the ordinary way.

By contrast:

```cpp
if consteval
```

asks whether the current invocation is being constant-evaluated.

A useful comparison is:

| Construct                      | Question                                              |
|--------------------------------|-------------------------------------------------------|
| `if`                           | Which branch executes at runtime/constant evaluation? |
| `if constexpr`                 | Which branch belongs to this template specialization? |
| `if consteval`                 | Is this invocation being constant-evaluated?          |
| `std::is_constant_evaluated()` | Boolean query of the current evaluation context       |

---

## 2. Type-level constness

### Top-level and low-level const

The position of `const` matters:

```cpp
int value = 0;

const int* p1 = &value;       // pointer to const int
int* const p2 = &value;       // const pointer to int
const int* const p3 = &value; // const pointer to const int
```

For `p1`:

```cpp
// *p1 = 1; // error
p1 = nullptr; // OK
```

For `p2`:

```cpp
*p2 = 1;      // OK
// p2 = nullptr; // error
```

A useful vocabulary is:

* **top-level const** qualifies the object itself;
* **low-level const** appears through an indirection, such as the pointed-to type.

This distinction drives template deduction, overload resolution, and conversion rules.

### East const and west const mean the same thing

These are identical types:

```cpp
const int* p;
int const* p;
```

Some codebases prefer “east const”:

```cpp
int const* p;
int* const p2 = ...;
```

because reading declarators from the identifier outward can make pointer constness easier to parse.

The language makes no semantic distinction.

### References preserve pointee constness

A reference itself cannot be cv-qualified in the same way an object can:

```cpp
int x = 1;
int& r = x;

// int& const bad = x; // ill-formed
```

The meaningful distinction is the referred-to type:

```cpp
int& r1 = x;
const int& r2 = x;
```

`r2` prevents mutation through that reference:

```cpp
// r2 = 3; // error
```

but another non-const alias may still mutate the object.

### `const T&` can bind to temporaries

```cpp
const std::string& text = std::string("hello");
```

The temporary's lifetime is extended to the lifetime of `text`.

But lifetime extension is contextual and does not magically propagate through arbitrary APIs.

This is still wrong:

```cpp
const std::string& bad()
{
    return std::string("temporary");
}
```

The returned reference dangles.

Constness and lifetime are separate properties.

### A const view can still dangle

This is a classic example:

```cpp
std::string_view bad()
{
    return std::string("temporary");
}
```

`string_view` exposes characters as read-only through its interface, but it owns nothing.

Likewise:

```cpp
std::span<const T>
const T*
const T&
```

can all refer to storage whose lifetime has ended.

“Cannot mutate” does not mean “keeps alive”.

### `auto` drops top-level const when deducing by value

```cpp
const int value = 42;

auto a = value;
static_assert(std::is_same_v<decltype(a), int>);
```

The new object `a` is independent, so the source object's top-level constness is not copied.

Reference deduction preserves it:

```cpp
auto& b = value;
static_assert(std::is_same_v<decltype(b), const int&>);
```

Similarly:

```cpp
auto&& c = value;
static_assert(std::is_same_v<decltype(c), const int&>);
```

because forwarding-reference deduction and reference collapsing preserve the lvalue's cv-qualification.

### Template by-value deduction drops top-level const

Given:

```cpp
template<class T>
void f(T value);
```

and:

```cpp
const int x = 42;
f(x);
```

`T` deduces as `int`, not `const int`.

For:

```cpp
template<class T>
void g(T& value);
```

`T` deduces as `const int` when called with `x`.

This is why forwarding/reference APIs preserve cv-ref information that by-value APIs intentionally discard.

### `decltype` follows different rules from `auto`

```cpp
const int x = 42;

decltype(x) a = x;   // const int
decltype((x)) b = x; // const int&
```

The unparenthesized id-expression form reports the declared type.

The parenthesized expression is analyzed by value category, so an lvalue expression produces an lvalue reference type.

This difference is foundational in generic libraries.

### `std::as_const` creates a const access path

C++17 provides:

```cpp
std::as_const(object)
```

which gives a const lvalue reference without copying:

```cpp
std::vector<int> values{1, 2, 3};

auto& cv = std::as_const(values);
static_assert(
    std::is_same_v<decltype(cv), const std::vector<int>&>
);
```

It is useful for intentionally selecting a const overload.

There is no rvalue overload because returning a const reference to a temporary this way would encourage dangling references.

### `const_cast` removes qualification, not physical read-only storage

This can be legal:

```cpp
int x = 1;
const int* p = &x;

int* q = const_cast<int*>(p);
*q = 2; // OK: original object is non-const
```

This is undefined behavior:

```cpp
const int x = 1;
const int* p = &x;

int* q = const_cast<int*>(p);
// *q = 2; // UB
```

The key question is the dynamic object's actual constness, not merely the current access path.

`const_cast` can also remove `volatile`, but that does not make hardware or concurrency semantics disappear.

### `const` is shallow

```cpp
int value = 1;

const std::vector<int*> pointers{&value};
*pointers[0] = 2; // legal
```

The vector cannot replace its element pointer through a const interface, but the pointed-to `int` is still mutable.

Similarly:

```cpp
const std::shared_ptr<T> owner = ...;
owner->mutate(); // may be legal
```

The smart pointer object is const; the managed object is not automatically const.

Deep immutability must be represented deliberately, for example with:

```cpp
std::shared_ptr<const T>
std::vector<const T*> // where appropriate
```

or with an API whose object graph does not expose mutation.

---

## 3. Constness in classes

### A const member function changes the type of `*this`

Given:

```cpp
class counter
{
public:
    int value() const
    {
        return value_;
    }

private:
    int value_{};
};
```

inside `value() const`, the object is accessed as const.

Conceptually, `this` behaves as a pointer to const `counter`:

```cpp
counter const*
```

Therefore the function cannot ordinarily mutate non-`mutable` data members or call non-const member functions.

The important point is that trailing `const` qualifies the implicit object parameter, not the return value.

### Const and non-const overloads form different member functions

A common accessor pair is:

```cpp
T& at(std::size_t i)
{
    return data_[i];
}

const T& at(std::size_t i) const
{
    return data_[i];
}
```

Calls through mutable objects select the mutable overload.

Calls through const objects select the const overload.

This is the basis of const-correct container interfaces.

### Ref qualifiers add another axis

Since C++11:

```cpp
T& value() &;
const T& value() const &;
T value() &&;
```

This distinguishes:

* mutable lvalue object;
* const lvalue object;
* temporary/rvalue object.

Returning by value from the rvalue overload can avoid dangling references:

```cpp
class name
{
public:
    const std::string& text() const & { return text_; }
    std::string text() && { return std::move(text_); }

private:
    std::string text_;
};
```

This is a powerful interaction between constness, value categories, and lifetime.

### C++23 explicit object parameters can deduce constness

C++23 explicit object parameters (“deducing `this`”) can replace families of cv/ref overloads in supported compilers:

```cpp
struct box
{
    int value_;

    template<class Self>
    decltype(auto) value(this Self&& self)
    {
        return std::forward_like<Self>(self.value_);
    }
};
```

Now the return type follows the cv/ref category of the object:

```cpp
box b{1};
const box cb{2};

b.value();            // int&
cb.value();           // const int&
std::move(b).value(); // int&&
```

This is one of the most important C++23 improvements for writing const-correct generic class interfaces without repeating four overloads.

### `mutable` supports logical constness

A const member function cannot modify ordinary members, but `mutable` members are exempt:

```cpp
class lazy_value
{
public:
    int get() const
    {
        if (!cached_) {
            cache_ = calculate();
            cached_ = true;
        }
        return cache_;
    }

private:
    int calculate() const;

    mutable int cache_{};
    mutable bool cached_{};
};
```

This supports **logical constness**:

> the externally observable logical value is unchanged even though implementation state changes.

Typical uses include:

* lazy caches;
* memoization;
* statistics/debug counters;
* synchronization primitives.

### `mutable` does not make const operations thread-safe

This is dangerous:

```cpp
int get() const
{
    if (!cached_) {
        cache_ = calculate();
        cached_ = true;
    }
    return cache_;
}
```

when multiple threads can call `get()` concurrently.

`mutable` merely permits modification through a const member function.

It provides no atomicity, locking, or happens-before relationship.

A thread-safe logically const cache may need:

```cpp
mutable std::mutex mutex_;
```

or atomics/other synchronization.

### Avoid `const_cast<this>` when `mutable` expresses the invariant

An implementation may technically cast away constness if the underlying object is not truly const:

```cpp
auto* self = const_cast<widget*>(this);
```

But using this to mutate cache state is usually inferior to declaring the cache `mutable`.

More importantly, a const member function may be called on a genuinely const object:

```cpp
const widget w;
w.inspect();
```

Casting away const and modifying a non-`mutable` subobject of that truly const object is undefined behavior.

### Constness participates in virtual overriding

These do not override each other:

```cpp
struct base
{
    virtual int value() const = 0;
};

struct derived : base
{
    int value() override; // error: missing const
};
```

The cv/ref qualification of a non-static member function is part of the member-function type/signature relationship relevant to overriding.

Use `override` so the compiler catches accidental mismatches.

### Static members do not have object constness

A static member function has no `this` pointer:

```cpp
struct S
{
    static int f();
};
```

Therefore trailing `const` is not meaningful:

```cpp
// static int f() const; // ill-formed
```

Constness for static data members is ordinary variable constness:

```cpp
struct constants
{
    static constexpr int width = 32;
};
```

Since C++17, a `constexpr` static data member is implicitly inline, which makes in-class definitions much easier to use from headers.

### Const member functions can still mutate external state

```cpp
struct handle
{
    int* p{};

    void increment() const
    {
        ++*p;
    }
};
```

The member `p` itself is not reseated through the const object, but the pointee is not a subobject of `handle`.

This illustrates why C++ constness is shallow and interface-oriented.

---

## 4. `constexpr`: values, functions, and classes

### A `constexpr` variable is implicitly const

```cpp
constexpr int answer = 42;

static_assert(std::is_const_v<decltype(answer)>);
```

A `constexpr` variable must be initialized in a way that satisfies constant-expression requirements.

Unlike ordinary `const`:

```cpp
const int x = read_from_socket();     // OK
// constexpr int y = read_from_socket(); // error
```

### A constexpr pointer is a const pointer, not necessarily a pointer to const

```cpp
static int value = 1;

constexpr int* p = &value;
```

The pointer object itself cannot be reseated:

```cpp
// p = nullptr; // error
```

but its pointee is mutable:

```cpp
*p = 2; // legal
```

This is analogous to:

```cpp
int* const p2 = &value;
```

not:

```cpp
const int* p3 = &value;
```

### `constexpr` functions are implicitly inline

A `constexpr` function is implicitly inline, which makes definitions in headers natural:

```cpp
constexpr int square(int x)
{
    return x * x;
}
```

This is especially important for templates and header-only libraries.

Do not interpret “inline” as “the compiler must substitute the function body”. The language meaning is primarily ODR/linkage-related.

### C++14 made `constexpr` functions practical

C++11 `constexpr` function bodies were severely restricted.

C++14 relaxed them to allow ordinary constructs such as:

```cpp
constexpr int factorial(int n)
{
    int result = 1;

    for (int i = 2; i <= n; ++i)
        result *= i;

    return result;
}
```

Modern `constexpr` code can often be written in ordinary imperative C++ rather than template-recursive metaprogramming.

### C++20 expanded constant evaluation dramatically

C++20 made many more operations possible during constant evaluation, including substantial library support and controlled dynamic allocation.

For example, modern standard-library containers gained important constexpr capabilities.

The important restriction is conceptual:

> storage allocated during constant evaluation generally cannot simply escape that constant evaluation as runtime-owned dynamic storage.

Using temporary dynamic storage while computing a constant result is useful:

```cpp
constexpr int compute()
{
    // C++20-era constexpr-capable library code may allocate transiently.
    // The constant evaluation still has to satisfy lifetime/allocation rules.
    return 42;
}
```

This is why “constexpr supports allocation” does not mean “the compiler can persist arbitrary heap objects into the executable”.

### Literal types and constexpr objects

A class can participate in constant evaluation when its type and operations satisfy the relevant rules:

```cpp
struct point
{
    double x;
    double y;

    constexpr point(double x_, double y_)
        : x{x_}, y{y_}
    {}

    constexpr double norm2() const
    {
        return x * x + y * y;
    }
};

constexpr point p{3.0, 4.0};
static_assert(p.norm2() == 25.0);
```

The important observation is that class-level constness and constant evaluation compose:

* the object may be `constexpr`;
* member functions may be `constexpr`;
* member functions may also be trailing-`const`;
* these properties answer different questions.

### A constexpr member function is not necessarily const

Since C++14:

```cpp
struct counter
{
    int value{};

    constexpr void increment()
    {
        ++value;
    }
};
```

A non-const `constexpr` member function is valid.

It can mutate an object during constant evaluation provided the object and operation satisfy constant-expression rules.

Do not infer trailing `const` from the `constexpr` keyword.

### Constructors, destructors, and virtual functions evolved

Modern C++ gradually expanded which class operations may be constexpr.

C++20 notably broadened constexpr support, including constexpr destructors and virtual functions under the applicable restrictions.

The practical lesson is:

> do not apply an old C++11-era checklist to C++20/23 constexpr classes.

When portability across language modes matters, use feature-test macros and test the actual minimum standard your project supports.

### C++23 relaxed constexpr function restrictions further

C++23 permits more constructs to appear syntactically inside `constexpr` functions, including constructs that may be unusable on a particular constant-evaluated path.

The design direction is important:

> declaring a function `constexpr` increasingly means “this function may have valid constant-evaluated executions”, not “every statement in its body is universally constexpr-friendly”.

For example, a runtime-only branch can contain operations that are not valid during constant evaluation, as long as a constant-evaluated invocation does not execute them.

### C++23 permits static constexpr locals in constexpr functions

C++23 allows useful patterns such as constant local lookup tables:

```cpp
constexpr int lookup(unsigned index)
{
    static constexpr int table[] = {10, 20, 30, 40};
    return table[index];
}
```

subject to the C++23 constant-expression rules and compiler support.

This reduces pressure to move implementation-only lookup data to namespace scope.

---

## 5. `consteval`: immediate functions

### Use `consteval` for compile-time-only APIs

Good candidates include:

* compile-time parsers;
* validation of literals;
* generated IDs/hashes;
* fixed schema calculations;
* compile-time format descriptors;
* building non-type template arguments;
* rejecting invalid compile-time configuration.

Example:

```cpp
consteval unsigned port(unsigned value)
{
    if (value == 0 || value > 65535)
        throw "invalid port";

    return value;
}

constexpr auto http = port(80);
```

The `throw` is not intended to execute successfully during constant evaluation; it makes invalid calls fail to form a constant expression.

### `consteval` parameters are not magically constant-expression variables

This is subtle.

Inside:

```cpp
consteval int f(int x)
{
    return x;
}
```

calls to `f` must be constant-evaluated, but the parameter name `x` is still a function parameter.

Do not assume it can be used everywhere a compile-time syntactic constant is required inside the function definition.

For example, patterns such as trying to use `x` directly as a non-type template argument inside the immediate function can fail even though callers must supply constant-evaluable arguments.

Immediate invocation and “this local identifier is itself a constant-expression entity” are not the same rule.

### Immediate functions are still ordinary typed functions

They have parameter types, overload resolution, templates, return types, and normal C++ type checking.

For example:

```cpp
consteval std::size_t checked_size(int n)
{
    if (n < 0)
        throw "negative";

    return static_cast<std::size_t>(n);
}
```

The compile-time nature of the function does not bypass conversions, narrowing rules, or object lifetime rules.

### Taking the address of an immediate function is restricted in practice

Immediate functions have function identities, but a pointer/reference to an immediate function cannot simply escape constant evaluation and become a runtime callback in the ordinary way.

That would contradict the “calls must be immediate” contract.

Treat `consteval` APIs as compile-time computation interfaces, not runtime function objects.

### C++23 strengthened immediate-function propagation

C++23 incorporates updated immediate-function propagation rules.

This matters when immediate invocations appear in templated or constexpr code: the language is stricter and more systematic about when compile-time-only requirements propagate through an enclosing function context.

For portable C++20/23 code, prefer straightforward structures and use the `__cpp_consteval` feature-test macro when behavior depends on the newer rules.

### C++23 `if consteval` creates an immediate-function context

This is one of the most useful additions:

```cpp
consteval int compile_time_impl(int x)
{
    return x * 2;
}

constexpr int twice(int x)
{
    if consteval {
        return compile_time_impl(x);
    } else {
        return x * 2;
    }
}
```

Without `if consteval`, mixing a runtime-capable `constexpr` wrapper with an immediate helper is more awkward.

The construct says precisely:

> this branch is specifically for immediate/constant evaluation.

---

## 6. `constinit`: static initialization without constness

### `constinit` applies only to static or thread storage duration

Typical forms are:

```cpp
constinit int global_counter = 0;

thread_local constinit int per_thread_counter = 0;

void f()
{
    static constinit int local_static = 0;
}
```

It does not apply to an ordinary automatic local variable:

```cpp
void f()
{
    // constinit int x = 0; // ill-formed
}
```

### `constinit` prevents dynamic initialization

For a non-local static object, initialization broadly divides into:

* static initialization;
* dynamic initialization.

`constinit` requires the declaration to remain in the static-initialization category.

This is useful for avoiding the static initialization order fiasco when one global depends on another.

### `constinit` does not imply `const`

```cpp
constinit int calls = 0;

void record_call()
{
    ++calls;
}
```

This is the entire point of the facility: require safe early initialization without making later mutation illegal.

You can combine it with ordinary `const`:

```cpp
constinit const int table_version = 3;
```

if both properties are desired.

### `constinit` and `constexpr` cannot be combined

This is ill-formed:

```cpp
// constinit constexpr int x = 42;
```

The facilities overlap but have different semantics.

A `constexpr` object is const-qualified and must satisfy constexpr-variable rules.

A `constinit` object need not be const and need not satisfy constant-destruction properties required of constexpr objects.

### `constinit` can be useful with types unsuitable for constexpr variables

A type may support constant initialization but not satisfy all requirements for a `constexpr` variable.

That is an important reason `constinit` exists independently rather than merely as an alias for `constexpr`.

### Thread-local declarations can benefit from `constinit`

For a `thread_local` object, `constinit` can make it possible for implementations to avoid some runtime initialization guard machinery because the program asserts that static initialization is sufficient.

This is a niche but real systems-level use.

---

## 7. `std::is_constant_evaluated()` and C++23 `if consteval`

### The function reports manifest constant evaluation

```cpp
constexpr int mode()
{
    return std::is_constant_evaluated() ? 1 : 0;
}
```

Then:

```cpp
constexpr int compile_time = mode();
static_assert(compile_time == 1);

int runtime = mode(); // normally 0
```

The question is not:

> did the optimizer happen to fold this call?

The question is:

> is the language currently evaluating this expression in a manifestly constant-evaluated context?

Optimization and constant evaluation are different concepts.

### Never use it to infer optimizer behavior

A compiler may completely fold:

```cpp
int x = square(5);
```

into a constant machine value while `std::is_constant_evaluated()` still reports false for the source-language evaluation context.

“Computed by the compiler” and “constant-evaluated by the C++ language rules” are not synonyms.

### Direct use inside `static_assert` is trivially true

This is not a meaningful test:

```cpp
static_assert(std::is_constant_evaluated());
```

A `static_assert` condition is manifestly constant-evaluated, so the answer is necessarily true.

Likewise, using it as the condition of `if constexpr` is almost certainly a conceptual mistake:

```cpp
if constexpr (std::is_constant_evaluated()) {
    // this condition is evaluated as a constant expression
}
```

Use ordinary `if` with `std::is_constant_evaluated()`, or use C++23 `if consteval`.

### Static and thread-local initialization has a trial-evaluation trap

Implementations may trial-evaluate some initializers to determine whether constant initialization is possible.

Code whose value itself changes based on `std::is_constant_evaluated()` can therefore produce unintuitive initialization behavior.

Do not use the function as a general mechanism for predicting startup order or dynamic initialization.

### Prefer `if consteval` in C++23 when branch intent is the point

C++20:

```cpp
constexpr int f(int x)
{
    if (std::is_constant_evaluated())
        return compile_time_path(x);

    return runtime_path(x);
}
```

C++23:

```cpp
constexpr int f(int x)
{
    if consteval {
        return compile_time_path(x);
    } else {
        return runtime_path(x);
    }
}
```

The latter more directly expresses the intent and supplies an immediate-function context in the consteval branch.

---

## 8. Constexpr and consteval in templates

### A constexpr function template can serve both compile-time and runtime callers

```cpp
template<class T>
constexpr T square(T value)
{
    return value * value;
}

static_assert(square(5) == 25);

double runtime = read_double();
double result = square(runtime);
```

Each specialization is checked according to the instantiated operations.

A template being declared `constexpr` does not guarantee that every possible specialization and every possible call can be constant-evaluated.

That is a major distinction from old template metaprogramming, where the type system itself often forced computation to happen at translation time.

### Constrain templates according to the operations the constexpr body needs

C++20 concepts make the contract clearer:

```cpp
template<std::integral T>
constexpr T gcd(T a, T b)
{
    while (b != 0) {
        T next = a % b;
        a = b;
        b = next;
    }

    return a;
}

static_assert(gcd(48, 18) == 6);
```

The `constexpr` keyword does not replace ordinary type constraints.

### A consteval function template creates a compile-time-only family

```cpp
template<std::unsigned_integral T>
consteval T bit_mask(unsigned bit)
{
    if (bit >= std::numeric_limits<T>::digits)
        throw "bit out of range";

    return T{1} << bit;
}

constexpr auto mask = bit_mask<std::uint32_t>(7);
```

Every selected specialization is an immediate function.

This is useful for APIs where accepting a runtime value would be semantically wrong.

### `if constexpr` is fundamental inside constexpr templates

```cpp
template<class T>
constexpr auto normalized(T value)
{
    if constexpr (std::is_floating_point_v<T>) {
        return value < T{} ? -value : value;
    } else if constexpr (std::is_signed_v<T>) {
        return value < 0 ? -value : value;
    } else {
        return value;
    }
}
```

The discarded branches can contain code that would be invalid for another `T`.

This is a template-instantiation feature, not a guarantee that the function call itself is constant-evaluated.

### Non-type template arguments require compile-time values

```cpp
template<std::size_t N>
struct fixed_buffer
{
    std::array<std::byte, N> storage;
};

consteval std::size_t packet_size()
{
    return 128;
}

fixed_buffer<packet_size()> packet;
```

This is a natural place for `consteval`: the consumer already requires a constant expression.

### Structural non-type template parameters widened in C++20

C++20 expanded the kinds of values usable as non-type template parameters through structural types.

This makes compile-time value programming less dependent on integers and enums alone.

For constness, the important connection is:

> a value used as a non-type template argument belongs to compile-time type/instantiation semantics, not merely “an optimized runtime constant”.

### Constexpr variable templates are useful compile-time constants

Since C++14:

```cpp
template<class T>
constexpr bool is_byte_like_v =
    std::is_same_v<std::remove_cv_t<T>, std::byte>;
```

Variable templates combine naturally with `inline`/ODR rules in modern headers.

Standard type traits expose most of their convenient `_v` forms this way.

### C++20 abbreviated templates preserve constness according to parameter form

```cpp
void inspect(const auto& value);
```

is an abbreviated function template.

The ordinary deduction rules still apply:

* `auto value` drops top-level const;
* `const auto& value` accepts mutable/const arguments through a const reference;
* `auto&& value` can be a forwarding reference and preserve cv/ref categories.

Concept syntax changes how the template is written, not the fundamental const-deduction rules.

### Constant evaluation can instantiate templates even when runtime evaluation is not required

Template instantiation rules account for expressions that are potentially constant-evaluated.

This can make a definition necessary earlier than expected.

For advanced template code, do not assume:

> if this branch does not execute at runtime, the compiler cannot need the constexpr definition.

Constant evaluation participates in determining whether certain template definitions must exist and be instantiated.

### C++23 `if consteval` composes well with templates

```cpp
template<class T>
constexpr T transform(T value)
{
    if consteval {
        return compile_time_transform(value);
    } else {
        return runtime_transform(value);
    }
}
```

This supports one generic API with two implementation strategies.

The compile-time branch may call immediate functions because it is an immediate-function context.

### Do not overuse consteval in generic libraries

A generic algorithm often benefits from being usable in both modes:

```cpp
template<class T>
constexpr T algorithm(T value);
```

Changing it to:

```cpp
template<class T>
consteval T algorithm(T value);
```

eliminates runtime callers completely.

Use `consteval` when the compile-time-only restriction is part of the semantic contract.

Use `constexpr` when dual-mode execution is useful.

---

## 9. API design and common constness pitfalls

### Const on a return-by-value type is usually harmful

Avoid:

```cpp
const std::string make_name();
```

Prefer:

```cpp
std::string make_name();
```

The returned object is a new value owned by the caller.

Top-level const on a prvalue provides little useful protection and historically interferes with move-aware/generic operations.

The caller can choose:

```cpp
const auto name = make_name();
```

if local immutability is desired.

### Const on a by-value parameter is an implementation detail

These declare the same function type for callers:

```cpp
void f(int value);
void f(const int value);
```

Inside the definition, marking the local copy const may document implementation intent:

```cpp
void f(const int value)
{
    // value cannot be reassigned here
}
```

but the const does not belong in an API-level distinction.

### Prefer const references for non-owning read-only access when lifetime is clear

```cpp
void process(const large_object& value);
```

is a common way to express:

* required argument;
* no copy;
* no mutation through this parameter.

But for small trivially-copyable types, pass-by-value may be simpler and faster.

Const-correctness is not a reason to mechanically pass everything by `const&`.

### Do not return const references to internal data from temporaries

A class may have a perfectly correct lvalue accessor:

```cpp
const std::string& name() const &;
```

but this is dangerous if callable on a temporary and the reference escapes.

Use ref qualification:

```cpp
const std::string& name() const &;
std::string name() &&;
```

or disable the rvalue overload if appropriate.

### Iterator constness has two different meanings

```cpp
std::vector<int>::const_iterator
```

is an iterator that yields read-only element access.

```cpp
const std::vector<int>::iterator
```

is a const iterator **object** that still denotes mutable elements.

For example:

```cpp
std::vector<int> values{1, 2, 3};

const std::vector<int>::iterator it = values.begin();

*it = 9; // OK
// ++it; // error: iterator object is const
```

Top-level constness of the iterator and low-level constness of the element access are different.

### Const does not imply thread safety

A `const` method can:

* mutate `mutable` state;
* mutate external objects through pointers;
* observe objects concurrently modified elsewhere.

A data race is still undefined behavior.

Constness is an API/type-system tool, not a synchronization primitive.

### `volatile` is not “the opposite of const”

The qualifiers are orthogonal:

```cpp
volatile const std::uint32_t* status_register;
```

may represent a read-only memory-mapped register.

`volatile` concerns observable accesses according to the language/platform contract.

It is not a general inter-thread synchronization mechanism.

Use atomics/mutexes for C++ concurrency.

### Constness does not provide ownership

```cpp
const T*
const T&
std::span<const T>
std::string_view
```

all say something about mutation through the interface.

None inherently owns the referred-to object.

Always ask separately:

> who keeps this object alive?

---

## Rules worth keeping in working memory

### Choose the facility by the guarantee you want

Use `const` when the guarantee is:

> this object/access path must not modify the value.

Use `constexpr` when the guarantee is:

> this variable must be a constant-expression value, or this function should be usable during constant evaluation.

Use `consteval` when the guarantee is:

> calling this function at runtime is a programming error.

Use `constinit` when the guarantee is:

> this static/thread-local variable must not require dynamic initialization.

Use `std::is_constant_evaluated()` when:

> C++20 code genuinely needs to inspect its current evaluation mode.

Use C++23 `if consteval` when:

> the implementation has a distinct constant-evaluation branch, especially if it needs immediate functions.

Use `if constexpr` when:

> template structure depends on types/compile-time template conditions.

### Review checklist

When reviewing modern constness code, ask:

| Question                                                                     | Why it matters                                                         |
|------------------------------------------------------------------------------|------------------------------------------------------------------------|
| Is `const` being confused with constant evaluation?                          | Runtime-initialized const objects are common.                          |
| Is top-level const being lost through by-value deduction intentionally?      | `auto` and templates drop it by value.                                 |
| Does a const reference/view outlive its owner?                               | Constness does not extend arbitrary lifetimes.                         |
| Does a const member mutate cache state?                                      | Use `mutable` deliberately and consider thread safety.                 |
| Is `const_cast` modifying a genuinely const object?                          | That is undefined behavior.                                            |
| Is constness expected to be deep?                                            | C++ constness is generally shallow.                                    |
| Should a function be `constexpr` or `consteval`?                             | Dual-mode versus compile-time-only is an API decision.                 |
| Is `constinit` being mistaken for const?                                     | It controls initialization, not mutation.                              |
| Is `is_constant_evaluated()` being used to predict optimization?             | It reports language evaluation mode, not optimizer folding.            |
| Is `is_constant_evaluated()` used in `if constexpr`?                         | That is almost always conceptually wrong.                              |
| Could C++23 `if consteval` express the intent better?                        | It directly models the mode branch.                                    |
| Does a constexpr template assume all specializations are constant-evaluable? | Constant-evaluability depends on the instantiated operations and call. |
| Is consteval unnecessarily blocking runtime use of a generic API?            | Prefer constexpr when both modes are valid.                            |
| Can a temporary call a reference-returning member?                           | Ref qualifiers can prevent dangling.                                   |
| Is a namespace/header constant ODR-safe?                                     | C++17 inline constexpr variables simplify this.                        |
| Is a const method assumed to be thread-safe?                                 | Const is not synchronization.                                          |

### Final mental model

Modern C++ has several independent axes that happen to use similar words:

```text
type mutability
    const / volatile

constant-expression capability
    constexpr

mandatory immediate evaluation
    consteval

static initialization guarantee
    constinit

evaluation-mode detection
    is_constant_evaluated / if consteval

template structure selection
    if constexpr
```

The most important habit is to stop asking:

> “Is this constant?”

and ask the more precise question:

> “Constant in which sense: type, value, initialization phase, evaluation mode, or template instantiation?”

Once those meanings are separated, most of C++ constness becomes systematic rather than mysterious.

---

---

## Diagnostics and useful compiler settings

TODO: complete the paragraph

---

## Standards timeline

### C++98/03: runtime/type-system constness

The core model already included:

* cv-qualified types;
* const member functions;
* const overloads;
* const references;
* `mutable`;
* `const_cast`;
* internal linkage behavior of namespace-scope const variables.

The model was almost entirely about runtime object interfaces and type checking.

### C++11: `constexpr` changes the role of C++ at compile time

C++11 introduced:

* `constexpr` variables;
* `constexpr` functions;
* `constexpr` constructors;
* literal types;
* a language-supported alternative to many template-metaprogramming calculations.

The original constexpr function-body restrictions were intentionally narrow.

### C++14: relaxed constexpr

C++14 made constexpr functions much more ordinary:

* local variables;
* loops;
* branches;
* mutation of local state during constant evaluation.

This is where constexpr began to look like regular C++ rather than a restricted expression language.

### C++17: inline constexpr variables and constexpr branching

C++17 added or matured:

* `if constexpr`;
* constexpr lambdas;
* inline variables;
* implicitly inline constexpr static data members.

This greatly improved header-only compile-time libraries.

### C++20: compile-time programming becomes a major execution mode

Important additions include:

* `consteval`;
* `constinit`;
* `std::is_constant_evaluated()`;
* much broader constexpr standard-library support;
* constexpr dynamic-allocation capabilities under constant-evaluation rules;
* expanded literal/constexpr class capabilities;
* concepts and abbreviated templates that compose with constexpr code.

C++20 is the release where “compile-time C++” became practical for much more ordinary application/library code.

### C++23: cleaner evaluation-mode branching and relaxed constexpr rules

C++23 adds or strengthens:

* `if consteval` / `if !consteval`;
* relaxed restrictions on what may appear in constexpr functions;
* static constexpr locals in constexpr functions under the new rules;
* refined immediate-function propagation behavior;
* explicit object parameters, which simplify cv/ref-correct class interfaces.

The overall direction is toward writing one natural C++ function and allowing it to participate in constant evaluation when its executed path permits it.

---

## Further reading

TODO: complete the paragraph

---
