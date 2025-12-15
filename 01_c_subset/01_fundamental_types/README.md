### Fundamental types 

#### 1. `bool`

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

#### 2. `nullptr` / `std::nullptr_t`

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

You already covered **the two “weird" non-numeric fundamentals** very well.
For a *“simplest C++ types that are not integer / FP"* reference, there are **only a few legitimate additions**, and they fall into two categories:

1. **True fundamental language types** you can document alongside `bool` / `nullptr_t`
2. **Frequently confused but *not* fundamental** types worth explicitly calling out so readers don’t expect them here

Below is a **clean, minimal extension** that fits your README style and doesn’t dilute the focus.

---

### 3. `void`

`void` is a **fundamental type with no values**.
It represents *absence of a value*, not a zero, not null, not false.

#### Core facts

* `void` has no values and no objects

  ```cpp
  void v;        // ill-formed
  sizeof(void); // ill-formed
  ```

* `void` is most commonly used:

  * as a function return type
  * as a generic object pointer target (`void*`)

* `void*` is a **generic object pointer**, not a typeless pointer

  ```cpp
  int x = 42;
  void* p = &x;          // OK
  int* ip = static_cast<int*>(p); // required cast
  ```

* `void*` can point to **any object type**, but **not** to:

  * function pointers
  * pointers to member

  ```cpp
  void (*fp)();
  // void* v = fp; // ill-formed in standard C++
  ```

* `void` participates in overload resolution

  ```cpp
  void f();
  int  f(int);

  f();   // calls void f()
  f(42); // calls int f(int)
  ```

* `void` expressions can appear in comma expressions

  ```cpp
  (void)side_effect(); // explicitly discard result
  ```

#### Pitfalls

* `void*` arithmetic is **not allowed** in C++

  ```cpp
  void* p;
  // ++p; // ill-formed (GNU extension allows this — non-portable)
  ```

* `void` is not a placeholder type
  If you want “unknown type", you probably want:

  * templates
  * `auto`
  * `std::any`
  * `std::variant`

---

### 4. `std::byte` (optional but strongly recommended)

> **Note:** `std::byte` is *not* a fundamental type, but it exists specifically to avoid abusing integers for raw memory.

#### Why it belongs here

`std::byte` fills the conceptual gap between:

* “integer"
* “raw memory"

and avoids the semantic abuse of `unsigned char`.

#### Core facts

* `std::byte` represents raw memory, **not a number**

* Introduced in **C++17**

* Defined as an `enum class`

  ```cpp
  #include <cstddef>

  std::byte b{0xFF};          // ill-formed
  std::byte b = std::byte{0xFF}; // OK
  ```

* No implicit integer conversions

  ```cpp
  std::byte b{};
  // int x = b; // ill-formed
  ```

* Only bitwise operations are allowed

  ```cpp
  b |= std::byte{0x01};
  ```

* Safe for raw object representation access

  ```cpp
  std::byte* raw = reinterpret_cast<std::byte*>(&obj);
  ```

#### Why it matters

* Prevents accidental arithmetic on raw memory
* Communicates intent better than `unsigned char`
* Plays well with strict aliasing rules

---

## Mental Model Summary

| Feature             | `bool`         | `nullptr_t`      | `void`               | `std::byte`                |
|---------------------|----------------|------------------|----------------------|----------------------------|
| Has values          | Yes (2)        | Yes (1)          | No                   | Yes                        |
| Integer-like        | Yes            | No               | No                   | No                         |
| Pointer-related     | Yes (conv)     | Yes (core role)  | Yes (`void*`)        | Yes (raw memory)           |
| Arithmetic          | Yes (promoted) | No               | No                   | Bitwise only               |
| Compile-time usable | Yes            | Yes              | Yes (in type system) | Yes                        |
| Common pitfall      | Promotions     | `NULL` confusion | Misuse as typeless   | Expecting integer behavior |

---
