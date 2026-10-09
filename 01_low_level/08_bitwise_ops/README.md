# 08. Bitwise Operations in C++

## Advanced Facts, Pitfalls, and Evolution (C++98 → C++20)

> Bitwise operations are where C++ stops abstracting and starts describing the machine.
> This chapter documents *what is guaranteed*, *what is merely common*, and *what is undefined*.

---

## 1. Bitwise operations and the C++ abstract machine

### What C++ guarantees (historically)

* Integers have:

  * a fixed width (`sizeof(T) * CHAR_BIT`)
  * a number of value bits (`std::numeric_limits<T>::digits`)
* Unsigned integers use modulo arithmetic (wraparound is defined)
* Signed integer representation was historically:

  * implementation-defined (two's complement, ones' complement, sign-magnitude)
* Bitwise operators are defined in terms of value representation, not CPU registers

### C++20 strengthening

* Practically all mainstream targets are two's complement
* Many bit tricks are now *effectively portable*, but only if you respect UB rules

---

## 2. Signed vs unsigned: the single most important rule

> Bitwise reasoning belongs to unsigned integers.

### Why?

* Signed overflow is undefined behavior
* Left-shifting a signed value into the sign bit is undefined
* Right-shifting a negative signed value is implementation-defined
* Unsigned operations are fully defined modulo 2ⁿ

### Canonical pattern

```cpp
uint32_t u = static_cast<uint32_t>(signed_value);
```

Perform all bitwise logic on `u`.

---

## 3. Bitwise operators: semantics and traps

### Operators

- `&` - bitwise AND
- `|` - bitwise OR
- `^` - bitwise XOR
- `~` - bitwise NOT
- `<<` - left shift
- `>>` - right shift

* Bitwise operators are applied to integral types only

### Precedence pitfalls

```cpp
a & b == c      // parsed as: a & (b == c)
(a & b) == c   // correct
```

### No short-circuiting

* `&` and `|` always evaluate both operands
* Unlike `&&` and `||`

---

## 4. Shift operators: UB minefield

### Undefined behavior

* If the shift count ≥ bit width

```cpp
uint32_t x = 1;
x << 32;    // UB
```

* If the left shift of signed integer that overflows

```cpp
int x = 1 << 31;   // UB on 32-bit int
```

* If the shift count is negative

```cpp
uint32_t x = 1;
x << -1;    // UB
```

### Implementation-defined

* Right shift of negative signed integers

```cpp
int x = -8;
x >> 1;     // arithmetic or logical shift?
```

### Safe rule

> Only shift unsigned integers. Guard shift counts.

---

## 5. Integer representation and two's complement

### Common identities (unsigned domain)

```cpp
~x + 1 == 0 - x
x & -x   // isolate lowest set bit
```

These are guaranteed for unsigned types.

### Signed caveat

* `~x + 1 == -x` relies on two's complement
* Historically not guaranteed - avoid unless constrained to modern platforms

---

## 6. Bitwise arithmetic (Hacker's Delight patterns)

### Addition without `+`

```cpp
while (b != 0) {
    carry = a & b;
    a ^= b;
    b = carry << 1;
}
```

### Subtraction

```cpp
a - b == a + (~b + 1)   // modulo arithmetic
```

### Overflow detection (unsigned)

```cpp
bool overflow = (a + b) < a;
```

### Branchless min/max (unsigned)

```cpp
mask = -(a < b);
min = (a & mask) | (b & ~mask);
```

### Signed warning

* `abs(INT_MIN)` is undefined behavior
* Branchless signed tricks often depend on implementation details

---

## 7. Power-of-two and bit tests

### Classic identities

```cpp
x & (x - 1) == 0    // power of two (x != 0)
```

### Bit isolation

```cpp
x & -x    // lowest set bit
```

### Meaning

* Powers of two have exactly one bit set
* `(x - 1)` clears the lowest set bit and sets all lower bits

---

## 8. Population count (Hamming weight)

### Algorithms

