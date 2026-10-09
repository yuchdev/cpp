# 06. C++ Arrays - Decay, Types, and Modern Alternatives

## Roadmap

### 01. Array decay, `sizeof`, and overloads

Key ideas:

* Arrays are objects with a real size, but in *most* expressions they **decay to a pointer to their first element**.
* The important non-decay cases you should remember:

  1. `sizeof(arr)` sees the whole array object
  2. `&arr` yields a **pointer-to-array** (`T (*)[N]`), not `T*`
  3. `decltype(arr)` for an unparenthesized id-expression preserves array type
  4. string literal initialization into `char[]` has special rules
  5. binding to a reference-to-array preserves type and size

What this example shows:

* `int a[5]; int* p = a;` works via decay.
* `int (*pa)[5] = &a;` is a pointer to the whole array.
* `p + 1` advances by `sizeof(int)` while `pa + 1` advances by `sizeof(a)`.
* Overload resolution prefers binding to `const int(&)[N]` over decaying to `const int*`.
* A function parameter `int x[]` is **exactly** `int* x`.
* The classic pitfall: `sizeof(a_param)` inside a function gives pointer size.

### 02. Templates, deduction of `N`, and array type traits


Key ideas:

* `T (&)[N]` is the canonical "array-preserving" parameter type.
* You can **deduce `N`** and keep array size at compile time.
* `auto&` preserves array-ness and constness when binding to an array.
* `decltype(a)` on a plain name `a` retains the array type (`int[4]`).

Type-trait tools covered:

* `std::extent<T>`: size of a given dimension
* `std::rank<T>`: number of dimensions
* `std::remove_extent<T>`: peel one dimension (e.g., `int[2][5] -> int[5]`)

### 03. Initialization rules and string literal facts

Key ideas:

* `int a[5] = {};` value-initializes all elements to zero.
* `int b[5] = {1,2};` zero-fills the tail.
* `int c[5];` is **uninitialized** for automatic storage (reading it is UB).
* Brace-initialization prevents narrowing in C++11+.

String literal specifics:

* A string literal like `"ABC"` is an **lvalue** of type `const char[4]` (includes the `\0`).
* `decltype("ABC")` is a reference type (`const char(&)[4]`) because `decltype(lvalue)` yields `T&`.
* `const char* p = "Hello";` stores a pointer (and `sizeof(p)` is pointer size).
* `char s1[] = "Hi";` creates an array sized to include the terminator.

### 04. Multi-dimensional arrays and pointer-to-array types

Key ideas:

* `int m[2][3]` is a *single contiguous object* laid out row-major.
* `&m[0][0]` is a plain `int*` view of contiguous storage.
* `m` decays to a pointer to its first row: `int (*)[3]`.
* `&m` is a pointer to the full matrix object: `int (*)[2][3]`.

Passing 2D arrays:

* The element type of `m` is `int[3]`, so the compiler must know the second dimension.
* You can accept a fixed shape (`const int (&)[2][3]`) or template on `(R, C)`.
* `int**` is a pointer-to-pointer and does **not** describe a contiguous 2D array.

### 05. Memory, `new[]/delete[]`, and typedef pitfalls

Key ideas:

* Arrays allocated with `new T[N]` must be freed with `delete[]`.
* Mixing `new[]` with `delete` is undefined behavior.

The subtle trap:

* `typedef int Week[7];` hides an array type.
* `new Week` allocates an array of 7 ints, but in many contexts you'll see it as `int*` due to decay.
* It becomes easier to accidentally:

  * use the wrong delete form
  * pass it around without size information

Also highlighted:

* Deleting an array through a base pointer (`Base* b = new Derived[2]; delete[] b;`) is a classic UB pattern.
  Even with virtual destructors, the array cookie/layout expectations can mismatch.

### 06. Modern helpers: raw arrays made safer, plus `std::array`

Key ideas:

* `std::begin(a)` / `std::end(a)` work for raw arrays in C++11+.
* Range-for iterates raw arrays **without decay**.
* `std::array<T, N>` is a thin wrapper around `T[N]`:

  * does not decay
  * carries its size
  * is copyable/assignable
  * provides `.data()` for C interop
  * supports `std::array<T, 0>`

---
---

## Rules worth keeping in working memory

The central distinction in arrays and contiguous views is between what the C++ type system guarantees, what the implementation commonly does, and what an API merely assumes. Carrying that distinction from declarations through operations avoids most of the surprises discussed above.

> A valid low-level operation needs a language-level contract, not just a machine-level outcome that looks plausible.

### Core rules

The following rules condense the chapter into reviewable decisions:

1. **A raw `T[N]` is an array type; decay to `T*` loses extent.**
2. **Remember that parameter spelling `T a[N]` adjusts to `T*`.**
3. **Keep multidimensional row types distinct from `T**`.**
4. **Use `std::array` for fixed-size value semantics and `std::span` for borrowed contiguous access.**


### Pitfalls at a glance

These recurring failures are particularly useful to recognize in code review because each starts from a plausible but insufficient assumption.

| Pitfall | What happens | Do instead |
|---|---|---|
| `sizeof` on array parameter | Measures pointer, not entire array | Use `std::span` or an array reference |
| `new[]` paired with `delete` | Undefined behavior | Use `std::vector` or `unique_ptr<T[]>` |
| `T[R][C]` treated as `T**` | Wrong layout and stride | Preserve row extent or use `mdspan` |

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

Use this checklist when changing arrays and contiguous views code or reviewing low-level interfaces:

