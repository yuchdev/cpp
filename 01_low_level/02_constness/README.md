## Constness facts (C++98 to C++17)

### Runtime vs compile-time constness

* `const` is not "compile-time constant": `const int x = f();` is runtime-initialized; it is read-only *after* initialization.
* `constexpr` means "can be constant-evaluated" (in a constant-expression context). A `constexpr` function can still run at runtime when the context doesn't require constant evaluation.
* `const` affects the *type system*; "constant expression" affects *evaluation*:
  `const` participates in overload resolution and template matching; "constant expression" governs whether the compiler may/has-to evaluate now (array bounds, `case` labels, template args, `static_assert`, etc.).
* `constexpr` variables are implicitly `const`: `constexpr int k = 3;` has type `const int`.
* In C++17, inline variables (`inline constexpr ...`) make it easier to define `constexpr` variables in headers without multiple-definition/ODR headaches.
* `volatile` is not "const but opposite": it's about observable side-effects and optimization barriers for specific operations; it is not a concurrency primitive. `volatile const` exists (e.g., read-only memory-mapped registers).

---

### Type-level const: top-level vs low-level

* Top-level const vs low-level const:

  * `const int* p` = pointer to const (low-level const: cannot modify `*p`)
  * `int* const p` = const pointer (top-level const: cannot reseat `p`)
  * `const int* const p` = both
* Template deduction and `auto` treat these differently:

  * By-value deduction drops top-level const: `const int cx; auto a = cx; // int`
  * References preserve const: `auto& r = cx; // const int&`
  * `decltype(expr)` preserves "exactness" (and has special rules depending on whether `expr` is an id-expression, parenthesized, etc.).

---

### Const references, temporaries, and lifetime pitfalls

* `const T&` can bind to temporaries; lifetime extension applies to the reference's lifetime:

  * `const std::string& r = std::string("hi");` is safe while `r` is alive.
* Lifetime extension does not cross function boundaries:

  * returning `const T&` bound to a temporary inside a function yields a dangling reference.
* `std::string_view` is a non-owning view: `const` doesn't help with lifetime.

  * Returning a `string_view` to a temporary string is a classic dangling bug.

---

### Const and member functions: what `const` really means

* `const` member functions are about the `this` type: in `T::foo() const`, `this` is `T const*` (conceptually: you can't modify non-`mutable` members).
* `mutable` members can be modified inside `const` member functions:

  * Common for caches, memoization, lazy evaluation, and debug counters.
* Constness in APIs is about *observability* ("logical constness") vs enforced "bitwise constness":

  * `mutable` is the standard way to allow "logically const" internal mutations.

---

### Overloading on const and value category

* You can overload member functions on constness:

  * `T& at(size_t);` vs `const T& at(size_t) const;`
* You can further overload on ref-qualifiers (C++11+):

  * `f() &`, `f() const &`, `f() &&`
    Useful to avoid returning references from temporaries and to enable "move-aware" APIs.

---

### `const_cast`: what is legal, and what becomes UB

* `const_cast` can remove constness, but modifying a truly `const` object is Undefined Behavior.
* It is only safe to remove constness and write when the original object was *not actually const*, e.g.:

  * object is non-const, but accessed through a `const T*`/`const T&` alias.
* Modifying string literals or any object in read-only storage is UB; `constexpr` does not magically make unsafe pointers safe.

---

### "Const is shallow": deep constness is not automatic

* `const` does not imply deep immutability:

  * `const std::shared_ptr<T>` makes the smart pointer non-reseatable, but does not make `T` const.
  * `const std::vector<T*>` prevents changing the vector elements (the pointers), but you can still mutate `*ptr` if it points to non-const.
* A `const` object is not thread-safe by virtue of being const:

  * another alias may mutate it; `mutable` may mutate it; and non-atomic data races are still UB.

---

### Return types and API pitfalls

* `const` on a return-by-value type is almost always useless and can be harmful:

  * `const T f();` doesn't provide meaningful safety and can interfere with moves and generic code.
  * Prefer `T f();` and use constness at the *use site* (`const auto x = f();`) if needed.
* `const` on a by-value parameter is typically pointless:

  * `void g(const int x)` is effectively the same as `void g(int x)` for callers (const applies only to the local copy).

---

### Iterator constness

* Distinguish:

  * `container::const_iterator` = iterator that yields `const T&` (cannot mutate elements)
  * `const container::iterator` = iterator object that can't be reseated, but still yields mutable `T&`
* A "const iterator object" is not the same thing as a "const_iterator type".

---

### `constexpr` objects and literal types

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
