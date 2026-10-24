# 10. Linking, External Declarations and the Program Model

> C-style linking. Linkage and function pointers.
> One Definition Rule (ODR). Translation units.
> Name mangling, ABI boundaries, symbol visibility, dynamic linking.

Linking is the phase where **separately compiled translation units** are combined into a single program or library.
Most subtle C++ bugs at scale are **linking bugs**, not syntax or type-system bugs.

---

## 1. Translation Units

A **translation unit (TU)** is the result of preprocessing a source file:

* One `.cpp` file
* Plus all headers it includes (recursively)
* After macro expansion and conditional compilation

Each TU is compiled **independently**, then linked together.

> The compiler never sees the whole program at once (unless LTO is enabled).

---

## 2. Linkage Types

### 1. Internal linkage

Entities with **internal linkage** are visible *only within their own translation unit*.

```cpp
static int i;   // internal linkage
```

* Each TU gets its **own copy**
* Same name in another TU refers to a **different entity**
* Common for:

  * `static` globals
  * entities in anonymous namespaces
  * `const` namespace-scope variables (by default, in C++)

---

### 2. External linkage

Entities with **external linkage** refer to the *same entity across all translation units*.

```cpp
int g;          // external linkage by default
extern int g;   // declaration only
```

* One definition in the entire program
* Accessible across TUs

#### Anonymous namespace (important subtlety)

```cpp
namespace {
    int i;
}
```

* `i` has **internal linkage**
* Not reachable from other TUs
* Preferred over `static` in modern C++

> In C++11 and later, anonymous namespaces are the **recommended way** to express internal linkage.

---

### 3. No linkage

Some names do **not participate in linkage at all**:

* Local variables
* Function parameters
* Labels
* Enumerators
* Typedef / `using` names
* Template parameters

They are **purely compile-time or local constructs**.

---

## 3. Scope vs Linkage (do not confuse)

| Concept     | Meaning                                             |
| ----------- | --------------------------------------------------- |
| **Scope**   | Where a name can be *written*                       |
| **Linkage** | Whether two declarations refer to the *same entity* |

C scopes:

* File scope
* Block scope
* Function prototype scope
* Function scope

C++ adds:

* Namespace scope
* Class scope

> Scope is a **syntax rule**, linkage is a **program-wide semantic rule**.

---

## 4. The One Definition Rule (ODR)

The **ODR** governs what may be defined where:

### Objects and non-inline functions

* **Exactly one definition** in the entire program

### Types, templates, inline functions

* One definition **per translation unit**
* All definitions must be **identical**

Violations lead to:

* Multiple-definition linker errors
* Or worse: **undefined behavior** if the linker merges incompatible definitions

---

## 5. `static` keyword: historical overload

`static` has **two unrelated meanings**:

1. **Class scope**
   → static data member (shared by all objects)

2. **Namespace/global scope**
   → internal linkage (historical "`intern`" keyword)

```cpp
static int x;   // internal linkage (global)
```

> Prefer **anonymous namespaces** instead of `static` at namespace scope.

---

## 6. `const` and linkage

In **C++ (unlike C)**:

```cpp
const int ci = 42;   // internal linkage by default
```

To make it externally visible:

```cpp
extern const int ci;   // declaration
```

Or, since **C++17**, the modern solution:

```cpp
inline constexpr int ci = 42;
```

* ✔ header-safe
* ✔ ODR-safe
* ✔ single entity program-wide

---

## 7. Headers and Inclusion Rules

Best practices:

* Use `#include` **only at namespace/global scope**
* Prefer `#pragma once` over macro guards
  (de-facto supported by all major compilers)
* Headers should be **self-sufficient**
* Headers should **declare**, not define (unless `inline`, templates, or header-only)

---

## 8. Global Initialization Order

There is **no guaranteed order** of initialization of global objects across TUs.

This causes the infamous **static initialization order fiasco**.

### Correct pattern (since C++11)

