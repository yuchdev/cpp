# 02. Constness Concept in Modern C++

## Runtime vs. compile-time constness

* `const` is not "compile-time constant": `const int x = f();` is runtime-initialized; it is read-only *after* initialization.
* `constexpr` means "can be constant-evaluated" (in a constant-expression context). A `constexpr` function can still run at runtime when the context doesn't require constant evaluation.
* `const` affects the *type system*; "constant expression" affects *evaluation*:
  `const` participates in overload resolution and template matching; "constant expression" governs whether the compiler may/has-to evaluate now (array bounds, `case` labels, template args, `static_assert`, etc.).
* `constexpr` variables are implicitly `const`: `constexpr int k = 3;` has type `const int`.
* In C++17, inline variables (`inline constexpr ...`) make it easier to define `constexpr` variables in headers without multiple-definition/ODR headaches.
* `volatile` is not "const but opposite": it's about observable side effects and optimization barriers for specific operations; it is not a concurrency primitive. `volatile const` exists (e.g., read-only memory-mapped registers).
* `consteval` is a C++20 keyword that means "must be constant-evaluated" (in a constant-expression context). A `consteval` function must run at compile-time when the context requires constant evaluation.

---

## Type-level const: top-level vs. low-level

* Top-level const vs. low-level const:

  * `const int* p` = pointer to const (low-level const: cannot modify `*p`)
  * `int* const p` = const pointer (top-level const: cannot reseat `p`)
  * `const int* const p` = both
* Template deduction and `auto` treat these differently:

  * By-value deduction drops top-level const: `const int cx; auto a = cx; // int`
  * References preserve const: `auto& r = cx; // const int&`
  * `decltype(expr)` preserves "exactness" (and has special rules depending on whether `expr` is an id-expression, parenthesized, etc.).

---

## Const references, temporaries, and lifetime pitfalls

* `const T&` can bind to temporaries; lifetime extension applies to the reference's lifetime:
  * `const std::string& r = std::string("hi");` is safe while `r` is alive.
* Lifetime extension does not cross function boundaries:
  * returning `const T&` bound to a temporary inside a function yields a dangling reference.
* `std::string_view` is a non-owning view: `const` doesn't help with lifetime.
  * Returning a `string_view` to a temporary string is a classic dangling bug.

---

## Const and member functions: what `const` really means

* `const` member functions are about the `this` type: in `T::foo() const`, `this` is `T const*` (conceptually: you can't modify non-`mutable` members).
* `mutable` members can be modified inside `const` member functions:

  * Common for caches, memoization, lazy evaluation, and debug counters.
* Constness in APIs is about *observability* ("logical constness") vs. enforced "bitwise constness":

  * `mutable` is the standard way to allow "logically const" internal mutations.

---

## Overloading on const and value category

* You can overload member functions on constness:

  * `T& at(size_t);` vs `const T& at(size_t) const;`
* You can further overload on ref-qualifiers (C++11+):

  * `f() &`, `f() const &`, `f() &&`
    Useful to avoid returning references from temporaries and to enable "move-aware" APIs.

---

## `const_cast`: what is legal, and what becomes UB

* `const_cast` can remove constness, but modifying a truly `const` object is Undefined Behavior.
* It is only safe to remove constness and write when the original object was *not actually const*, e.g.:
  * the object is non-const, but accessed through a `const T*`/`const T&` alias.
* Modifying string literals or any object in read-only storage is UB; `constexpr` does not magically make unsafe pointers safe.

---

## "Const is shallow": deep constness is not automatic

* `const` does not imply deep immutability:
  * `const std::shared_ptr<T>` makes the smart pointer non-reseatable, but does not make `T` const.
  * `const std::vector<T*>` prevents changing the vector elements (the pointers), but you can still mutate `*ptr` if it points to non-const.
* A `const` object is not thread-safe by virtue of being const:
  * another alias may mutate it; `mutable` may mutate it; and non-atomic data races are still UB.

---

## Return types and API pitfalls

* `const` on a return-by-value type is almost always useless and can be harmful:
  * `const T f();` doesn't provide meaningful safety and can interfere with moves and generic code.
  * Prefer `T f();` and use constness at the *use site* (`const auto x = f();`) if needed.
* `const` on a by-value parameter is typically pointless:
  * `void g(const int x)` is effectively the same as `void g(int x)` for callers (const applies only to the local copy).

---

## Iterator constness

* Distinguish:
  * `container::const_iterator` = iterator that yields `const T&` (cannot mutate elements)
  * `const container::iterator` = iterator object that can't be reseated, but still yields mutable `T&`
* A "const iterator object" is not the same thing as a "const_iterator type".

---

## `constexpr` objects and literal types

* A class usable in constant expressions is a literal type (informally: can appear in `constexpr` contexts).
* In C++11/14/17, `constexpr` constructors and member functions enable compile-time objects.
* You can have `constexpr` arrays and access array elements / object members in constant expressions.

```cpp
struct point {
  double x_, y_, z_;
  constexpr point(double x, double y, double z) : x_{x}, y_{y}, z_{z} {}
  constexpr double norm2() const { return x_*x_ + y_*y_ + z_*z_; }
};

constexpr point p{1.,2.,3.};
constexpr point parr[] = {{1.,2.,3.},{1.,2.,3.}};
static_assert(p.norm2() == 14.0);
static_assert(parr[0].norm2() == 14.0);
```

## `const`, `constexpr`, `consteval`, and `constinit` in practice

`const` is about the immutability of an object through a particular name; it says nothing about whether the compiler can initialize it at compile time. 

`constexpr` says a value or function may participate in constant evaluation, but a call is still allowed to run at runtime when the context does not require it. 

C++20 introduced the sharper split: `consteval` makes a function immediate, so every call must be constant-evaluated; `constinit` requires a variable with static storage duration to be constant-initialized, which is useful for global state that must not be zero-initialized and then assigned dynamically; and `std::is_constant_evaluated()` lets a function detect whether it is being evaluated in a constant-expression context so it can choose different code paths without guessing. C++23 keeps this model consistent by extending what is legal in constant evaluation and by making the language more predictable for compile-time work, so the same API can be written once and still behave differently depending on whether the compiler is evaluating it now or later. 

Example:

```cpp
#include <type_traits>

consteval int square(int value) {
  return value * value;
}

constexpr int evaluation_kind() {
  return std::is_constant_evaluated() ? 1 : 2;
}

constexpr int compile_time_square = square(4); // OK: consteval call
static_assert(evaluation_kind() == 1);         // constant evaluation

int main() {
  int runtime_result = evaluation_kind(); // 2: evaluated at runtime
  int input = 4;
  // int runtime_square = square(input);  // error: immediate call uses runtime input
}
```


The common traps are: treating `constexpr` as a command to force compile-time execution, treating `consteval` as a runtime optimization hint, and treating `std::is_constant_evaluated()` as a way to predict dynamic initialization or global startup semantics. The right mental model is: `const` = write protection, `constexpr` = usable in a constant expression, `consteval` = must be immediate, `constinit` = must be constant-initialized, and `std::is_constant_evaluated()` = the exact hook for “compile time vs runtime” logic.
