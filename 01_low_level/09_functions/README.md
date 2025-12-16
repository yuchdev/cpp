# Functions in C++

> Functions are the fundamental abstraction boundary in C++.
> They combine language rules, ABI conventions, optimizer behavior, and C legacy.

## Advanced Facts, Pitfalls, and Evolution (C++98 → C++20)

## 1. What a function is (and is not)

* A function is a named callable entity with:

  * a type (`R(Args...)`)
  * linkage
  * calling convention
* A function is not:

  * an object
  * a closure (unless wrapped)
  * a constructor

> Constructors are not functions - they have no return type and special lookup/dispatch rules.

---

## 2. C inheritance: the foundation

C++ function rules are built on C:

* Call by value
* Pointer decay
* No runtime type information for parameters
* ABI compatibility with C when `extern "C"` is used

```cpp
extern "C" void c_api(int);
```

### Key C-inherited facts

* Top-level `const` on parameters is ignored
* Arrays decay to pointers
* Function names decay to function pointers

```cpp
void f(int);
void f(const int);   // same function type
```

---

## 3. Function size and performance myths

* Function call cost is negligible compared to:

  * cache misses
  * branches
  * memory access
* Inlining is an optimization hint, not a command
* Readability beats micro-optimization

> Modern compilers inline aggressively - *even without `inline`*.

---

## 4. Function types and declarations

### Prefix vs suffix return type

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

---

## 5. `auto` and return type deduction

### Evolution

| Standard | Feature               |
| -------- | --------------------- |
| C++11    | trailing return types |
| C++14    | return type deduction |
| C++20    | constrained auto      |

```cpp
auto f() { return 42; }        // int
auto g() { return 3.14; }      // double
```

❌ Mixing return types is ill-formed.

---

## 6. `constexpr`, `consteval`, `constinit`

### `constexpr` functions

* Can run at compile-time or runtime
* Cannot have observable side effects
* May contain loops (since C++14)

```cpp
constexpr int square(int x) { return x*x; }
```

### `consteval` (C++20)

* Must be evaluated at compile time
* Runtime call is ill-formed

```cpp
consteval int id(int x) { return x; }
```

---

## 7. Attributes (`[[...]]`)

Attributes annotate functions for tools and optimizers.

### Common attributes

* `[[nodiscard]]` (C++17)
* `[[noreturn]]`
* `[[deprecated]]`
* `[[likely]]`, `[[unlikely]]` (C++20)

```cpp
[[noreturn]] void fatal();
```

> Attributes do not change semantics - they convey intent.

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

## 12. Function pointers

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

---

## 13. Inline functions

### Meaning of `inline`

* Allows multiple definitions (ODR)
* Suggests inlining (not guaranteed)

### When `inline` matters

* Header-defined functions
* Template definitions

### When inlining is impossible

* Recursion
* Virtual dispatch
* Taking function address

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

## 16. Lambdas (anonymous function objects)

### Mental model

> A lambda is syntax sugar for an unnamed class with `operator()`.

```cpp
auto l = [](int x) { return x*x; };
```

Becomes (conceptually):

```cpp
struct __lambda {
    int operator()(int x) const { return x*x; }
};
```

---

## 17. Lambda captures

| Capture      | Meaning              |
| ------------ | -------------------- |
| `[=]`        | copy                 |
| `[&]`        | reference            |
| `[this]`     | current object       |
| `[x = expr]` | init-capture (C++14) |

### Important rules

* Namespace variables don't need capture
* Members captured via `this` are references
* Lambdas may outlive scope → prefer copy capture

---

## 18. Mutable lambdas

```cpp
auto f = [x]() mutable { x++; };
```

Allows modification of captured copies.

---

## 19. Lambda types and storage

* Every lambda has a unique, unnamed type
* No two lambdas share a type
* Store using:

  * `auto`
  * `std::function<R(Args...)>`

### `std::function`

* Type-erased
* Allocates
* Slower than direct calls

---

## 20. `operator()` and callable objects

Any type with `operator()` is a callable.

```cpp
struct Adder {
    int operator()(int a, int b) const { return a+b; }
};
```

Used by:

* algorithms
* policies
* function objects

---

## 21. Macros vs functions

### Macros

* No types
* No scope
* No debugging
* No overloads

```cpp
#define SQR(x) ((x)*(x))
```

### Predefined macros

* `__cplusplus`
* `__FILE__`, `__LINE__`
* `__DATE__`, `__TIME__`
* `__func__`

### Prefer functions unless:

* Conditional compilation
* Compile-time code generation
* Platform detection

---

## 22. Function evolution timeline

| Standard | Features                                         |
| -------- | ------------------------------------------------ |
| C++98    | Basic functions, pointers                        |
| C++11    | `auto`, lambdas, `constexpr`, variadics          |
| C++14    | return type deduction, init-capture              |
| C++17    | attributes, `constexpr` expanded                 |
| C++20    | `consteval`, concepts, `std::span`, `[[likely]]` |

---

## 23. Migration note for Java / Python / C# developers

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

