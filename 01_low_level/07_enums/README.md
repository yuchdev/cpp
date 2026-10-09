# 07. Enums in Modern C++

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

## 6. Scoped enums (`enum class`) - C++11+

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

* ❌ Using unscoped enums in headers
* ❌ Relying on default underlying type
* ❌ Assuming enum values are contiguous
* ❌ Using enums as bitmasks without operators
* ❌ Switching without handling invalid states

---
---

## Rules worth keeping in working memory

The central distinction in enumerations and their underlying representations is between what the C++ type system guarantees, what the implementation commonly does, and what an API merely assumes. Carrying that distinction from declarations through operations avoids most of the surprises discussed above.

> A valid low-level operation needs a language-level contract, not just a machine-level outcome that looks plausible.

### Core rules

The following rules condense the chapter into reviewable decisions:

1. **Prefer scoped enums when accidental integer conversion would be unsafe.**
2. **Specify an underlying type when layout or ABI demands it.**
3. **Do not assume every underlying value names an enumerator.**
4. **Implement flag operators deliberately rather than treating every enum as a bitmask.**


The practical guidance already emphasized in this chapter remains applicable:

Use `enum class` with explicit underlying types by default.
Only use unscoped enums when:

* required for C compatibility
* interacting with legacy APIs
* performance constraints mandate it (rare)

Enums are a type-safety tool, not just named integers - treat them as such.

---

### Pitfalls at a glance

These recurring failures are particularly useful to recognize in code review because each starts from a plausible but insufficient assumption.

| Pitfall | What happens | Do instead |
|---|---|---|
| `static_cast<E>(wire_value)` assumed validated | May produce unnamed enumerator value | Validate protocol values |
| Implicit unscoped enum to integer | Can select unintended overloads | Prefer `enum class` |
| Unspecified enum storage expected stable | ABI/layout coupling | Specify underlying type at interfaces |

Each safer alternative makes an implicit precondition visible either in the type or in the code that checks it.

### Mental model summary

The underlying distinction is consistent across the subject: a language guarantee is portable; an implementation choice must be checked; a domain requirement must be stated by the program.

| Question | Language guarantee | Implementation detail | Application responsibility |
|---|---|---|---|
| What may this operation assume? | Specified preconditions and types | ABI, representation, optimizer strategy | Select the appropriate contract |
| What happens at an edge case? | Specified result or behavior category | Diagnostics and machine reaction | Validate inputs and lifetimes |
| Can this cross an interface? | Only what types and linkage guarantee | Toolchain and platform compatibility | Document conversions and ownership |

The third column cannot substitute for the first, and the fourth is where domain-specific requirements belong.

### Review checklist

Use this checklist when changing enumerations and their underlying representations code or reviewing low-level interfaces:

- [ ] Have we distinguished language guarantees from platform-specific observations?
- [ ] Are all input domains and conversion or lifetime preconditions explicit?
- [ ] Can the relevant edge cases be tested without executing undefined behavior?
- [ ] Does the chosen API encode as much of the intended contract as practical?

---

## Diagnostics, useful compiler settings and extensions

Compilers can diagnose many suspicious uses of enumerations and their underlying representations, but they cannot infer every application invariant. A clean warning build is useful evidence, not proof: tools can recognize some invalid expressions statically and others only when particular paths execute.

### Warnings

Start with high-signal diagnostics and add specialized checks where the chapter's failure modes justify them.

| Compiler | Flags | What they reveal |
|---|---|---|
| GCC | `-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion` | Suspicious conversions, extensions, and common type mistakes |
| Clang | `-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion` | Similar mistakes, with different analysis coverage |
| MSVC | `/W4 /permissive-` | Common warnings and nonconforming language extensions |

### Sanitizers and runtime checks

Runtime instrumentation can expose executed failures but does not validate every semantic assumption.

| Tool | Setting | Best use |
|---|---|---|
| AddressSanitizer | `-fsanitize=address` or MSVC `/fsanitize=address` | Invalid object lifetime and memory access when applicable |
| UndefinedBehaviorSanitizer | `-fsanitize=undefined` (GCC/Clang) | Instrumented undefined operations; coverage is incomplete |
| clang-tidy / static analysis | Selected checks and `/analyze` (MSVC) | Suspicious contracts, conversions, and API use |

### Semantics-changing options

Language mode, optimizer assumptions, and vendor extensions can affect which source expressions are accepted and how they are interpreted. The following controls deserve deliberate treatment rather than being regarded as speed settings alone.

| Option | Effect | Recommendation |
|---|---|---|
| `-std=gnu++20` vs `-std=c++20` | GNU mode may accept non-standard constructs | Prefer ISO mode when teaching portable rules |
| `-fno-strict-aliasing` | Weakens some alias-based optimization assumptions, not object-lifetime rules | Never use it to justify invalid accesses |
| `-fwrapv` (GCC/Clang) | Gives signed overflow wrapping semantics under that compiler option | Use only under a documented toolchain contract |
| `-ffast-math` (GCC/Clang) | Permits FP transformations inconsistent with strict numerical assumptions | Avoid for reproducibility and FP environment examples |

