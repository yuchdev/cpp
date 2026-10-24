# 12. C and C++ Compatibility from C++98 to C++26

This chapter focuses on **principal language and toolchain differences** that matter when you:
- build mixed C/C++ projects
- wrap C libraries in C++
- expose C++ APIs to C code
- reason about initialization, ABI, and linkage
- migrate code between the languages

Some files are **C-only** (`.c`) and some are **C++** (`.cpp`)

## 1. Files

1. `01_name_mangling_and_overload` examples
   - C++ name mangling, why overloads work
   - how to inspect symbols with `nm` / `objdump`
   - why C cannot overload by signature

2. `02_extern_c_callbacks` examples
   - `extern "C"`: linkage and ABI
   - C-callable functions exported from C++
   - callbacks/function pointers across the boundary
   - pitfalls: exceptions, C++ objects across ABI

3. `03_static_init_and_main` examples
   - C++ startup: global constructors run **before `main`**
   - C startup: no user constructors/destructors
   - init-order fiasco + safe patterns

4. `04_headers_and_types` examples
   - `<string.h>` vs `<cstring>` mapping
   - `void*` conversions, `malloc` in C vs C++
   - `bool` vs `_Bool`, `nullptr` vs `NULL`
   - `restrict` in C99 and portable C++ replacements

5. `05_c_things_not_in_cpp` examples
   - Valid C features that are ill-formed in standard C++:
     VLAs, compound literals, array designators, `restrict`, implicit `void*` conversion,
     C struct-tag differences.

6. `06_cpp_things_not_in_c` examples
   - C++ features that C doesn't have (RAII/classes/templates/etc.)
   - a correct "C ABI facade" pattern: opaque handles + create/destroy + error codes

## 2. ABI safety checklist for mixed projects

- `extern "C"` changes **linkage / mangling**, not C++ semantics.
- Never let C++ exceptions cross into C: catch inside and return error codes.
- Never expose `std::string`, `std::vector`, or C++ classes in a C ABI.
- Prefer an opaque-handle API: `create/destroy`, `int` status codes, POD structs.

---

## 3. C and C++ are different languages sharing an ABI only by agreement

The shared heritage of C and C++ makes mixed-language projects seem simpler than they are. Compiling the same header as C17 and C++20 can produce different types, different name lookup, different initialization, and different interpretations of an expression. The interoperability contract is therefore an explicit interface between two languages, not an assumption that C++ is a strict superset of C.

## 4. C linkage affects language linkage, not everything the ABI requires

Declaring a function `extern "C"` gives it C language linkage, including rules governing its name and function type. This does not guarantee identical calling conventions across all targets, compatible definitions of C and C++ structs, or a stable exception ABI.

```cpp
extern "C" int codec_decode(const unsigned char* bytes,
                             std::size_t length) noexcept;
```

The corresponding C header must use declarations legal in C, such as `size_t` from `<stddef.h>`. Avoid putting `noexcept` in a shared C declaration; use conditional macros guarded by `__cplusplus`. An exported function catches every C++ exception internally and reports errors through a C-compatible result.

## 5. The same `struct` layout is a requirement to verify, not assume

C and C++ often agree on the layout of simple record types under a matching target ABI. But a C++ class with virtual functions, nonstandard layout, inheritance, or implementation-specific packing is not a candidate for direct C interchange.

```c
/* header visible to C and C++ */
typedef struct device_packet {
    unsigned int id;
    unsigned char mode;
} device_packet;
```

Define the shared ABI in terms of exact available fundamental or fixed-width types, packing/alignment rules, and size/version fields. Check `sizeof`, `alignof`, and `offsetof` on every supported target. For long-lived disk/network interchange, serialize fields explicitly instead of copying raw struct bytes.

## 6. Allocation and deallocation must be paired across the boundary

C's `malloc/free` allocation model and C++'s `new/delete` model are not interchangeable, especially when constructors and destructors are involved. A library that hands a resource to C callers must export the matching destroy operation; the caller should not guess the allocator family.

```c
typedef struct codec_handle codec_handle;
codec_handle* codec_create(void);
void codec_destroy(codec_handle*);
```

The C++ implementation can use RAII internally while maintaining an opaque externally visible handle. This also shields the API from changes in internal class layout and C++ standard library implementation.

## 7. Function pointers and callbacks need a deliberate lifetime protocol

A C callback typically consists of a function pointer plus a context pointer. A capturing C++ lambda cannot implicitly become a C function pointer; a noncapturing lambda converts to a pointer to a function with C++ language linkage, so it is not a portable match for a C-linkage callback type, even if common compilers accept it. When the callback contract requires C linkage, use a named `extern "C"` trampoline.

```c
typedef void (*on_result)(void* context, int status);
void codec_register(codec_handle*, on_result, void* context);
```

