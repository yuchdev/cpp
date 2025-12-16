# C++ Enums — Advanced Facts, Pitfalls, and Evolution (C++98 → C++20)

> This document is a reference chapter on `enum` and `enum class`, collecting obscure, non-obvious, and advanced facts useful for low-level, ABI-sensitive, or correctness-critical C++ code.

---

## 1. What an enum *is* in C++

* An `enum` is a distinct user-defined type whose values are represented as integers.
* Each enumerator is an integral constant expression.
* An enum object:

  * occupies storage
  * has a concrete underlying integer representation
  * is *not* just an `int`, even if it behaves like one in some contexts

```cpp
enum Color { Red, Green, Blue };
```

---

## 2. Unscoped enums (C++98 style)

### Key properties

* Enumerators are injected into the enclosing scope
* Enum values implicitly convert to `int`
* Underlying type is implementation-defined
* Size is platform-dependent
* Arithmetic on enum values is allowed (often unintentionally)

```cpp
enum Status { Ok, Error };
int x = Ok + 10;   // allowed
```

### Pitfalls

* Name collisions in headers
* Accidental arithmetic
* Silent conversions in overload resolution
* Values outside enumerator set are possible

```cpp
Status s = static_cast<Status>(42); // allowed, but not meaningful
```

---

## 3. Underlying type and size rules

### C++98

* Underlying type is implementation-defined
* Must be large enough to represent all enumerators

### C++11+

* You may explicitly specify the underlying type:

```cpp
enum ErrorCode : unsigned short { A = 1, B = 2 };
```

### Practical implications

* ABI stability requires explicit underlying types
* Network / file formats should never rely on default enum size
* `sizeof(enum)` is not guaranteed to equal `sizeof(int)`

---

## 4. Enum values outside enumerators

* Casting an integer to an enum is allowed
* Resulting value may not correspond to any enumerator
* Merely *holding* such a value is not automatically UB
* Logic assuming "only valid enumerators exist" becomes broken

```cpp
enum Mode { Idle, Run, Stop };
Mode m = static_cast<Mode>(99); // legal, but invalid state
```

Rule:
Validate when converting from external or untrusted data.

---

## 5. Enums and constant expressions

* Enumerators are integral constant expressions
* Can be used in:

  * `case` labels
  * array bounds
  * `static_assert`
  * template non-type parameters

```cpp
enum Size { Small = 4, Large = 16 };
int buffer[Large];
```

---

## 6. Scoped enums (`enum class`) — C++11+

### Motivation

Scoped enums fix most C++98 enum problems.

### Properties

* No implicit conversion to `int`
* Enumerators do not leak into enclosing scope
* Stronger type safety
* Underlying type can be fixed
* Forward declaration always allowed

```cpp
enum class Color : std::uint8_t { Red, Green, Blue };
```

```cpp
Color c = Color::Red;
// int x = c; // ERROR
```

---

## 7. Name scoping and collisions

### Unscoped enums

```cpp
enum Color { Red, Green };
enum Traffic { Red, Yellow }; // collision
```

### Scoped enums

```cpp
enum class Color { Red, Green };
enum class Traffic { Red, Yellow }; // OK
```

### C++20: `using enum`

```cpp
using enum Color;
Color c = Red;  // shorter, but may reintroduce collisions
```

Guideline:
Use `using enum` sparingly and never in widely included headers.

---

## 8. Forward declaration rules

### Scoped enums

```cpp
enum class ErrorCode; // OK
```

### Unscoped enums (C++11+)

```cpp
enum Legacy : unsigned int; // must specify underlying type
```

* Incomplete enum types can be:

  * declared
  * used as pointers/references
* But not:

  * `sizeof`
  * value construction
  * switching

---

## 9. Conversions and `std::underlying_type`

### Safe extraction of integer value

```cpp
template <typename E>
constexpr auto to_underlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}
```

* Essential for:

  * logging
  * serialization
  * bitmask operations
  * interop with C APIs

---

## 10. Enums as bitmasks

### Why scoped enums are better for flags

* No accidental arithmetic
* No implicit conversion
* Explicit operator definitions required