These options affect different topics to different degrees; only enable a topic-specific workaround when it addresses a known and measured requirement.

### Compiler extensions

Vendor builtins can improve diagnostics or performance, but they are not portable substitutes for a standard language rule.

| Extension | Compilers | Use | Portable approach |
|---|---|---|---|
| `__builtin_assume` / `__assume` | Clang / MSVC | Gives optimizer an unchecked precondition | Validate invariants; use `[[assume]]` only where C++23 is supported |
| `__builtin_object_size` | GCC/Clang | Estimates available object storage | Carry an explicit extent in APIs |
| `__attribute__` / `__declspec` attributes | GCC/Clang / MSVC | Vendor-specific linkage/layout hints | Prefer standard attributes where applicable |

Keep extensions behind small wrappers, prefer `__cpp_*`/feature tests over compiler versions, and use `-pedantic-errors` or `/permissive-` when checking standard conformance.

### Libraries and tooling beyond the standard

The C++ Core Guidelines and the static analyzers built around them can help express and review preconditions that a raw type cannot encode.

| Tool | Purpose | When useful |
|---|---|---|
| clang-tidy / cppcheck | Static analysis | Reviewing conversion, lifetime, and interface contracts |
| Microsoft GSL | Non-owning and bounds-oriented vocabulary | Modernizing raw-pointer/size interfaces |
| Compiler Explorer | Compare generated code | Verifying performance assumptions against actual toolchains |

The best defense is layered: expressive types and APIs first, then warnings, targeted tests, runtime checking, and narrowly isolated extensions.

---

## Standards timeline

The core model of enumerations and their underlying representations evolved incrementally; newer standards add safer vocabulary without retroactively rewriting all legacy expressions. The milestones below separate changes to the language from changes to available library interfaces.

### C++98/03: The original model

Unscoped enumerations and underlying representations are long-standing.

### C++11: Stronger types and interfaces

Scoped enums and fixed underlying types substantially improve APIs.

### C++14: Incremental refinement

No material change to the underlying scalar model.

### C++17: Library and deduction evolution

No material change to enum validation obligations.

### C++20: Modern vocabulary

Concepts can express enum constraints in templates.

### C++23: Further standard facilities

`std::to_underlying` exposes the underlying value explicitly.

### C++26: Emerging improvements

Reflection-related conveniences remain an evolving area; do not assume automatic enum iteration.

When documenting a facility, state its required language/library version rather than inferring support from the compiler's branding.

---

## Migration note for Java / Python / C# developers

Readers familiar with managed runtimes often expect enumerations and their underlying representations to come with runtime metadata, automatic lifetime management, or checked failures. In C++, some of those services come from a chosen library type, while raw language mechanisms may deliberately expose more responsibility to the caller. The important translation is from a *runtime guarantee* in one language to a *type, contract, and lifetime guarantee* in C++.

### Where intuition transfers and where it breaks

The same apparent operation may have a different failure mode or storage model in each language.

| Concept | Java | Python | C# | C++ reality |
|---|---|---|---|---|
| enum | named constant group | Enum is a class with instances | `Enum`/`IntEnum` classes | Enum with value metadata |
| Invalid access/operation | Usually throws or is checked | Usually raises an exception | Often throws in safe code | May be ill-formed, defined, unspecified, or undefined depending on the operation |
| Lifetime | Managed object reachability | Reference counting / GC | Managed GC | Automatic, dynamic, and explicitly borrowed lifetimes coexist |

The distinction matters at API boundaries: a familiar surface syntax does not imply familiar failure behavior.

### Common wrong assumptions

One tempting assumption is that a successful local test demonstrates that an operation is safe. For the low-level C++ rules in this chapter, a test can demonstrate behavior of one build, but cannot establish portability or rule out undefined behavior. Another is that an object is kept alive by every handle referring to it; non-owning pointers, references, and views do not do that. Finally, a managed-language exception should not be presumed to exist at an equivalent C++ failure point.

### Idiomatic C++ replacement

The following mappings help make intent explicit without imitating a managed runtime mechanically.

| Habit from managed languages | Idiomatic C++ |
|---|---|
| Closed domain | Use `enum class` with explicit parsing/validation |
| Flags | Define underlying bit operations and masks intentionally |
| Expect automatic runtime bounds/lifetime checks | Select an owning container or checked API; validate preconditions explicitly |

Use these choices because they express the program's requirements, not merely because they resemble familiar constructs from another language.

---

## Further reading

These references document the underlying language rules and library contracts. They are starting points for checking precise preconditions; the chapter's examples explain how the rules interact.

### Standard and language reference

* [language/enum](https://en.cppreference.com/w/cpp/language/enum)
* [types/underlying_type](https://en.cppreference.com/w/cpp/types/underlying_type)
* [utility/to_underlying](https://en.cppreference.com/w/cpp/utility/to_underlying)