- [ ] Have we distinguished language guarantees from platform-specific observations?
- [ ] Are all input domains and conversion or lifetime preconditions explicit?
- [ ] Can the relevant edge cases be tested without executing undefined behavior?
- [ ] Does the chosen API encode as much of the intended contract as practical?

---

## Diagnostics, useful compiler settings and extensions

Compilers can diagnose many suspicious uses of arrays and contiguous views, but they cannot infer every application invariant. A clean warning build is useful evidence, not proof: tools can recognize some invalid expressions statically and others only when particular paths execute.

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

The core model of arrays and contiguous views evolved incrementally; newer standards add safer vocabulary without retroactively rewriting all legacy expressions. The milestones below separate changes to the language from changes to available library interfaces.

### C++98/03: The original model

Raw arrays, decay, adjusted parameters, and array new/delete are inherited fundamentals.

### C++11: Stronger types and interfaces

`std::array`, `std::begin`, and range-for improve fixed-size use.

### C++14: Incremental refinement

`make_unique<T[]>` simplifies ownership.

### C++17: Library and deduction evolution

`std::size`, `std::data`, and deduction improvements reduce boilerplate.

### C++20: Modern vocabulary

`std::span`, `std::to_array`, bounded-array traits, and ranges preserve usable extent.

### C++23: Further standard facilities

`std::mdspan` adds multidimensional non-owning mapping.

### C++26: Emerging improvements

Checked span access and multidimensional slicing improve safety, subject to implementation support.

When documenting a facility, state its required language/library version rather than inferring support from the compiler's branding.

---

## Migration note for Java / Python / C# developers

Readers familiar with managed runtimes often expect arrays and contiguous views to come with runtime metadata, automatic lifetime management, or checked failures. In C++, some of those services come from a chosen library type, while raw language mechanisms may deliberately expose more responsibility to the caller. The important translation is from a *runtime guarantee* in one language to a *type, contract, and lifetime guarantee* in C++.

### Where intuition transfers and where it breaks

The same apparent operation may have a different failure mode or storage model in each language.

| Concept | Java | Python | C# | C++ reality |
|---|---|---|---|---|
| arr | bounds-checked array object | Arrays with runtime checks | List/array objects | Array, `Span<T>` |
| Invalid access/operation | Usually throws or is checked | Usually raises an exception | Often throws in safe code | May be ill-formed, defined, unspecified, or undefined depending on the operation |
| Lifetime | Managed object reachability | Reference counting / GC | Managed GC | Automatic, dynamic, and explicitly borrowed lifetimes coexist |

The distinction matters at API boundaries: a familiar surface syntax does not imply familiar failure behavior.

### Common wrong assumptions

One tempting assumption is that a successful local test demonstrates that an operation is safe. For the low-level C++ rules in this chapter, a test can demonstrate behavior of one build, but cannot establish portability or rule out undefined behavior. Another is that an object is kept alive by every handle referring to it; non-owning pointers, references, and views do not do that. Finally, a managed-language exception should not be presumed to exist at an equivalent C++ failure point.

The earlier chapter's comparison is worth retaining in this context: If you come from Python/Java/C#, C++ arrays can feel "unfair" at first. The mental shift is:

In many languages, an "array" is a **first-class runtime object** (size, bounds, metadata, often heap-based).
In C++, a raw array `T[N]` is a **compile-time-sized object** with *no runtime metadata*. It's closer to "a struct containing N elements" than to a high-level collection.

What commonly surprises people:

**The decay rule**: passing an array often silently turns it into a pointer, losing size information.
**`int x[]` in parameters is not an array**: it's a pointer. This is one of the oldest compatibility features in C/C++.
**`sizeof` is context-sensitive**: `sizeof(a)` is the full array size; `sizeof(param)` in a function is pointer size.
**2D arrays are not `T**`**: `T[R][C]` is contiguous, while `T**` usually describes a "jagged" structure with separate allocations.
**Manual memory is real**: if you use `new[]`, you must match it with `delete[]` (and in modern C++, you usually avoid `new` entirely).

Practical migration advice:

Prefer **`std::array`** for fixed-size arrays and **`std::vector`** for dynamic size.
For APIs that need "pointer + length", prefer **`std::span`** (C++20) or pass `(ptr, size)` explicitly.
Treat raw arrays and pointer arithmetic as a **low-level interop/performance tool**, not your default data structure.

Once you internalize that raw arrays are *types with shape* (`T[N]`, `T[R][C]`) rather than "objects with metadata", the rules become consistent-and your C++ becomes both safer and more predictable.

### Idiomatic C++ replacement

The following mappings help make intent explicit without imitating a managed runtime mechanically.

| Habit from managed languages | Idiomatic C++ |
|---|---|
| Fixed extent value | Prefer `std::array<T,N>` |
| Borrowed contiguous sequence | Prefer C++20 `std::span<T>` |
| Expect automatic runtime bounds/lifetime checks | Select an owning container or checked API; validate preconditions explicitly |

Use these choices because they express the program's requirements, not merely because they resemble familiar constructs from another language.

---

## Further reading

These references document the underlying language rules and library contracts. They are starting points for checking precise preconditions; the chapter's examples explain how the rules interact.

### Standard and language reference

* [language/array](https://en.cppreference.com/w/cpp/language/array)
* [container/array](https://en.cppreference.com/w/cpp/container/array)
* [container/span](https://en.cppreference.com/w/cpp/container/span)
* [container/mdspan](https://en.cppreference.com/w/cpp/container/mdspan)
