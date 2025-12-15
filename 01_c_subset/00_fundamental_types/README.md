### Fundamental types 

#### 1) `bool`

* `bool` is a distinct fundamental type, but participates in implicit integer promotions
  In most expressions, `bool` is promoted to `int`.

  ```cpp
  bool a = true, b = true;
  auto x = a + b; // int, value == 2
  ```

* Only two values are representable, but more can exist in memory
  Formally, `bool` can only hold `true` or `false`. Assigning any non-zero value converts to `true`, but the *stored bit pattern* is implementation-defined.
  This matters for serialization and memory inspection.

* `sizeof(bool)` is implementation-defined but ≥ 1
  It's usually `1`, but never rely on bit-packing unless you use `std::bitset` or bit-fields.

* `bool` has special formatting pitfalls

  ```cpp
  std::cout << true;           // prints 1
  std::cout << std::boolalpha // prints true
  ```

  Many logging systems silently print `1/0` unless explicitly configured.

* `bool&` is rare and dangerous in generic code
  Because of integer promotions and temporary materialization, APIs returning `bool&` are fragile and often break expectations.

* `vector<bool>` is not a `vector` of `bool`
  It's a bit-packed specialization returning proxy objects instead of `bool&` -- an example of premature optimization, that haunts C++ Standardization Committee to this day.

  ```cpp
  std::vector<bool> v;
  auto r = v[0]; // proxy, not bool&
  ```

  This breaks generic code and surprises even senior developers.

* `bool` can silently break overload resolution

  ```cpp
  void f(int);
  void f(bool);
  f('a'); // calls f(int), but subtle changes can flip behavior
  ```

* A pointer can be implicitly converted to a `bool`

* `bool` is often abused as a domain type
  Many APIs overload meaning into `bool` (`success`, `ownership`, `mode`).
  `enum class` is almost always a better semantic choice.

* Atomic `bool` is special
  `std::atomic<bool>` may be lock-free even when other atomics aren't. It's often implemented using bit-test instructions.

---

#### 2) `nullptr` / `std::nullptr_t`

* `nullptr` is not a pointer
  It's a prvalue of type `std::nullptr_t`, a distinct fundamental type convertible to *any* pointer or pointer-to-member.

* `nullptr` fixes overload ambiguity that `NULL` cannot

  ```cpp
  void f(int);
  void f(char*);
  f(nullptr); // calls f(char*)
  f(NULL);    // ambiguous or f(int)
  ```

* `std::nullptr_t` is implicitly convertible, but not comparable to integers

  ```cpp
  nullptr == 0;     // OK
  nullptr == false // ill-formed
  ```

* There is exactly one value of `std::nullptr_t`
  You can copy it, pass it, and store it—but there is no "other" null pointer value.

* `decltype(nullptr)` is useful in templates

  ```cpp
  template <typename T>
  void f(T);

  f(nullptr); // T == std::nullptr_t
  ```

  This allows precise specialization for "null-ness".

* `std::nullptr_t` participates in overload resolution as a real type

  ```cpp
  void f(std::nullptr_t);
  void f(void*);
  f(nullptr); // prefers std::nullptr_t
  ```

* `nullptr` converts to pointer-to-member, integers do not

  ```cpp
  int C::*;
  int C::* p = nullptr; // OK
  ```

  This is one of the reasons `nullptr` had to be its own type.

* `nullptr` has no addressable object identity

  ```cpp
  auto* p = &nullptr; // ill-formed
  ```

  It's a pure language literal, not an object.

* `nullptr` is usable in constant expressions

  ```cpp
  constexpr int* p = nullptr;
  ```

  This allows compile-time reasoning about pointer states.

* `nullptr` enables safer SFINAE and concepts
  You can distinguish:

  * "Pointer is provided"
  * "Explicit null was passed"
    which is impossible with `0` or `NULL`.

---

## Mental model summary

| Feature       | `bool`                        | `nullptr`                       |
|---------------|-------------------------------|---------------------------------|
| Type category | Fundamental integer-like type | Fundamental null-literal type   |
| Implicit conv | To `int`                      | To any pointer / member pointer |
| Pitfalls      | Promotions, `vector<bool>`    | Overload ambiguity with `NULL`  |
| Compile-time  | Yes, but rarely semantic      | Fully constexpr-capable         |
| Best practice | Avoid as domain signal        | Always mixing with `NULL` / `0` |

---

### Runtime/Compile-time constness

* `const` is not "compile-time constant": `const int x = f();` is still runtime-initialized; it's just read-only *after* initialization.
* `constexpr` is "can be constant-evaluated" -- not "will be": a `constexpr` function can run at runtime too. Constant evaluation happens when the context requires it.
* `const` affects a type system; "constant expression" affects evaluation: `const` participates in overload resolution and template matching; "constant expression" is about whether the compiler may/has-to evaluate now.
* `const` member functions are about the `this` type: in `T::foo() const`, `this` is `T const*`. But `mutable` fields ignore this rule and can be modified inside `const` functions.
* Top-level vs low-level const:
    * `const int* p` = pointer to const (low-level const)
    * `int* const p` = const pointer (top-level const)
    * template deduction treats these very differently.
* `constexpr` variables are implicitly `const`: `constexpr int k = 3;` is `const int` (and may be ODR-used issues apply if you take addresses etc., though C++17 inline vars eased this).
* `constinit` is *only* about initialization timing: it guarantees static initialization (no dynamic init / order fiasco), but the variable can still be mutable: `constinit int g = 42;` is allowed.
* `consteval` is "must be compile-time": calling a `consteval` function in a runtime-only context is ill-formed. Great for "enforced compile-time validation".
* `const` does not imply thread-safety: `const` only restricts mutation through that expression/type. The same object can be mutated elsewhere (non-const alias, `mutable`, atomics, `const_cast` UB if originally const).
* `volatile` is not "const but opposite": it's about observable side effects and preventing certain optimizations; it's not a concurrency primitive. `volatile const` exists and is occasionally meaningful for memory-mapped registers.

If you want, I can turn these into "flashcards" with tiny runnable examples (each fact in ~10–20 lines) so you can drop them into your Study C++ notes or a quiz script.

