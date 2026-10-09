# 13. Undefined Behavior in C and C++

This chapter is dedicated to Undefined Behavior (UB) - one of the most misunderstood,
dangerous, and performance‑critical aspects of C and C++.

UB means:
> *The C++ standard imposes no requirements on what happens.*

That includes:
- crashes
- seemingly "working" behavior
- silent data corruption
- optimizer removing your code entirely

---

## Why UB exists

UB is not a compiler bug. It exists to:
- allow aggressive optimizations
- avoid runtime checks
- support low‑level hardware models
- keep the language usable for kernels and runtimes

---

## UB vs. Unspecified vs. Implementation‑Defined

| Category               | Meaning                         |
|------------------------|---------------------------------|
| Undefined behavior     | No guarantees at all            |
| Unspecified behavior   | One of several valid behaviors  |
| Implementation‑defined | Compiler documents the behavior |

---

## How compilers exploit UB

If the compiler proves UB *could* happen, it may assume it never happens.

```cpp
int f(int* p) {
    if (!p) return 0;
    return *p;
}
```

If `p` is ever null → UB → compiler may remove the check entirely.

---

## How to fight UB

- Compile with sanitizers: `-fsanitize=undefined,address`
- Enable warnings: `-Wall -Wextra -Wpedantic`
- Prefer standard abstractions
- Treat UB as security bugs

---

## Files

* `ub_examples.cpp` - isolated UB examples with detailed comments

---

## Rule

> If your program has UB, the compiler owes you nothing.


## UB cases explained (based on the original educational snippet)

Below is a short explanation for each UB pattern from your original file.  
Rule of thumb: the moment UB occurs, the whole program becomes unconstrained - even code "after" the UB may be optimized away.

### 1) `dereference_null()` - dereferencing a null pointer
What happens: `*ptr` is UB because a null pointer does not point to a valid object.  
Why it's UB: a pointer must be *dereferenceable*; `nullptr` never is.  
Typical outcomes: crash, "works" in debug, miscompilation in release.  
Fix: check before use; prefer references/RAII when ownership is clear.

### 2) `buffer_overflow()` - out-of-bounds write
What happens: `buffer[10] = 'A'` writes past a 5-byte array.  
Why it's UB: array indexing is only valid within bounds; out-of-bounds writes corrupt unrelated objects.  
Typical outcomes: silent corruption, security bugs, optimizer assuming it never happens.  
Fix: bounds checks; prefer `std::array`, `std::span`, `.at()` for checked access.

### 3) `signed_overflow()` - signed integer overflow
What happens: `max + 1` overflows `int`.  
Why it's UB: signed overflow is undefined (unlike unsigned, which wraps modulo 2^N).  
Typical outcomes: "impossible" branches removed, wrong loops/comparisons.  
Fix: widen types, check overflow, or use unsigned if a wrap is intended.

### 4) `division_by_zero()` - integer divide by zero
What happens: `i / 0`.  
Why it's UB: division by zero has no defined result.  
Typical outcomes: crash (SIGFPE), trap, random result.  
Fix: validate denominators; define domain behavior (error/optional/exception).

### 5) `uninitialized_var()` - reading an uninitialized automatic variable
What happens: prints `i` without initializing it.  
Why it's UB: the value is indeterminate; reading it is undefined.  
Fix: initialize (`int i{};`) and enable warnings/sanitizers.

### 6) `array_out_of_bounds()` - out-of-bounds read
What happens: reads `array[5]` from `int array[5]`.  
Why it's UB: element 5 does not exist; even reads can violate object bounds assumptions.  
Fix: bounds checks; use `.at()` for teaching/debug.

### 7) `expired_pointer()` - dangling pointer to a dead local
What happens: a pointer refers to `x`, then `x` goes out of scope.  
Why it's UB: object lifetime ended; dereference is invalid.  
Fix: never return/store pointers to locals; copy the value or allocate with ownership.

### 8) `type_punning_pointer()` - strict-aliasing violation via `reinterpret_cast`
What happens: reads an `int` object as `float`.  
Why it's UB: violates strict aliasing (and can violate alignment).  
Fix: `std::memcpy` (C++11+) or `std::bit_cast` (C++20) for a bit of reinterpretation.

### 9) `evaluation_order()` - using a variable in its own initializer
What happens: `int i = i * 0;` reads `i` before it's initialized.  
Why it's UB: direct uninitialized read (not merely "order of evaluation").  
Fix: initialize first (`int i = 0;`).

### 10) `deleted_pointer()` - use-after-delete
What happens: dereference after `delete`.  
Why it's UB: object lifetime ended; memory may be reused.  
Fix: avoid manual `new/delete`; use `std::unique_ptr`/`std::shared_ptr`.

### 11) `call_non_exist_virtual()` - virtual call after delete
What happens: calls `ptr->foo()` after `delete ptr`.  
Why it's UB: use-after-free; virtual dispatch relies on vptr/vtable that no longer exists.  
Fix: never use pointers after delete; use smart pointers and clear ownership rules.

### 12) `dangling_reference()` - reference to a freed object
What happens: reference `ref` outlives the pointee.  
Why it's UB: references to don't rebind; they dangle if the object dies.  
Fix: don't bind refs to owned dynamic objects unless lifetime is guaranteed.

### 13) `realloc_fails()` - mixing allocation families (`new` with `realloc`)
What happens: passing `new`-allocated memory to `realloc`.  
Why it's UB: `realloc` only works on pointers from `malloc/calloc/realloc`.  
Fix: use `std::vector`/`std::string` for resizing; or stick to `malloc/realloc/free` consistently.