| Method        | Characteristics       |
|---------------|-----------------------|
| SWAR          | fixed ops, branchless |
| Kernighan     | fast for sparse bits  |
| Lookup tables | memory tradeoff       |
| De Bruijn     | bit scans             |

### C++20 solution

```cpp
std::popcount(x);
```

### Recommendation

> Use `<bit>` in C++20+, otherwise pick algorithm based on bit density.

---

## 9. Bit scans (CTZ / CLZ)

### What they do

* CTZ: count trailing zeros (LSB index)
* CLZ: count leading zeros (MSB index)

### Pre-C++20

* De Bruijn sequences
* Compiler intrinsics
* Portable loops (slow but safe)

### C++20

```cpp
std::countr_zero(x);
std::countl_zero(x);
```

---

## 10. Bit masks and flags

### The wrong way

```cpp
#define READ  1
#define WRITE 2
```

### The correct modern way

```cpp
enum class Perm : uint32_t { Read = 1, Write = 2 };
```

### Operator discipline

* Explicit `|`, `&`, `^`, `~`
* Always define `All`
* Mask results after `~`

```cpp
(~p) & Perm::All
```

---

## 11. Library abstractions

### `std::bitset<N>` (C++98)

* Fixed-size
* No decay to pointer
* Good for masks, protocol fields, teaching

### `std::vector<bool>`

* Not a vector of bools
* Uses proxy references
* Breaks generic code assumptions

> Prefer `vector<uint8_t>` if you need storage.

### `std::byte` (C++17)

* Explicit "raw byte" type
* No arithmetic
* Ideal for binary protocols and memory views

---

## 12. `<bit>` header (C++20)

### Standardized bit utilities

* `std::popcount` - population count
* `std::countr_zero`, `std::countl_zero` - bit scans, CTZ/CLZ
* `std::rotl`, `std::rotr` - bit rotations, defined behavior
* `std::has_single_bit` - power-of-two test
* `std::bit_width` - minimum bits to represent value
* `std::bit_ceil`, `std::bit_floor`, `std::bit_round` - power-of-two rounding

### Impact

> Decades of "Hacker's Delight" manually implemented tricks are now portable, readable, and optimized.

---

## 13. Data layout & algorithms

### Radix sort

* Bitwise partitioning
* Stable LSD/MSD passes
* No comparisons
* Extremely cache-friendly

### Bit packing

* Manual field extraction
* Endianness-sensitive
* Requires explicit masks and shifts

---

## 14. Undefined behavior summary (memorize this)

* ❌ Signed overflow
* ❌ Shifting signed values into sign bit
* ❌ Shift count ≥ width
* ❌ Assuming signed right shift behavior
* ❌ Assuming enum/flag width without fixing it

---
---

## Rules worth keeping in working memory

The central distinction in bitwise operations and shifts is between what the C++ type system guarantees, what the implementation commonly does, and what an API merely assumes. Carrying that distinction from declarations through operations avoids most of the surprises discussed above.

> A valid low-level operation needs a language-level contract, not just a machine-level outcome that looks plausible.

### Core rules

The following rules condense the chapter into reviewable decisions:

1. **Use unsigned types for predictable modular bit patterns.**
2. **Check shift counts and widths before shifting.**
3. **Prefer the C++20 `<bit>` primitives to handwritten scan/popcount tricks.**
4. **Separate logical bit operations from representations and serialization byte order.**


The practical guidance already emphasized in this chapter remains applicable:

> Write bitwise code rarely - but when you do, write it deliberately, defensively, and documented.

Prefer:

* unsigned types
* explicit masks
* standard `<bit>` utilities
* named helper functions

Bitwise C++ is not about cleverness - it is about precision.

---

### Pitfalls at a glance

These recurring failures are particularly useful to recognize in code review because each starts from a plausible but insufficient assumption.

| Pitfall | What happens | Do instead |
|---|---|---|
| Negative or excessive shift count | Undefined behavior | Validate `0 <= n < width` |
| Signed overflow in bit tricks | Undefined behavior | Use unsigned arithmetic |
| Hand-written clz/ctz on zero | Builtins may have preconditions | Use `std::countl_zero`/`std::countr_zero` |

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

