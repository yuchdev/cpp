# Bitwise Operations in C++

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
* If the left shift of signed integer that overflows
* If the shift count is negative

```cpp
uint32_t x = 1;
x << 32;    // UB
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

* `std::popcount`
* `std::countr_zero`, `std::countl_zero`
* `std::rotl`, `std::rotr`
* `std::has_single_bit`

### Impact

> Decades of "Hacker's Delight" manully implemented tricks are now portable, readable, and optimized.

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

❌ Signed overflow
❌ Shifting signed values into sign bit
❌ Shift count ≥ width
❌ Assuming signed right shift behavior
❌ Assuming enum/flag width without fixing it

---

## 15. Mental model summary

> Bitwise operations in C++ are not expressions over bits,
> they are operations over integer representations constrained by the abstract machine.

* Unsigned = safe bit domain
* Signed = arithmetic domain
* UB is not "edge case" - it is *optimization fuel*
* C++ gives you the power of assembly without its guardrails

---

## 16. Migration note for developers from Java, Python, Rust, etc.

If you come from Java, Python, C#, or JavaScript, bitwise operations in C++ will feel *alarmingly low-level*.

In those languages:

* Integers are abstracted (often arbitrary precision)
* Shifts are defined and safe
* Overflow is either impossible or specified
* Bitwise ops are *logical*, not representational

In C++:

* Integers have finite width
* Overflow can be undefined behavior
* Shifts can be undefined or implementation-defined
* Bitwise ops expose real machine representations
* The compiler assumes UB never happens - and optimizes accordingly

This is why:

* Bitwise code must be defensive
* Types matter deeply (`signed` vs `unsigned`)
* You cannot "try and see" - correctness is architectural

The reward is absolute control: predictable layouts, zero-cost abstractions, hardware-level algorithms, and performance characteristics that higher-level languages intentionally hide.

---

## Final recommendation

> Write bitwise code rarely - but when you do, write it deliberately, defensively, and documented.

Prefer:

* unsigned types
* explicit masks
* standard `<bit>` utilities
* named helper functions

Bitwise C++ is not about cleverness - it is about precision.

---