```cpp
int& instance()
{
    static int value = 0;
    return value;
}
```

* Construct-on-first-use
* Thread-safe since C++11

### C++20 improvement: `constinit`

```cpp
constinit int g = 42;
```

* Enforces **static initialization**
* Prevents accidental dynamic initialization
* Does **not** imply constness

---

## 9. `extern "C"` and ABI Boundaries

```cpp
extern "C" void c_function();
```

`extern "C"` specifies:

* **No C++ name mangling**
* C ABI-compatible symbol
* Required for:

  * C / C++ interop
  * `dlsym`, `GetProcAddress`
  * Assembly, Fortran, foreign runtimes

Rules:

* Declarations and definitions must **agree**
* You cannot mix C and C++ linkage for the same entity
* Only affects **linkage**, not type checking

Wrapping headers:

```cpp
#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#ifdef __cplusplus
}
#endif
```

---

## 10. Name Mangling

C++ encodes:

* Function names
* Namespaces
* Classes
* Parameter types
* cv-qualifiers

into **mangled symbol names**.

This enables overloading, but breaks cross-language linking.

Use tools to inspect:

* `nm -C`
* `objdump`
* `dumpbin`
* `readelf`
* `otool`

---

## 11. Inline Functions and Variables

### Inline functions

* May appear in multiple TUs
* ODR-merged into one entity

### Inline variables (C++17)

```cpp
inline int counter = 0;
```

* Exactly one program-wide object
* Header-safe
* Replaces many old `extern` patterns

---

## 12. Templates and Linking

Templates are **instantiated at use sites**.

Common pitfalls:

* Template defined in `.cpp` only → **undefined reference**
* Fixes:

  * Move definition to header
  * Or use **explicit instantiation**

```cpp
template int foo<int>(int);
```

---

## 13. Symbol Visibility (Shared Libraries)

On ELF / Mach-O systems (GCC/Clang):

```cpp
__attribute__((visibility("default")))
```

Typical pattern:

* Compile with `-fvisibility=hidden`
* Explicitly export API symbols

Benefits:

* Faster linking
* Smaller dynamic symbol tables
* Better encapsulation

---

## 14. Windows DLL Import / Export

```cpp
__declspec(dllexport)
__declspec(dllimport)
```

Usually wrapped in macros:

```cpp
#ifdef BUILDING_DLL
#define API __declspec(dllexport)
#else
#define API __declspec(dllimport)
#endif
```

Different from ELF:

* Symbol visibility is **opt-in**
* Import libraries participate in linking

---

## 15. Dynamic Linking at Runtime

POSIX example:

* `dlopen`
* `dlsym`
* `dlclose`

Requires:

* C linkage symbols
* Stable ABI
* Careful lifetime management

---

## 16. Facts about Linking

```
extern "C" {
// ...
}
```

* This technique is commonly used to produce a C++ header from a C header
* Alternatively, conditional compilation (12.6.1) can be used to create a common C and C++ header:

```
#ifdef __cplusplus
extern "C" {...}
```

* A name with C linkage can be declared in a namespace (std::printf)
* A variable defined outside any function (that is, global, namespace, and class static variables) is initialized before main() is invoked
* Often, a function returning a reference to static is a good alternative to a global variable
* The initialization of a local static is thread-safe (42.3.3)

---

---

## 17. Modern linking and ABI engineering: details beyond the linker command

### C++17 inline variables preserve one entity across translation units

C++98's common header pattern for global mutable variables required an `extern` declaration in a header and exactly one definition elsewhere. C++17 introduces inline variables, allowing an identical definition to appear in multiple translation units while designating one program-level entity, subject to the one-definition rule. An inline variable should be declared inline on every relevant declaration that defines it; it is not a substitute for managing a shared library's symbol visibility and ABI.

```cpp
// configuration.hpp (C++17)
inline std::atomic<unsigned> active_sessions{0};
// Unlike a namespace-scope const in a header, this is one shared entity
// within a correctly linked program.
```