Use this checklist when changing bitwise operations and shifts code or reviewing low-level interfaces:

- [ ] Have we distinguished language guarantees from platform-specific observations?
- [ ] Are all input domains and conversion or lifetime preconditions explicit?
- [ ] Can the relevant edge cases be tested without executing undefined behavior?
- [ ] Does the chosen API encode as much of the intended contract as practical?

---

## Diagnostics, useful compiler settings and extensions

Compilers can diagnose many suspicious uses of bitwise operations and shifts, but they cannot infer every application invariant. A clean warning build is useful evidence, not proof: tools can recognize some invalid expressions statically and others only when particular paths execute.

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

The core model of bitwise operations and shifts evolved incrementally; newer standards add safer vocabulary without retroactively rewriting all legacy expressions. The milestones below separate changes to the language from changes to available library interfaces.

### C++98/03: The original model

Bitwise operations inherit integral promotion and width constraints.

### C++11: Stronger types and interfaces

Scoped enum flag domains and fixed-width integers help name masks.

### C++14: Incremental refinement

Binary literals and digit separators improve legibility.

### C++17: Library and deduction evolution

No material change to core bitwise operators.

### C++20: Modern vocabulary

`<bit>` adds rotations, scans, count operations, and bit casts.

### C++23: Further standard facilities

`std::byteswap` adds portable byte-order reversal.

### C++26: Emerging improvements

Further bit facilities may appear, but signed overflow and invalid shifts still require care.

When documenting a facility, state its required language/library version rather than inferring support from the compiler's branding.

---

## Migration note for Java / Python / C# developers

Readers familiar with managed runtimes often expect bitwise operations and shifts to come with runtime metadata, automatic lifetime management, or checked failures. In C++, some of those services come from a chosen library type, while raw language mechanisms may deliberately expose more responsibility to the caller. The important translation is from a *runtime guarantee* in one language to a *type, contract, and lifetime guarantee* in C++.

### Where intuition transfers and where it breaks

The same apparent operation may have a different failure mode or storage model in each language.

| Concept | Java | Python | C# | C++ reality |
|---|---|---|---|---|
| bit | bounded signed machine integers | Defined-width integer bit operations | Unbounded integers and sign extension | Fixed-width integer operators |
| Invalid access/operation | Usually throws or is checked | Usually raises an exception | Often throws in safe code | May be ill-formed, defined, unspecified, or undefined depending on the operation |
| Lifetime | Managed object reachability | Reference counting / GC | Managed GC | Automatic, dynamic, and explicitly borrowed lifetimes coexist |

The distinction matters at API boundaries: a familiar surface syntax does not imply familiar failure behavior.

### Common wrong assumptions

One tempting assumption is that a successful local test demonstrates that an operation is safe. For the low-level C++ rules in this chapter, a test can demonstrate behavior of one build, but cannot establish portability or rule out undefined behavior. Another is that an object is kept alive by every handle referring to it; non-owning pointers, references, and views do not do that. Finally, a managed-language exception should not be presumed to exist at an equivalent C++ failure point.

### Idiomatic C++ replacement

The following mappings help make intent explicit without imitating a managed runtime mechanically.

| Habit from managed languages | Idiomatic C++ |
|---|---|
| Bit masks | Use unsigned fixed-width types and named constants |
| Bit scans | Use `<bit>` operations with their zero-input contracts |
| Expect automatic runtime bounds/lifetime checks | Select an owning container or checked API; validate preconditions explicitly |

Use these choices because they express the program's requirements, not merely because they resemble familiar constructs from another language.

---

## Further reading

These references document the underlying language rules and library contracts. They are starting points for checking precise preconditions; the chapter's examples explain how the rules interact.

### Standard and language reference

* [language/operator_arithmetic](https://en.cppreference.com/w/cpp/language/operator_arithmetic)
* [numeric/bit](https://en.cppreference.com/w/cpp/numeric/bit)
