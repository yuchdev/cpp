# 13. Undefined Behavior and Its Evolution from C++98 to C++26

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

## 1. Why UB exists

UB is not a compiler bug. It exists to:
- allow aggressive optimizations
- avoid runtime checks
- support low‑level hardware models
- keep the language usable for kernels and runtimes

---

## 2. UB vs. Unspecified vs. Implementation‑Defined

| Category               | Meaning                         |
|------------------------|---------------------------------|
| Undefined behavior     | No guarantees at all            |
| Unspecified behavior   | One of several valid behaviors  |
| Implementation‑defined | Compiler documents the behavior |

---

## 3. How compilers exploit UB

For a path with undefined behavior, the optimizer may reason under the assumption that valid executions do not take that path. A speculative possibility of UB elsewhere is not enough to invalidate a correctly guarded access.

```cpp
int f(int* p) {
    if (!p) return 0;
    return *p;
}
```

The null check makes this example **defined** when `p` is null; the dereference is reached only when the pointer is non-null. Non-null alone is not sufficient for safe dereference: the pointer must still designate a live, accessible object.

---

## 4. How to fight UB

- Compile with sanitizers: `-fsanitize=undefined,address`
- Enable warnings: `-Wall -Wextra -Wpedantic`
- Prefer standard abstractions
- Treat UB as security bugs

---

## 5. Files

* `ub_examples.cpp` - isolated UB examples with detailed comments

---

## 6. Rule

> If your program has UB, the compiler owes you nothing.


## 7. UB cases explained (based on the original educational snippet)

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

## 8. Practical way to study UB
Run one case at a time with sanitizers:

```bash
clang++ -std=c++20 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer ub_examples.cpp -o ub
./ub
```

Sanitizers do not catch *all* UB (e.g., some strict-aliasing miscompilations), but they catch a lot.

---

## 9. Why optimizer reasoning about undefined behavior is more subtle than a crash

Undefined behavior means that a particular execution has no requirements imposed by the C++ abstract-machine rules. It does not mean that a compiler can remove every guard merely because a dangerous operation appears somewhere in the function. The key distinction is whether the bad operation is reachable along the actual execution path and whether the compiler can rely on a violated precondition.

### A prior check of a pointer can be entirely valid

A null check *before* dereferencing a pointer is a perfectly legitimate defensive pattern. It is wrong to claim that a compiler may arbitrarily remove such a check because the program would have undefined behavior *if the pointer were null*.

```cpp
int value_or_zero(const int* p) {
    if (p == nullptr) return 0;
    return *p; // only reached for non-null p
}
```

A null pointer is not dereferenced on the null path. Optimization may simplify code under additional documented assumptions, but ordinary C++ semantics require the branch to protect that access. By contrast, a check *after an unconditional dereference* can be optimized away as redundant on defined executions.

### Signed overflow lets optimization reject impossible paths

For ordinary signed integer addition, overflow is undefined behavior. A compiler may optimize on the premise that a defined program does not overflow, invalidating intuitive attempts to detect overflow by examining a wrapped result.

```cpp
bool greater_after_increment(int x) {
    return x + 1 > x; // true for every defined evaluation
}
```

To test overflow, use precondition checks before adding, a wider type where available, or a checked-arithmetic facility. C++26's `std::ckd_add`, `std::ckd_sub`, and `std::ckd_mul` provide standardized checked integer operations where supported. Do not silently assume that a compiler option such as `-fwrapv` is a cross-platform language rule.

---

## 10. Object lifetimes and aliasing create UB without crossing a buffer boundary

Many low-level errors produce no out-of-bounds address at all. They occur because an object does not exist at the accessed location, because the glvalue type is not allowed to access the object, or because the program incorrectly assumes a memory representation proves a live object lifetime.

### `reinterpret_cast` does not make the target object exist

Casting an address to `T*` is only a pointer conversion; it neither allocates suitably aligned storage nor starts an arbitrary object's lifetime. This matters when parsing packets or implementing custom allocators.

```cpp
alignas(double) unsigned char bytes[sizeof(double)]{};
auto* p = reinterpret_cast<double*>(bytes);
// Dereferencing p is not a general object-creation mechanism.
```

Use actual construction, `std::bit_cast` for equal-size trivially copyable representation conversion, or applicable C++20 implicit-lifetime operations and C++23 `std::start_lifetime_as` with their stated preconditions.

### The pointer can keep the same address after the object dies

Destroying an object ends its lifetime even if the backing allocation remains valid. A stale pointer does not revive the old object; container erasure, vector reallocation, and placement-new storage reuse require careful reassessment of pointer validity.

```cpp
std::vector<int> values{1, 2};
int* p = values.data();
values.push_back(3); // may reallocate: p may now dangle
```

Reserve policies, owning containers, and stable identifiers can prevent some invalidation bugs; an address equality check alone cannot prove object identity after reuse.

---

## 11. Concurrency adds undefined behavior that sanitizers only approximate

A C++ data race arises when conflicting evaluations access the same memory location in different threads without a happens-before relationship and at least one is not atomic, subject to the memory-model rules. The program then has undefined behavior, even if the machine appears to perform indivisible aligned writes.

### Volatile is not a portable thread-synchronization mechanism

