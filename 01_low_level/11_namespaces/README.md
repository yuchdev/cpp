# 11. Namespaces in Modern C++

> Logical structure. Name lookup. Linkage interaction.
> Namespaces are not modules, not packages, and not a visibility mechanism.

Namespaces are a **compile-time name organization tool**.
They exist to structure programs, prevent naming conflicts, and enable large-scale library composition.

---

## Purpose of namespaces

* Use namespaces to express logical structure (C++ Standard 14.3.1)
* Prevent name collisions across large codebases and libraries
* Allow independent development and composition of components
* Wrap legacy C libraries to avoid global namespace pollution
* Facilitate argument-dependent lookup (ADL)
* Enable controlled API evolution (inline namespaces)

Namespaces **do not**:

* affect object layout
* control linkage by themselves
* isolate macros
* enforce dependencies
* provide runtime encapsulation

---

## Linkage and namespaces

* A name that can be used in translation units different from the one in which it was defined is said to have **external linkage**
* A name that can be referred to only in the translation unit in which it is defined is said to have **internal linkage**

### Default linkage rules in namespace scope

By default, the following have **internal linkage** in namespace scope:

* `const` objects
* `constexpr` objects
* type aliases
* anything declared `static`

This differs from C, where `const` objects have external linkage by default.

```cpp
const int x = 10;        // internal linkage in C++
extern const int y = 20; // explicitly external
```

---

## Unnamed (anonymous) namespaces

* An unnamed namespace can be used to make names local to a compilation unit
* The effect of an unnamed namespace is very similar to that of internal linkage

```cpp
namespace {
    int helper;
    void internal_function();
}
```

### Important details

* Entities technically have external linkage
* But they are **unreachable** from other translation units
* Preferred modern replacement for `static` globals
* Applies to variables, functions, classes, templates

---

## Named namespaces and namespace merging

Namespaces can be **defined in multiple places** and are merged by name:

```cpp
namespace my {
    int a;
}

namespace my {
    int b;
}
```

### Properties

* Namespaces can be reopened across translation units
* Widely used in the standard library
* No ordering or dependency constraints

---

## Namespace aliases

Namespace aliases provide shorter or version-neutral access:

```cpp
namespace very_long_vendor_library {
    void f();
}

namespace lib = very_long_vendor_library;
```

### Common use cases

* Readability
* Vendor prefixes
* Version abstraction
* Gradual refactoring

---

## `using` declarations and `using` directives

### `using` directive

```cpp
using namespace std;
```

* Brings **all names** into scope
* Can cause ambiguities
* Must **never** appear in headers
* Acceptable only in small, local scopes

### `using` declaration (preferred)

```cpp
using std::vector;
```

* Imports a single name
* Participates in overload resolution
* Safer and explicit

> Prefer `using-declaration` whenever possible.

---

## Argument-Dependent Lookup (ADL)

Namespaces participate in **argument-dependent lookup** (Koenig lookup):

```cpp
namespace math {
    struct vec {};
    void normalize(vec&);
}

math::vec v;
normalize(v); // found via ADL
```

### Why ADL exists

* Enables clean operator syntax
* Avoids verbose qualification
* Fundamental to STL design

### Pitfalls

* Can introduce surprising overloads
* Requires careful namespace design
* Overusing ADL can reduce clarity

---

## Namespaces and operator overloading

Operators should usually be **free functions in the same namespace as their operands**:

```cpp
namespace math {
    vec operator+(vec, vec);
}
```

### Why not member operators?

* Breaks symmetry
* Blocks implicit conversions
* Disables ADL

---

## Inline namespaces (C++11+)

* The `inline` specifier makes a nested namespace the **default meaning** of the enclosing namespace
* This is primarily used for **API and ABI versioning**

```cpp
namespace lib {
    inline namespace v2 {
        void f();
    }
    namespace v1 {
        void f();
    }
}
```

```cpp
lib::f();      // v2
lib::v1::f();  // explicit
```

### Key facts

* Inline namespaces affect name lookup **and ABI**
* Widely used in standard library implementations
* Enable backward compatibility without breaking source code
* Prefer using-declarations to using-directives when accessing inline namespace members

```cpp
using lib::f;        // imports v2::f
using lib::v1::f;    // imports v1::f as well

// Directive would import both versions
using lib;
```

* Whenever possible, use using declarations and directives in scoped locations
* The overloading mechanism works through namespaces
* When declaring using `operator+()`, all overloaded members of `operator+()` will be resolved
* 

---

## Headers, includes, and namespaces

* Spaces are significant within the `< >` or `" "` of an include directive
* The absence of a `.h` suffix does **not** imply anything about how the header is stored

  * `<map>` is often stored as `map.h` internally
* For each C standard-library header `<X.h>`, there is a corresponding C++ header `<cX>`

```cpp
#include <string.h>  // C header
#include <cstring>  // C++ header (preferred)
```

---

## C linkage and namespaces

* We can specify a linkage convention to be used in an `extern` declaration
* `extern "C"` specifies C linkage (no name mangling)