```cpp
enum class Perm : uint32_t {
    Read = 1 << 0,
    Write = 1 << 1
};
```

### Operator overloads required

```cpp
Perm operator|(Perm a, Perm b);
Perm operator&(Perm a, Perm b);
```

### Pitfalls

* Prefer unsigned underlying type
* `~flag` flips *all bits*, not just known flags
* Often need an `All` mask:

```cpp
(~x) & All
```

---

## 11. Enums in `switch` statements

### No enforced exhaustiveness

* C++ does not require handling all enumerators
* Compilers may warn, but language does not guarantee safety

```cpp
switch (mode) {
    case Mode::Run: ...
}
```

### Invalid enum values

* `switch` may reach no `case`
* Always handle default or validate beforehand

```cpp
default: return "<unknown>";
```

---

## 12. Enums and overload resolution

### Unscoped enums

* Implicit conversion to `int` participates in overload resolution
* May select unintended overload

### Scoped enums

* No implicit conversions
* Safer and more predictable overload behavior

---

## 13. Enums and ABI / interfaces

### Public headers should:

* Use `enum class`
* Fix underlying type
* Avoid unscoped enums
* Avoid relying on `sizeof(enum)`

### Serialization / networking

* Always serialize underlying value
* Never rely on enumerator ordering implicitly

---

## 14. Enums vs other integer-like types

| Type            | Type Safety | Implicit Conversion | Scope    | Use Case       |
| --------------- | ----------- | ------------------- | -------- | -------------- |
| `int`           | none        | yes                 | global   | raw arithmetic |
| unscoped `enum` | weak        | yes                 | injected | legacy C APIs  |
| `enum class`    | strong      | no                  | scoped   | modern C++     |
| `std::byte`     | strong      | no                  | scoped   | raw memory     |

---

## 15. Common anti-patterns

❌ Using unscoped enums in headers
❌ Relying on default underlying type
❌ Assuming enum values are contiguous
❌ Using enums as bitmasks without operators
❌ Switching without handling invalid states

---

## 16. Mental model summary

> An enum is not an integer, but it is represented by one.

* Unscoped enums favor convenience over safety
* Scoped enums favor correctness over convenience
* Enum values can exist outside the listed enumerators
* The language does not enforce "valid enum state"
* `enum class` is the default choice in modern C++

---

### 17. A note for developers coming from **Java** and **Python**

If you come from Java or Python, it's important to reset some expectations about what an enum *is* in C++.

In Java, an `enum` is a full-fledged class: each value is a singleton object, can have fields, methods, constructors, inheritance-like behavior, identity (`==`), guaranteed exhaustiveness in `switch`, and runtime reflection support. In Python, enums are also objects, dynamically created, introspectable, iterable, and fundamentally runtime constructs with no concern for storage layout or ABI.

In C++, an enum is not an object hierarchy and not a runtime abstraction. It is a compile-time type with an integer representation, designed primarily for efficiency, ABI stability, and low-level control. Even `enum class`—the "safe" modern form—does not create objects with identity or behavior; it creates a *strongly typed integer domain*. C++ enums can hold values that are *not* listed among their enumerators, can be cast from raw integers, and are not inherently range-checked or exhaustive. The compiler does not enforce "only valid enum states exist."

This means:

* There is no built-in guarantee of exhaustiveness like Java's `switch` over enums.
* There is no reflection or automatic iteration over enum values.
* Enum size, signedness, and layout matter for ABI, serialization, and performance.
* Validation is the developer's responsibility, especially at system boundaries.

The payoff is that C++ enums integrate cleanly with memory, bitwise operations, hardware interfaces, and wire formats—things Java and Python deliberately abstract away. Think of C++ enums not as "objects with names", but as type-safe symbolic integers whose power (and danger) comes from how close they are to the machine.

---

## Final recommendation

Use `enum class` with explicit underlying types by default.
Only use unscoped enums when:

* required for C compatibility
* interacting with legacy APIs
* performance constraints mandate it (rare)

Enums are a type-safety tool, not just named integers — treat them as such.

---