`volatile` affects certain evaluations of volatile-qualified objects; it does not provide atomic read-modify-write operations, release/acquire synchronization, or a happens-before edge.

```cpp
std::atomic<bool> stop{false};
// Thread A: stop.store(true, std::memory_order_release);
// Thread B: while (!stop.load(std::memory_order_acquire)) { }
```

The exact memory order may be relaxed for a standalone stop flag depending on the larger protocol. The fundamental requirement is to model the shared state with atomics, mutexes, or another valid synchronization mechanism.

### Atomic pointer publication is not automatic memory reclamation

An atomic pointer can publish an object to another thread, but a reader can still race with deletion after loading the pointer. Hazard pointers, epoch reclamation, read-copy-update designs, and locks solve different parts of the lifetime protocol. C++26 standard hazard-pointer and RCU library facilities increase the vocabulary, but do not retroactively make raw atomic pointers safe.

### ThreadSanitizer findings and limitations

ThreadSanitizer instruments executed accesses and can expose many unprotected data races. It is not a proof of race freedom and may require a specialized runtime, supported platform, and avoidance of unsupported synchronization mechanisms. Pair it with stress tests, invariants, and a clear ownership model.

---

## 12. Compile-time evaluation and unreachable contracts change the location of failures

C++20 `consteval` and later constant-evaluation changes make some invalid operations diagnosable at compile time when they are actually evaluated during constant evaluation. This is different from runtime sanitization; not every potential runtime path is evaluated by the compiler.

### `std::unreachable` in C++23 is a dangerous assertion about control flow

The standard `std::unreachable()` tells the compiler that reaching this point has undefined behavior. Its purpose is to give optimizers a strong assumption, not to implement a safe fallback for unknown enum states or corrupt input.

```cpp
#include <utility> // for other modern utilities
// #include <utility> is NOT the home of std::unreachable;
// include <utility> for std::to_underlying, <utility> notwithstanding.
// C++23: #include <utility> is not sufficient; use <utility> only for its own APIs.
```

Do not use `std::unreachable` to handle untrusted input or an incomplete switch. Instead throw, return an error, or terminate deliberately according to the API. In C++23 its header is `<utility>`? No: use the documented `<utility>`/implementation reference for target library rather than guessing from legacy builtins.

### Contracts introduced by C++26 are not permission to ignore preconditions

C++26 contracts add language mechanisms for expressing preconditions and postconditions, with behavior depending on contract semantics and the selected evaluation mode. The feature does not abolish undefined behavior inside unchecked operations, and a contract annotation cannot make a dangling pointer valid.

Keep contract checks at trust boundaries, but specify how release builds and consumers should behave when input validation fails. Do not confuse an optimizer assumption with a recoverable validation failure.

---

## 13. C++26 uninitialized values distinguish two classes of problematic evaluation

Historically, explanations of uninitialized automatic objects focused on indeterminate values. C++26 introduces additional terminology for *erroneous values*, changing how some uninitialized evaluations are classified without authorizing their ordinary use.

### Reading an unwritten ordinary `int` is still not a valid data source

```cpp
int x;    // no initialization
// int y = x; // do not do this
x = 10;
int y = x; // defined
```

The distinction between undefined and erroneous behavior matters in standard wording and tool reporting. For ordinary application programming, the practical rule is unchanged: initialize values or prove that every execution writes them before use. Byte-oriented types have narrowly specified uninitialized-friendly operations, but those exceptions are not permission to use arbitrary uninitialized data.

### Runtime checks only cover the paths they observe

AddressSanitizer catches many heap/stack/global bounds and lifetime defects. UndefinedBehaviorSanitizer instruments selected undefined operations, and MemorySanitizer detects uses of uninitialized data on supported toolchains. A single run with no report cannot establish that all source paths are defined.

When studying intentionally invalid snippets, isolate one scenario per executable and invoke it only in a controlled sanitizing test environment. Do not run all UB demonstrations in one binary and expect deterministic results after the first invalid operation.

---

## 14. Portability, numerical assumptions, and diagnostic strategy

Some behavior is implementation-defined or unspecified rather than undefined. Confusing those categories leads to false bug reports and weak tests. Standard C++ programs may depend on documented implementation choices, but must not accidentally rely on a choice being universal.

### Floating-point arithmetic is not interchangeable with signed integer overflow

IEEE-style floating overflow often produces infinity when the implementation supports it; signed integer overflow is undefined by core language rules. Floating-point precision, NaNs, trap settings, and fast-math compiler modes are separate concerns and should be tested according to the actual numerical contract.

### Compiler flags can mask bugs while preserving non-portability

`-fno-strict-aliasing` alters some optimization assumptions but does not start object lifetimes or fix alignment. `-fwrapv` on supported compilers supplies a particular signed-overflow interpretation, but the ISO C++ rule remains different. Building with `-O0` can change symptoms while leaving the invalid source unchanged.

For senior-level debugging, compile the smallest reproducer under at least two optimization levels and two compilers where practical. Treat sanitizer results, generated assembly, and the abstract-machine rules as complementary evidence—not as interchangeable authorities.

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

### C++26: Checked arithmetic, contracts, and erroneous values

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
