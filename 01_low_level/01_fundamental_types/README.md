# 01. Fundamental types 

## 1. `bool`

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

## 2. `nullptr` / `std::nullptr_t`

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
  You can copy it, pass it, and store it - but there is no "other" null pointer value.

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

## 3. `void`

`void` is a **fundamental type with no values**.
It represents *absence of a value*, not a zero, not null, not false.

### Core facts

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

* `void*` normally used at the lowerer levels of abstraction (C APIs, memory management). In the upper levels, its use is most likely a design error.
* Standard C++ forbids pointer arithmetic on `void*`

```cpp
void* p;
// p++; // ill-formed
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

### `std::void_t`

`std::void_t` is a type-trait utility (available since C++17), defined roughly as:

```cpp
template<class...>
using void_t = void;
```

It turns any list of well-formed types into `void`. Its purpose is to make
type or expression validity participate in template substitution, commonly
for SFINAE-based detection:

```cpp
template<class T, class = void>
struct has_value_type : std::false_type {};

template<class T>
struct has_value_type<T, std::void_t<typename T::value_type>>
    : std::true_type {};
```

If `T::value_type` exists, the specialization is valid and the trait is true.
If it does not, the substitution fails and the primary template is used instead.
This lets generic code detect optional members or expressions at compile time
without making the program ill-formed.

### Pitfalls

* `void*` arithmetic is **not allowed** in C++

  ```cpp
  void* p;
  // ++p; // ill-formed (GNU extension allows this  -  non-portable)
  ```

* `void` is not a placeholder type. If you want "unknown type", you probably want:
  * templates
  * `auto`
  * `std::any`
  * `std::variant`

---

## 4. `std::byte` (optional but strongly recommended)

> **Note:** `std::byte` is *not* a fundamental type, but it exists specifically to avoid abusing integers for raw memory.

### Why it belongs here

`std::byte` fills the conceptual gap between:

* "integer"
* "raw memory"

and avoids the semantic abuse of `unsigned char`.

### Core facts

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

### Why it matters

* Prevents accidental arithmetic on raw memory
* Communicates intent better than `unsigned char`
* Plays well with strict aliasing rules

---
---

## Rules worth keeping in working memory

The central distinction in fundamental types (`bool`, `nullptr_t`, `void`, and `std::byte`) is between what the C++ type system guarantees, what the implementation commonly does, and what an API merely assumes. Carrying that distinction from declarations through operations avoids most of the surprises discussed above.

> A valid low-level operation needs a language-level contract, not just a machine-level outcome that looks plausible.

### Core rules

The following rules condense the chapter into reviewable decisions:

1. **A `bool` represents truth, but integer promotions can still affect an expression.**
2. **Use `nullptr` for null pointer values rather than overloaded integer-zero conventions.**
3. **Treat `void` as absence of an object value, not a typeless object.**
4. **Use `std::byte` for raw data whose arithmetic interpretation must remain explicit.**


### Pitfalls at a glance

These recurring failures are particularly useful to recognize in code review because each starts from a plausible but insufficient assumption.

| Pitfall | What happens | Do instead |
|---|---|---|
| `bool` passed into arithmetic | Integer promotion may obscure the intended domain | Make conversions and predicates explicit |
| `NULL` in overloaded calls | May resolve to an integer overload | Pass `nullptr` |
| Arithmetic on `std::byte` | Ordinary integer operators are unavailable | Convert with `std::to_integer` |

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

Use this checklist when changing fundamental types (`bool`, `nullptr_t`, `void`, and `std::byte`) code or reviewing low-level interfaces:

- [ ] Have we distinguished language guarantees from platform-specific observations?
- [ ] Are all input domains and conversion or lifetime preconditions explicit?
- [ ] Can the relevant edge cases be tested without executing undefined behavior?
- [ ] Does the chosen API encode as much of the intended contract as practical?

---

## Diagnostics, useful compiler settings and extensions

Compilers can diagnose many suspicious uses of fundamental types (`bool`, `nullptr_t`, `void`, and `std::byte`), but they cannot infer every application invariant. A clean warning build is useful evidence, not proof: tools can recognize some invalid expressions statically and others only when particular paths execute.

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

The core model of fundamental types (`bool`, `nullptr_t`, `void`, and `std::byte`) evolved incrementally; newer standards add safer vocabulary without retroactively rewriting all legacy expressions. The milestones below separate changes to the language from changes to available library interfaces.

### C++98/03: The original model

Core scalar categories and the `void` type were already part of the language.

### C++11: Stronger types and interfaces

No new fundamental model of `bool` or `void`; `nullptr` and `std::nullptr_t` separated null pointers from integer zero.

### C++14: Incremental refinement

No material changes to these core categories.

### C++17: Library and deduction evolution

`std::byte` provided a distinct vocabulary for object representations.

### C++20: Modern vocabulary

Concepts and expanded constexpr facilities improve generic code around these types.

### C++23: Further standard facilities

No material change to the semantics of these four categories.

### C++26: Emerging improvements

Future library facilities may build on the same scalar type model; check implementation support.

When documenting a facility, state its required language/library version rather than inferring support from the compiler's branding.

---

## Migration note for Java / Python / C# developers

Readers familiar with managed runtimes often expect fundamental types (`bool`, `nullptr_t`, `void`, and `std::byte`) to come with runtime metadata, automatic lifetime management, or checked failures. In C++, some of those services come from a chosen library type, while raw language mechanisms may deliberately expose more responsibility to the caller. The important translation is from a *runtime guarantee* in one language to a *type, contract, and lifetime guarantee* in C++.

### Where intuition transfers and where it breaks

The same apparent operation may have a different failure mode or storage model in each language.

| Concept | Java | Python | C# | C++ reality |
|---|---|---|---|---|
| types | boolean expression, `None`, or `null` | Primitive `boolean` | Built-in `bool` | Built-in `bool` |
| Invalid access/operation | Usually throws or is checked | Usually raises an exception | Often throws in safe code | May be ill-formed, defined, unspecified, or undefined depending on the operation |
| Lifetime | Managed object reachability | Reference counting / GC | Managed GC | Automatic, dynamic, and explicitly borrowed lifetimes coexist |

The distinction matters at API boundaries: a familiar surface syntax does not imply familiar failure behavior.

### Common wrong assumptions

One tempting assumption is that a successful local test demonstrates that an operation is safe. For the low-level C++ rules in this chapter, a test can demonstrate behavior of one build, but cannot establish portability or rule out undefined behavior. Another is that an object is kept alive by every handle referring to it; non-owning pointers, references, and views do not do that. Finally, a managed-language exception should not be presumed to exist at an equivalent C++ failure point.

### Idiomatic C++ replacement

The following mappings help make intent explicit without imitating a managed runtime mechanically.

| Habit from managed languages | Idiomatic C++ |
|---|---|
| B | o |
| U | s |
| Expect automatic runtime bounds/lifetime checks | Select an owning container or checked API; validate preconditions explicitly |

Use these choices because they express the program's requirements, not merely because they resemble familiar constructs from another language.

---

## Further reading

These references document the underlying language rules and library contracts. They are starting points for checking precise preconditions; the chapter's examples explain how the rules interact.

### Standard and language reference

* [language/types](https://en.cppreference.com/w/cpp/language/types)
* [types/byte](https://en.cppreference.com/w/cpp/types/byte)
* [types/nullptr_t](https://en.cppreference.com/w/cpp/types/nullptr_t)