Keep dynamic initialization costs and initialization dependencies visible even for inline variables. An ODR violation may be ill-formed with no diagnostic required, rather than an ordinary linker error.

### C++20 modules introduce a third linkage category: module linkage

In named-module contexts, some namespace-scope entities have *module linkage*: they can be referenced from translation units attached to the same named module, but are not exposed as external-linkage declarations to unrelated translation units. This is not equivalent to an anonymous namespace's internal linkage or to a namespace nested under a module name.

```cpp
// module-unit.cpp (C++20 modules)
module;
#include <cstdint>
export module telemetry;
int detail_counter = 0; // module linkage in the appropriate context
export int read_counter() { return detail_counter; }
```

This requires an actual module build graph and compiled module interface artifacts; simply renaming a header to a module does not solve dependency or ODR problems. The global module fragment and textual headers preserve relevant traditional linkage rules.

### Dynamic initialization ordering has stronger and weaker guarantees

Within a translation unit, ordered dynamic initialization follows definition order for the applicable variables. Across translation units, initialization order is much more constrained by language rules and may remain indeterminate for many dependencies. The *construct-on-first-use* idiom relies on function-local statics and C++11's thread-safe initialization guarantee.

```cpp
Registry& registry() {
    static Registry instance;
    return instance;
}
```

Initialization is safe under the specified concurrency rules; destruction-order dependencies can still be problematic at process termination. C++20 `constinit` forces static initialization or rejects the declaration, but does not make an object immutable and does not impose arbitrary cross-module teardown ordering.

### LTO can expose ODR and ABI mistakes that separate compilation concealed

Link-time optimization allows the optimizer to see definitions across translation units and reason more aggressively about inline functions, class layouts, and aliasing. Code that accidentally violates the one-definition rule might appear stable when compiled independently and break under LTO.

```text
TU A: header sees PACKED=1, defines inline access()
TU B: header sees PACKED=0, defines inline access()
Result: potentially different definitions of the same inline entity
```

The problem is not LTO itself: compiling the same entity inconsistently already breaks the C++ program contract. Enable reproducible macro settings and ABI-oriented integration tests, and consider compiler ODR diagnostics where available.

### Weak symbols, COMDAT, and visibility are ABI mechanisms, not ISO C++ contracts

ELF symbol binding, PE/COFF COMDAT folding, Mach-O symbol visibility, and linker-specific dead-stripping are implementation technologies. They help implement inline definitions, templates, and dynamic libraries, but their exact behavior is not a source-language guarantee.

For ELF use `readelf -Ws`, `nm -C`, `objdump -T` and linker map files; for Windows use `dumpbin /symbols` and `dumpbin /exports`. Record symbol exports as part of ABI tests rather than expecting visibility to follow from namespaces or C++ linkage alone.

### C++23 and C++26 preserve explicit module and ABI design obligations

C++23's language and library additions do not replace the one-definition rule, and C++26 reflection or package-oriented tooling does not promise ABI compatibility between independently built C++ libraries. Template instantiations, exception models, RTTI, standard library configurations, and calling conventions still require deliberate compatibility decisions.

Prefer narrow C interfaces for long-lived cross-toolchain plugin boundaries, and document compiler, target, standard library, and exported symbol versions for C++-to-C++ binary interfaces.

---

## Rules worth keeping in working memory

The controlling distinction for linkage and translation units is between the portable language contract and the behavior of a particular compilation environment. C++ gives programmers the ability to build close to system boundaries, but also requires those boundaries to be specified rather than guessed.

> A source declaration states a C++ contract; neither a plausible runtime result nor a successful build can silently strengthen that contract.

### Core rules

These rules are a compact form of the chapter's essential reasoning:

1. **Keep one non-inline definition where the ODR requires it.**
2. **Separate name lookup from linkage and symbol visibility.**
3. **Control non-local initialization dependencies explicitly.**
4. **Treat `extern "C"` as a linkage mechanism, not universal ABI safety.**