```cpp
extern "C" void c_function();
```

### Linkage blocks

This construct, commonly called a **linkage block**, can be used to enclose a complete C header:

```cpp
extern "C" {
#include <string.h>
}
```

### Key rules

* C functions must be declared with C linkage to be callable from C++
* If a function has multiple linkage declarations, they must agree
* Declaring a function as both C and C++ linkage is an error

---

---

## Rules worth keeping in working memory

The controlling distinction for namespace lookup is between the portable language contract and the behavior of a particular compilation environment. C++ gives programmers the ability to build close to system boundaries, but also requires those boundaries to be specified rather than guessed.

> A source declaration states a C++ contract; neither a plausible runtime result nor a successful build can silently strengthen that contract.

### Core rules

These rules are a compact form of the chapter's essential reasoning:

1. **A namespace is a name scope, not a runtime module loader.**
2. **Avoid using-directives in public headers.**
3. **Keep argument-dependent lookup in mind for operators and generic functions.**
4. **Use unnamed namespaces for translation-unit-local declarations.**

### Pitfalls at a glance

Both failures below are attractive shortcuts precisely because they may seem to work on one toolchain.

| Pitfall | What happens | Do instead |
|---|---|---|
| `using namespace` in headers | Affects downstream name lookup | Use qualification or narrow using-declarations |
| Unexpected ADL overload | Associated namespace adds a candidate | Keep operators in the associated namespace |

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

Before publishing or refactoring namespace lookup code, ask:

- [ ] Have we stated the preconditions and the relevant lifetime or linkage rules?
- [ ] Can another translation unit or toolchain use this interface without undocumented assumptions?
- [ ] Have we distinguished a guaranteed rule from an implementation observation?
- [ ] Are relevant boundary and negative cases represented in tests?

---

## Diagnostics, useful compiler settings and extensions

The compiler can identify many malformed uses of namespace lookup, while some errors only surface at a link step, in a different build configuration, or along an executed path. Diagnostics complement, but cannot replace, correct declarations and contracts.

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

The standards preserve the core rules of namespace lookup while expanding ways to state interfaces more precisely. The milestones matter when modernizing an older codebase across several compiler modes.

### C++98/03: Original model

Named and unnamed namespaces support lookup organization.

### C++11: New language vocabulary

Inline namespaces aid versioning.

### C++14: Incremental refinement

No material changes.

### C++17: Library evolution

No material changes.

### C++20: Modern interfaces

Modules affect declaration visibility but do not make namespaces modules.

### C++23: Broader facilities

No material change to namespace lookup fundamentals.

### C++26: Forthcoming support

New reflection/library features still obey namespace and lookup rules.

Annotate version requirements explicitly in source and build definitions; current compiler behavior alone is not a version contract.

---

## Migration note for Java / Python / C# developers

Managed language packages and Python modules usually combine namespacing with packaging or loading. C++ namespaces do not compile, load, or link a source file: they organize lookup of declarations available to a translation unit. A managed-language analogy is useful for identifying an intent, but not for predicting C++ storage lifetime, ABI layout, or failure semantics.

### Where intuition transfers and where it breaks

This comparison highlights why translating syntax directly is less reliable than translating contracts.

| Concern | Java | Python | C# | C++ |
|---|---|---|---|---|
| Runtime loading | JVM class loading | Import system | CLR assemblies | Translation units, linker, and platform loader |
| Object lifetime | GC | Reference counts/GC | GC | Determined by storage duration and ownership |
| Invalid operations | Frequently exceptions | Frequently exceptions | Frequently exceptions | May be diagnosed, defined, or undefined |
| namespace lookup | Managed runtime conventions | Dynamic language conventions | CLR/runtime conventions | Explicit language and ABI contracts |

The main difference is that C++ exposes several boundaries which other runtimes coordinate automatically.

### Common wrong assumptions

It is unsafe to assume that names in different source files are automatically linked, that a non-owning pointer keeps its pointee alive, or that an invalid low-level operation will throw an exception. In C++ these are separate questions. A successful test under one compiler confirms only the observed execution; it does not confer a language guarantee or validate every ABI configuration.

### Idiomatic C++ replacement

The migration is clearest when expressed in terms of intent rather than translated keywords.

| Managed-language habit | Idiomatic C++ |
|---|---|
| Package namespace as loader | Use namespaces for lookup and build dependencies for linkage |
| Expect runtime checks to catch every misuse | Use type-safe interfaces, validated preconditions, and instrumented debug builds |
| Treat source-file/module visibility as one property | Distinguish lookup, linkage, and ABI/export requirements |

---

## Further reading

The language and standard-library references provide the precise conditions behind the examples. They are sufficient starting points for this chapter; longer bibliographies can be added separately.

### Standard and language reference

* [language/namespace](https://en.cppreference.com/w/cpp/language/namespace)
* [language/adl](https://en.cppreference.com/w/cpp/language/adl)
* [language/using_declaration](https://en.cppreference.com/w/cpp/language/using_declaration)
