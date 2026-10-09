# 12. C/C++ Compatibility

This chapter focuses on **principal language and toolchain differences** that matter when you:
- build mixed C/C++ projects
- wrap C libraries in C++
- expose C++ APIs to C code
- reason about initialization, ABI, and linkage
- migrate code between the languages

Some files are **C-only** (`.c`) and some are **C++** (`.cpp`)

## Files

1. `01_name_mangling_and_overload.cpp`
   - C++ name mangling, why overloads work
   - how to inspect symbols with `nm` / `objdump`
   - why C cannot overload by signature

2. `02_extern_c_and_callbacks.cpp`
   - `extern "C"`: linkage and ABI
   - C-callable functions exported from C++
   - callbacks/function pointers across the boundary
   - pitfalls: exceptions, C++ objects across ABI

3. `03_static_init_and_main.cpp`
   - C++ startup: global constructors run **before `main`**
   - C startup: no user constructors/destructors
   - init-order fiasco + safe patterns

4. `04_headers_and_types.cpp`
   - `<string.h>` vs `<cstring>` mapping
   - `void*` conversions, `malloc` in C vs C++
   - `bool` vs `_Bool`, `nullptr` vs `NULL`
   - `restrict` in C99 and portable C++ replacements

5. `05_c_things_not_in_cpp.c`
   - Valid C features that are ill-formed in standard C++:
     VLAs, compound literals, array designators, `restrict`, implicit `void*` conversion,
     C struct-tag differences.

6. `06_cpp_things_not_in_c.cpp`
   - C++ features that C doesn't have (RAII/classes/templates/etc.)
   - a correct "C ABI facade" pattern: opaque handles + create/destroy + error codes

## ABI safety checklist for mixed projects

- `extern "C"` changes **linkage / mangling**, not C++ semantics.
- Never let C++ exceptions cross into C: catch inside and return error codes.
- Never expose `std::string`, `std::vector`, or C++ classes in a C ABI.
- Prefer an opaque-handle API: `create/destroy`, `int` status codes, POD structs.

---

## Rules worth keeping in working memory

The controlling distinction for C/C++ interoperability is between the portable language contract and the behavior of a particular compilation environment. C++ gives programmers the ability to build close to system boundaries, but also requires those boundaries to be specified rather than guessed.

> A source declaration states a C++ contract; neither a plausible runtime result nor a successful build can silently strengthen that contract.

### Core rules

These rules are a compact form of the chapter's essential reasoning:

1. **Compile C code with a C compiler when C semantics matter.**
2. **Use `extern "C"` for language linkage only, not as a layout guarantee.**
3. **Contain exceptions and C++ object ownership behind an ABI facade.**
4. **Document allocator matching and binary data boundaries.**

### Pitfalls at a glance

Both failures below are attractive shortcuts precisely because they may seem to work on one toolchain.

| Pitfall | What happens | Do instead |
|---|---|---|
| Exposing `std::string` in C ABI | No stable C object layout contract | Use buffer and length |
| Throwing C++ exceptions across a C boundary | Unspecified interoperability and unsafe unwinding | Translate errors to status codes |

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

Before publishing or refactoring C/C++ interoperability code, ask:

- [ ] Have we stated the preconditions and the relevant lifetime or linkage rules?
- [ ] Can another translation unit or toolchain use this interface without undocumented assumptions?
- [ ] Have we distinguished a guaranteed rule from an implementation observation?
- [ ] Are relevant boundary and negative cases represented in tests?

---

## Diagnostics, useful compiler settings and extensions

The compiler can identify many malformed uses of C/C++ interoperability, while some errors only surface at a link step, in a different build configuration, or along an executed path. Diagnostics complement, but cannot replace, correct declarations and contracts.

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

The standards preserve the core rules of C/C++ interoperability while expanding ways to state interfaces more precisely. The milestones matter when modernizing an older codebase across several compiler modes.

### C++98/03: Original model

C and C++ already differ in type checking despite C-compatible linkage.

### C++11: New language vocabulary

RAII/move semantics improve C++ ownership wrappers.

### C++14: Incremental refinement

No material change to the C ABI contract.

### C++17: Library evolution

No material change.

### C++20: Modern interfaces

`std::span` can wrap C pointer/length inputs inside C++.

### C++23: Broader facilities

Modern library vocabulary still belongs behind the facade.

### C++26: Forthcoming support

Future language/library changes cannot remove the need for an explicit ABI contract.

Annotate version requirements explicitly in source and build definitions; current compiler behavior alone is not a version contract.

---

## Migration note for Java / Python / C# developers

Java JNI, Python native extensions, and C# P/Invoke already require explicit foreign interfaces. A C++ wrapper around a C ABI still has to manage layout, calling conventions, allocation, and error translation; `extern "C"` only addresses language linkage. A managed-language analogy is useful for identifying an intent, but not for predicting C++ storage lifetime, ABI layout, or failure semantics.

### Where intuition transfers and where it breaks

This comparison highlights why translating syntax directly is less reliable than translating contracts.

| Concern | Java | Python | C# | C++ |
|---|---|---|---|---|
| Runtime loading | JVM class loading | Import system | CLR assemblies | Translation units, linker, and platform loader |
| Object lifetime | GC | Reference counts/GC | GC | Determined by storage duration and ownership |
| Invalid operations | Frequently exceptions | Frequently exceptions | Frequently exceptions | May be diagnosed, defined, or undefined |
| C/C++ interoperability | Managed runtime conventions | Dynamic language conventions | CLR/runtime conventions | Explicit language and ABI contracts |

The main difference is that C++ exposes several boundaries which other runtimes coordinate automatically.

### Common wrong assumptions

It is unsafe to assume that names in different source files are automatically linked, that a non-owning pointer keeps its pointee alive, or that an invalid low-level operation will throw an exception. In C++ these are separate questions. A successful test under one compiler confirms only the observed execution; it does not confer a language guarantee or validate every ABI configuration.

### Idiomatic C++ replacement

The migration is clearest when expressed in terms of intent rather than translated keywords.

| Managed-language habit | Idiomatic C++ |
|---|---|
| Passing managed objects to native code | Expose opaque handles and plain buffers with explicit lifetime |
| Expect runtime checks to catch every misuse | Use type-safe interfaces, validated preconditions, and instrumented debug builds |
| Treat source-file/module visibility as one property | Distinguish lookup, linkage, and ABI/export requirements |

---

## Further reading

The language and standard-library references provide the precise conditions behind the examples. They are sufficient starting points for this chapter; longer bibliographies can be added separately.

### Standard and language reference

* [language/language_linkage](https://en.cppreference.com/w/cpp/language/language_linkage)
* [language/extern](https://en.cppreference.com/w/cpp/language/extern)
* [language/types](https://en.cppreference.com/w/cpp/language/types)