### Pitfalls at a glance

Both failures below are attractive shortcuts precisely because they may seem to work on one toolchain.

| Pitfall | What happens | Do instead |
|---|---|---|
| Multiple definitions in headers | ODR violation or linker conflict | Use inline definitions or single-source definitions |
| Global initialization order dependencies | May use an object before it is initialized | Prefer constant initialization or local statics |

Neither local behavior nor a compiler warning replaces a defined API and lifetime contract.

### Mental model summary

The following comparison makes clear which properties are supplied by the language and which must be established in your project.

| Concern | Language | Toolchain/platform | Application |
|---|---|---|---|
| Names and types | Lookup, declarations, type rules | ABI encoding and diagnostic quality | Stable interface design |
| Runtime behavior | Specified behavior categories | Execution mechanism | Input validation and ownership |
| Portability | Requirements of chosen C++ edition | Vendor extensions and build mode | Testing supported configurations |

The application column is where external requirements become enforceable preconditions.

### Review checklist

Before publishing or refactoring linkage and translation units code, ask:

- [ ] Have we stated the preconditions and the relevant lifetime or linkage rules?
- [ ] Can another translation unit or toolchain use this interface without undocumented assumptions?
- [ ] Have we distinguished a guaranteed rule from an implementation observation?
- [ ] Are relevant boundary and negative cases represented in tests?

---

## Diagnostics, useful compiler settings and extensions

The compiler can identify many malformed uses of linkage and translation units, while some errors only surface at a link step, in a different build configuration, or along an executed path. Diagnostics complement, but cannot replace, correct declarations and contracts.

### Warnings

Start with standard conformance and conversion diagnostics; enable more targeted checks when a specific problem class appears.

| Compiler | Flags | Catches |
|---|---|---|
| GCC/Clang | `-Wall -Wextra -Wpedantic -Wconversion` | Common suspicious declarations, conversions, and extensions |
| MSVC | `/W4 /permissive- /analyze` | Language conformance and many statically recognizable mistakes |

### Sanitizers and runtime checks

Instrumentation can expose faults in executed code. It cannot certify that every program path is portable or free of undefined behavior.

| Tool | Setting | Detects |
|---|---|---|
| AddressSanitizer | `-fsanitize=address` or `/fsanitize=address` | Many invalid memory accesses and lifetime mistakes |
| UndefinedBehaviorSanitizer | `-fsanitize=undefined` | Selected undefined operations, not all UB |
| Static analyzer | clang-tidy, cppcheck, MSVC `/analyze` | Suspicious contracts and data-flow defects |

### Semantics-changing options

These switches affect how the compiler may interpret source or optimize assumptions; they are not merely performance presets.

| Option | Effect | Recommendation |
|---|---|---|
| `-std=gnu++20` instead of `-std=c++20` | Permits selected GNU extensions | Prefer ISO C++ mode to validate portable examples |
| `-fno-strict-aliasing` | Restricts optimizer alias assumptions | Does not legalize invalid object access |
| `-fwrapv` | Compiler-specific signed overflow behavior | Do not assume the ISO language guarantees it |
| `-flto` | Cross-unit optimization exposes additional visibility/ODR assumptions | Test both LTO and non-LTO if supported |

### Compiler extensions

Extensions can provide essential platform services, but should not escape into a chapter's portable rule statements.

| Extension | Compilers | Purpose | Portable treatment |
|---|---|---|---|
| `__attribute__((visibility))` | GCC/Clang | Symbol exports and visibility | Isolate behind build-specific macros |
| `__declspec(dllexport)` | MSVC | DLL exports | Use a project export macro |
| `__builtin_assume` / `__assume` | Clang/MSVC | Optimization assumptions | Check preconditions rather than asserting false invariants |