### 14) `misalignment()` - misaligned access
What happens: cast `char*` to `int*` and dereference.  
Why it's UB: the pointer may not meet `alignof(int)`; misaligned dereference can trap.  
Fix: allocate as `int`, or use aligned allocation APIs; don't "invent" alignment by casting.

### 15) `break_strict_aliasing()` - strict aliasing again
Same issue as #8.  
Fix: `std::memcpy` or `std::bit_cast` (C++20).

### 16) `missed_return()` - falling off the end of a non-void function
What happens: not all control paths return a value.  
Why it's UB: caller expects an `int` value; none is produced.  
Fix: return on all paths; enable `-Wreturn-type`.

---

## Practical way to study UB
Run one case at a time with sanitizers:

```bash
clang++ -std=c++20 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer ub_examples.cpp -o ub
./ub
```

Sanitizers do not catch *all* UB (e.g., some strict-aliasing miscompilations), but they catch a lot.

---

## Rules worth keeping in working memory

The controlling distinction for undefined behavior is between the portable language contract and the behavior of a particular compilation environment. C++ gives programmers the ability to build close to system boundaries, but also requires those boundaries to be specified rather than guessed.

> A source declaration states a C++ contract; neither a plausible runtime result nor a successful build can silently strengthen that contract.

### Core rules

These rules are a compact form of the chapter's essential reasoning:

1. **Differentiate undefined, unspecified, and implementation-defined behavior.**
2. **A successful execution is not a proof that code has no UB.**
3. **Treat sanitizers as partial detection tools, not specification.**
4. **Prefer legal object access, checked bounds, and explicit lifetime management.**

### Pitfalls at a glance

Both failures below are attractive shortcuts precisely because they may seem to work on one toolchain.

| Pitfall | What happens | Do instead |
|---|---|---|
| Reading outside an array | Undefined behavior, often memory corruption | Track bounds |
| Signed overflow or aliasing violation | Optimizer can exploit forbidden assumptions | Validate before arithmetic and use legal representation APIs |

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

Before publishing or refactoring undefined behavior code, ask:

- [ ] Have we stated the preconditions and the relevant lifetime or linkage rules?
- [ ] Can another translation unit or toolchain use this interface without undocumented assumptions?
- [ ] Have we distinguished a guaranteed rule from an implementation observation?
- [ ] Are relevant boundary and negative cases represented in tests?

---

## Diagnostics, useful compiler settings and extensions

The compiler can identify many malformed uses of undefined behavior, while some errors only surface at a link step, in a different build configuration, or along an executed path. Diagnostics complement, but cannot replace, correct declarations and contracts.

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

The standards preserve the core rules of undefined behavior while expanding ways to state interfaces more precisely. The milestones matter when modernizing an older codebase across several compiler modes.

### C++98/03: Original model

Undefined and implementation-defined behaviors form part of the basic abstract machine.

### C++11: New language vocabulary

Move/lifetime idioms create new sites requiring careful analysis.

### C++14: Incremental refinement

Relaxed constexpr can catch more problems in required constant expressions.

### C++17: Library evolution

Sequencing rules were strengthened for several expressions.

### C++20: Modern interfaces

`std::bit_cast` and `span` offer safer alternatives to common tricks.

### C++23: Broader facilities

Further constexpr/lifetime clarifications refine diagnostics.

### C++26: Forthcoming support

Safety facilities evolve, but UB does not universally become a runtime exception.

Annotate version requirements explicitly in source and build definitions; current compiler behavior alone is not a version contract.

---

## Migration note for Java / Python / C# developers

Managed runtimes frequently throw exceptions for bounds, invalid casts, and lifetime misuse. C++ deliberately leaves some invalid operations undefined, so the optimizer may transform surrounding code without preserving what one machine happened to do. A managed-language analogy is useful for identifying an intent, but not for predicting C++ storage lifetime, ABI layout, or failure semantics.

### Where intuition transfers and where it breaks

This comparison highlights why translating syntax directly is less reliable than translating contracts.

| Concern | Java | Python | C# | C++ |
|---|---|---|---|---|
| Runtime loading | JVM class loading | Import system | CLR assemblies | Translation units, linker, and platform loader |
| Object lifetime | GC | Reference counts/GC | GC | Determined by storage duration and ownership |
| Invalid operations | Frequently exceptions | Frequently exceptions | Frequently exceptions | May be diagnosed, defined, or undefined |
| undefined behavior | Managed runtime conventions | Dynamic language conventions | CLR/runtime conventions | Explicit language and ABI contracts |

The main difference is that C++ exposes several boundaries which other runtimes coordinate automatically.

### Common wrong assumptions

It is unsafe to assume that names in different source files are automatically linked, that a non-owning pointer keeps its pointee alive, or that an invalid low-level operation will throw an exception. In C++ these are separate questions. A successful test under one compiler confirms only the observed execution; it does not confer a language guarantee or validate every ABI configuration.

### Idiomatic C++ replacement

The migration is clearest when expressed in terms of intent rather than translated keywords.

| Managed-language habit | Idiomatic C++ |
|---|---|
| Relying on an exception for every invalid access | Use checked abstractions, assertions, and sanitizers for debugging |
| Expect runtime checks to catch every misuse | Use type-safe interfaces, validated preconditions, and instrumented debug builds |
| Treat source-file/module visibility as one property | Distinguish lookup, linkage, and ABI/export requirements |

---

## Further reading

The language and standard-library references provide the precise conditions behind the examples. They are sufficient starting points for this chapter; longer bibliographies can be added separately.

### Standard and language reference

* [language/ub](https://en.cppreference.com/w/cpp/language/ub)
* [language/lifetime](https://en.cppreference.com/w/cpp/language/lifetime)
* [language/eval_order](https://en.cppreference.com/w/cpp/language/eval_order)