Never allow a callback to access an already-destroyed context. Document whether callbacks can run concurrently, after unregister, or during destruction; those decisions are more important than the lexical spelling of the callback declaration.

## 8. Exceptions, RTTI, and standard-library objects must stay inside the C++ side

A C caller cannot portably catch a C++ exception or understand `std::string`, `std::vector`, `std::shared_ptr`, or a C++ polymorphic object's ABI. Catch exceptions at each C-exported entry point, record diagnostics, and return an integer error code plus an explicit output buffer or handle.

This is not just conservatism: differing compiler versions, standard libraries, runtime settings, and exception models can disagree about object layout, allocation, and exception unwinding. A stable C facade permits the C++ implementation to evolve without requiring every consumer to share its toolchain.

## 9. C99 VLAs and compound literals are not ordinary C++ syntax

A locally declared C variable-length array has automatic storage duration; ISO C does not prescribe a stack allocation mechanism. Standard C++ does not provide C99 VLAs. Some GCC and Clang C++ modes accept them as extensions, which can accidentally turn portable-looking code into vendor-specific code.

```c
/* C99, not standard C++ */
void compute(int n) {
    int scratch[n];
    use(scratch);
}
```

A C compound literal also differs from C++ temporary-expression rules, including lifetime. Prefer `std::vector` or a bounded owner for dynamic C++ arrays, and explicitly manage the lifetime of any temporary view created while adapting a C API.

## 10. C99 `restrict` has no ISO C++ spelling

C's `restrict` is a pointer-aliasing contract with specific block execution semantics. C++20/23/26 do not include a portable `restrict` keyword; compilers offer `__restrict` or `__restrict__` extensions with toolchain-specific meaning.

Passing two overlapping buffers to an API that assumes non-aliasing can invalidate optimizer assumptions. Prefer documenting actual disjointness preconditions and encapsulating compiler extensions rather than simply copying a C `restrict` qualifier into a shared header.

## 11. C and C++ disagree about implicit conversions and type names

C traditionally permits implicit conversion from `void*` to object pointers, while C++ requires a cast. C also has a distinct `struct` tag namespace, and C++ supports class names differently. These differences affect macros, generic API shims, and the shape of headers compiled by both compilers.

```c
/* C permits this, standard C++ rejects the implicit conversion */
int* p = malloc(100 * sizeof *p);
```

C++ should normally avoid raw `malloc` for objects requiring construction. A shared header can expose C-compatible opaque pointers while implementing typed ownership in the C++ translation unit.

## 12. Compound type qualifiers and atomics have different models

C11 `_Atomic` and C++11 `std::atomic<T>` are not universally interchangeable ABI types. Their spelling, type requirements, initialization, and layout are not guaranteed to coincide, even when the platform uses the same underlying instructions.

Do not expose an atomic object directly through a supposedly portable C/C++ boundary. Instead export operations that synchronize internally, or agree on a platform-specific ABI and document toolchain assumptions.

## 13. C23 and C++23 share familiar tokens without sharing the whole language

C23 introduced or standardized several spellings familiar to C++ developers, but C23's language rules are not C++23 rules. In mixed build systems compile `.c` sources with a chosen C standard and `.cpp` with a chosen C++ standard; do not expect one set of flags or feature-test macros to cover both languages.

For example, `nullptr` and boolean support have evolving C definitions and availability, while C++ treats them through its own language type rules. Cross-language headers must explicitly conditionally declare facilities rather than assuming spellings imply binary compatibility.

## 14. C++20 modules do not make C headers into portable modules automatically

A C header has textual macro and declaration semantics. Importing a C++20 header unit or including that header in the global module fragment changes build architecture but does not eliminate underlying ABI issues, macro dependencies, or the need to compile C code with a C compiler.

A safe migration starts from a C-facing header that remains valid C, then wraps it behind C++ classes and module interfaces. The wrapper owns resource handles and translates errors, callbacks, and strings across the boundary.

## 15. C++26 reflection is not a shared ABI protocol

Static reflection can help generate code around C++ types, but its existence does not automatically define a stable representation for C consumers, nor does it make C++ classes safe C structs. Use generated bindings only alongside a versioned C schema, explicit ownership rules, and ABI tests on all intended compilers.

---

## 16. Cross-language review checklist for production APIs

A C ABI should define allocator ownership, error handling, callback lifetime, thread-safety, calling convention, alignment, scalar widths, and versioning. For a plugin interface, keep an explicit versioned function table or handle-based API rather than passing compiler-specific C++ virtual objects through a foreign implementation.

When in doubt, test the same public header under both C17/C23 and C++20/C++23 and compare binary exports with `nm`, `readelf`, and `dumpbin`. Runtime integration tests must exercise error paths and teardown paths, not only successful calls.

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

### C++26: Reflection does not define an ABI

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