Prefer the standard facility where one exists, keep extensions behind wrappers, test with feature macros such as `__has_builtin` and `__cpp_*`, and run `-pedantic-errors` or `/permissive-` to find unintentional nonstandard dependencies.

### Libraries and tooling beyond the standard

Some failure modes belong to the build graph, analyzer, or instrumentation rather than a language feature.

| Tool | Purpose | Typical use |
|---|---|---|
| clang-tidy / cppcheck | Static analysis | Detect suspicious constructs missed by compiler defaults |
| Compiler Explorer | Inspect generated code | Compare compiler assumptions and ABI choices |
| Linker map / `nm` / `dumpbin` | Symbol inspection | Diagnose linkage, exports, and object-file contents |

The layered strategy is a clear type/API contract, strict warnings, sanitizers, platform-specific options kept behind wrappers, static analysis, and targeted tests.

---

## Standards timeline

The standards preserve the core rules of linkage and translation units while expanding ways to state interfaces more precisely. The milestones matter when modernizing an older codebase across several compiler modes.

### C++98/03: Original model

Linkage, translation units, and ODR were fundamental.

### C++11: New language vocabulary

No material change to the core linker model.

### C++14: Incremental refinement

No material change to the core linker model.

### C++17: Library evolution

Inline variables simplify shared header constants.

### C++20: Modern interfaces

Modules provide explicit compiled interfaces but retain ABI/linkage concerns.

### C++23: Broader facilities

Module integration improves without eliminating platform linkers.

### C++26: Evolving tools without fundamental linkage or lookup changes

Expect evolving module-toolchain interoperability; check build-system support.

Annotate version requirements explicitly in source and build definitions; current compiler behavior alone is not a version contract.

---

## Migration note for Java / Python / C# developers

Java and C# load managed modules or assemblies; Python imports runtime modules. C++ compilation commonly produces object files that are combined by a linker. Name mangling, visibility, and ABI compatibility are independent of whether a declaration appears in a header. A managed-language analogy is useful for identifying an intent, but not for predicting C++ storage lifetime, ABI layout, or failure semantics.

### Where intuition transfers and where it breaks

This comparison highlights why translating syntax directly is less reliable than translating contracts.

| Concern | Java | Python | C# | C++ |
|---|---|---|---|---|
| Runtime loading | JVM class loading | Import system | CLR assemblies | Translation units, linker, and platform loader |
| Object lifetime | GC | Reference counts/GC | GC | Determined by storage duration and ownership |
| Invalid operations | Frequently exceptions | Frequently exceptions | Frequently exceptions | May be diagnosed, defined, or undefined |
| linkage and translation units | Managed runtime conventions | Dynamic language conventions | CLR/runtime conventions | Explicit language and ABI contracts |

The main difference is that C++ exposes several boundaries which other runtimes coordinate automatically.

### Common wrong assumptions

It is unsafe to assume that names in different source files are automatically linked, that a non-owning pointer keeps its pointee alive, or that an invalid low-level operation will throw an exception. In C++ these are separate questions. A successful test under one compiler confirms only the observed execution; it does not confer a language guarantee or validate every ABI configuration.

### Idiomatic C++ replacement

The migration is clearest when expressed in terms of intent rather than translated keywords.

| Managed-language habit | Idiomatic C++ |
|---|---|
| Package-level import | Explicit headers/modules plus correct linker inputs |
| Expect runtime checks to catch every misuse | Use type-safe interfaces, validated preconditions, and instrumented debug builds |
| Treat source-file/module visibility as one property | Distinguish lookup, linkage, and ABI/export requirements |

---

## Further reading

The language and standard-library references provide the precise conditions behind the examples. They are sufficient starting points for this chapter; longer bibliographies can be added separately.

### Standard and language reference

* [language/definition](https://en.cppreference.com/w/cpp/language/definition)
* [language/storage_duration](https://en.cppreference.com/w/cpp/language/storage_duration)
* [language/language_linkage](https://en.cppreference.com/w/cpp/language/language_linkage)
