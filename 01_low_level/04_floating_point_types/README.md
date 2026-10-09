# 04. Floating-Point Types in Modern C++

Floating-point arithmetic is one of the places where code can be perfectly valid C++ and still behave very differently from the mathematical notation it resembles.

The familiar explanation — "`0.1` cannot be represented exactly in binary" — is only the beginning. For experienced C++ developers, the more important questions are:

* what the C++ language actually guarantees about `float`, `double`, and `long double`;
* when IEEE 754 assumptions are justified and when they are not;
* how precision changes with magnitude;
* what `epsilon()` does and, more importantly, what it does **not** mean;
* why NaNs and signed zero break ordinary ordering intuition;
* why comparison by a single global epsilon is usually wrong;
* how rounding modes, fused operations, excess precision, and compiler flags can change results;
* why some apparently clever representation tricks are undefined behavior in standard C++;
* why converting a floating-point value to an integer can itself be undefined behavior;
* and why "fast math" optimizations change semantics, not merely speed.

This chapter follows the seven examples in this directory:

1. [`00_floating_point_representation/floating_point_representation.cpp`](00_floating_point_representation/floating_point_representation.cpp) — representation, precision, range, C++20 `std::bit_cast`/`std::endian`, and guarded C++23 `<stdfloat>`.
2. [`01_extract_fp_components/extract_fp_components.cpp`](01_extract_fp_components/extract_fp_components.cpp) — sign, exponent, significand, subnormals, and C++20-safe bit inspection.
3. [`02_compare_fp/compare_fp.cpp`](02_compare_fp/compare_fp.cpp) — equality, ULPs, relative/absolute error, signed zero, NaNs, and C++20 partial ordering.
4. [`03_fp_functions/fp_functions.cpp`](03_fp_functions/fp_functions.cpp) — rounding, classification, stable math functions, C++20 constants, interpolation, and midpoint.
5. [`04_fp_errors/fp_errors.cpp`](04_fp_errors/fp_errors.cpp) — floating-point environment, status flags, rounding modes, traps, compiler interaction, and C++20 atomic floating-point operations.
6. [`05_fast_integer_cast/integer_cast.cpp`](05_fast_integer_cast/integer_cast.cpp) — floating/integer conversions, historical bit hacks, and undefined-behavior boundaries.
7. [`06_fast_reverse_sqrt/reverse_sqrt.cpp`](06_fast_reverse_sqrt/reverse_sqrt.cpp) — the classic fast inverse square root, Newton iteration, representation tricks, and why the hack is mainly historical now.

The emphasis is on facts that remain surprising even to experienced C++ programmers.

The repository currently builds as C++20. C++23-only examples are feature-tested so the C++20 baseline stays buildable while newer toolchains can expose the additional demonstrations.

---

## 1. Floating-point representation: the type name is not the format

The first example visualizes the bit representation of `float` and `double` values. On mainstream machines the output usually matches IEEE 754 binary32 and binary64 exactly, but portable C++ must distinguish between **what is common** and **what the language requires**.

### C++ does not require `float` to be IEEE binary32

C++ provides three standard floating-point types:

```cpp
float
double
long double
```

Their size ordering is guaranteed:

```cpp
sizeof(float) <= sizeof(double)
sizeof(double) <= sizeof(long double)
```

but exact sizes are not.

On mainstream systems, the usual mapping is:

| C++ type | Common format | Precision | Typical storage |
|---|---|---:|---:|
| `float` | IEEE 754 binary32 | 24 binary digits | 32 bits |
| `double` | IEEE 754 binary64 | 53 binary digits | 64 bits |
| `long double` | ABI-dependent | ABI-dependent | 64, 80, 96, 128 bits, or other |

Do not encode the first two rows as language guarantees.

A useful compile-time check is:

```cpp
#include <limits>

static_assert(std::numeric_limits<double>::is_iec559);
```

`is_iec559` tells you whether the implementation claims the IEC 559 / IEEE-754 properties required by `std::numeric_limits`.

Even then, check the exact properties you rely on:

```cpp
using limits = std::numeric_limits<double>;

static_assert(limits::radix == 2);
static_assert(limits::digits == 53);
static_assert(sizeof(double) == 8);
```

That is much better than silently assuming "all doubles are IEEE binary64".

### `FLT_RADIX` need not be 2

Most modern general-purpose machines use binary floating point, but the C and C++ floating-point model allows another radix.

```cpp
#include <cfloat>

std::cout << FLT_RADIX;
```

Code that assumes a binary significand should make that assumption explicit.

This matters particularly for algorithms derived from the bit layout. A numerical algorithm written only in terms of `+`, `-`, `*`, `/`, `frexp`, `ldexp`, and `numeric_limits` is much more portable than one that assumes:

```text
1 sign bit
8 exponent bits
23 stored fraction bits
bias 127
```

Those numbers describe binary32, not "C++ float" in general.

### `long double` is one of the least portable fundamental types

`long double` often surprises code that crosses platforms.

Common implementations include:

* the same representation as `double` on MSVC targets;
* x87 80-bit extended precision, often stored in 12 or 16 bytes, on many x86 Unix-like ABIs;
* IEEE binary128 on some architectures and toolchains;
* "double-double" representations on some PowerPC environments.

Therefore:

```cpp
sizeof(long double)
```

does not directly tell you its effective precision.

Use:

```cpp
std::numeric_limits<long double>::digits
std::numeric_limits<long double>::digits10
std::numeric_limits<long double>::max_digits10
```

when the property you actually care about is precision.

### C++23 added optional fixed-width floating-point types — with an unusual rule

C++23 introduced `<stdfloat>` and optional aliases such as:

```cpp
std::float16_t
std::float32_t
std::float64_t
std::float128_t
std::bfloat16_t
```

These are available only if the implementation supports the corresponding extended floating-point types.

A subtle point: unlike `std::uint32_t`, `std::float32_t` is not simply a portable alias for ordinary `float`. The fixed-width floating-point aliases, when provided, name **extended floating-point types**, not the three standard floating-point types.

So this is not a universal replacement for:

```cpp
float
double
long double
```

It is a facility for code that specifically needs one of the standardized extended formats and whose implementation supplies it.

### IEEE binary floating point stores scale and precision separately

For normal IEEE binary32 values:

```text
value = (-1)^sign × (1.fraction) × 2^(exponent - 127)
```

The leading `1` is implicit for normal values, so 23 stored fraction bits provide 24 bits of precision.

For binary64:

```text
value = (-1)^sign × (1.fraction) × 2^(exponent - 1023)
```

with 52 stored fraction bits and 53 bits of precision.

This is why binary floating point has enormous range but only a fixed number of significant binary digits.

The decimal point is not really "floating". The **binary exponent** changes the scale of a fixed-precision significand.

### Floating-point spacing is not uniform

Integers in a fixed-width integer type are equally spaced.

Floating-point values are not.

Near `1.0`, adjacent binary64 numbers are separated by roughly:

```text
2^-52
```

Near `2.0`, the spacing is twice as large.

Near `2^100`, it is vastly larger.

This leads to one of the most important mental models for floating point:

> precision is approximately relative for normal values, not absolute.

The spacing between adjacent representable values is often described in ULPs — *units in the last place*.

For a binary format with precision `p`, normal values around exponent `e` have spacing approximately:

```text
2^(e - (p - 1))
```

Crossing a power-of-two boundary changes the ULP size.

### `epsilon()` is local to 1.0

For a floating-point type `T`:

```cpp
std::numeric_limits<T>::epsilon()
```

is the difference between `1` and the next representable value greater than `1`.

For IEEE binary64:

```text
epsilon = 2^-52 ≈ 2.220446049250313e-16
```

This is **not**:

* the smallest representable positive number;
* a universal rounding error;
* the correct tolerance for every comparison;
* the smallest difference between any two floating-point values;
* the error accumulated by an arbitrary computation.

Under round-to-nearest, the maximum relative rounding error of one correctly rounded normal operation is often described using the **unit roundoff**, which is roughly `epsilon / 2`. That is a different concept.

### `min()` is a famous trap

For integer types, programmers often expect `min()` to mean "most negative".

For floating-point types:

```cpp
std::numeric_limits<double>::min()
```

is the **smallest positive normal** value.

The most negative finite value is:

```cpp
std::numeric_limits<double>::lowest()
```

The smallest positive subnormal, if subnormals are supported, is:

```cpp
std::numeric_limits<double>::denorm_min()
```

For generic numeric code, confusing `min()` and `lowest()` is an easy way to write a range bug.

### All integers are exact only up to a precision boundary

An IEEE binary32 `float` has 24 bits of precision. Therefore every integer up to `2^24` in magnitude can be represented exactly.

After that, representable integers start skipping values.

For example on binary32:

```cpp
float a = 16'777'216.0f; // 2^24
float b = 16'777'217.0f;

assert(a == b);
```

`16,777,217` rounds to `16,777,216`.

For binary64, the analogous boundary is `2^53`.

This matters in:

* counters stored accidentally as `double`;
* timestamps;
* database IDs;
* JSON numbers;
* JavaScript interoperability;
* financial import/export;
* hash keys;
* telemetry sequence numbers.

A `double` has a huge numeric range, but it does **not** have 64 bits of integer precision.

### Decimal fractions and binary fractions are different sets

`0.5` is exact in binary:

```text
0.1₂
```

`0.125` is exact:

```text
0.001₂
```

But decimal `0.1` has an infinite repeating binary expansion.

That means:

```cpp
double x = 0.1;
```

stores the nearest representable binary approximation, not "decimal one tenth".

This is not a flaw in IEEE 754. Decimal floating-point formats have the mirror-image problem: many binary fractions cannot be represented exactly in decimal formats either.

The real engineering question is whether the chosen radix matches the semantics of the domain.

For money, integer minor units or decimal arithmetic may be more appropriate than binary floating point.

### Exact round-trip text needs `max_digits10`, not `digits10`

Three precision constants are easy to confuse:

```cpp
std::numeric_limits<T>::digits
std::numeric_limits<T>::digits10
std::numeric_limits<T>::max_digits10
```

For an IEEE binary64 `double`:

* `digits == 53`: binary precision;
* `digits10 == 15`: decimal digits that can be converted to `double` and back without change;
* `max_digits10 == 17`: decimal digits sufficient to print any `double` so that parsing it reconstructs the same value.

For diagnostic serialization:

```cpp
std::cout << std::setprecision(
    std::numeric_limits<double>::max_digits10
) << value;
```

If exact textual round-trip matters, `max_digits10` is usually the relevant property.

For machine-readable interchange, `std::to_chars` / `std::from_chars` are attractive because they are locale-independent.

### Bit inspection through an unrelated pointer is not portable C++

The representation example contains a classic historical technique:

```cpp
auto p = reinterpret_cast<std::uint32_t*>(&f);
std::cout << *p;
```

The intent is understandable, but dereferencing an unrelated pointer type can violate the C++ aliasing rules.

In C++20, prefer:

```cpp
#include <bit>
#include <cstdint>

static_assert(sizeof(float) == sizeof(std::uint32_t));

std::uint32_t bits = std::bit_cast<std::uint32_t>(f);
```

Before C++20, `std::memcpy` is the portable idiom:

```cpp
std::uint32_t bits;
std::memcpy(&bits, &f, sizeof bits);
```

`std::bit_cast` expresses exactly what is wanted: copy the object representation into another trivially copyable type of the same size without creating an aliasing violation.

**Engineering takeaway:** before reasoning about floating-point bits, separate C++ type guarantees from ABI facts and IEEE-754 facts.

---

## 2. Extracting sign, exponent, and significand: normal values are only one case

The component-extraction example illustrates the classic IEEE layout, but it also reveals why decoding floating-point representation is trickier than masking fields.

### Normal binary32 values use an implicit leading one

For a normal binary32 value with fields:

```text
s | exponent | fraction
```

the value is:

```text
(-1)^s × 1.fraction × 2^(exponent - 127)
```

For example:

```text
1.5 = 0 01111111 10000000000000000000000
```

The stored fraction is `0.5`, but the actual significand is `1.5` because the leading `1` is implicit.

This "hidden bit" buys one extra bit of precision.

### Subnormal values do not use the implicit leading one

Exponent field zero is special.

For binary32 subnormals:

```text
value = (-1)^s × 0.fraction × 2^-126
```

not:

```text
(-1)^s × 1.fraction × 2^-127
```

Two differences matter:

1. the leading significand bit is `0`, not `1`;
2. the effective exponent is the minimum normal exponent, `-126`.

Subnormals fill the gap between the smallest normal value and zero. This is called **gradual underflow**.

Without subnormals, values would jump abruptly from the smallest normal number to zero.

### Exponent all-ones is not a large finite exponent

For IEEE formats, the maximum encoded exponent is reserved.

For binary32:

```text
exponent = 255, fraction = 0     -> infinity
exponent = 255, fraction != 0    -> NaN
```

Therefore a decoder must distinguish at least:

* zero;
* subnormal;
* normal;
* infinity;
* NaN.

The portable library already exposes this classification:

```cpp
switch (std::fpclassify(x)) {
case FP_ZERO:
case FP_SUBNORMAL:
case FP_NORMAL:
case FP_INFINITE:
case FP_NAN:
    break;
}
```

### Signed zero is a real floating-point value

IEEE floating point has both:

```cpp
+0.0
-0.0
```

They compare equal:

```cpp
assert(+0.0 == -0.0);
```

but they are not semantically interchangeable in all operations:

```cpp
1.0 / +0.0 // +infinity on IEC 559 implementations
1.0 / -0.0 // -infinity
```

Use:

```cpp
std::signbit(x)
```

when the sign of zero matters.

A comparison like:

```cpp
x < 0
```

does not detect negative zero.

Signed zero is important in complex arithmetic, branch cuts, interval algorithms, and code that deliberately preserves the direction from which a limit was approached.

### NaNs also have a sign bit, but do not treat it as an ordinary sign

NaNs occupy many possible bit patterns. IEEE formats allow:

* quiet NaNs;
* signaling NaNs;
* implementation-dependent payload bits;
* a sign bit.

Do not build application semantics around the sign or payload of a NaN unless you control the platform and explicitly designed the protocol around those bits.

Optimizers, arithmetic operations, conversions, and libraries may canonicalize NaNs or discard payload information.

### Union type punning is not the portable C++ representation API

A common C idiom is:

```cpp
union {
    float f;
    std::uint32_t i;
} u;

u.f = value;
auto bits = u.i;
```

In portable C++, reading a different inactive union member is not the general-purpose type-punning mechanism.

Prefer `std::bit_cast`:

```cpp
auto bits = std::bit_cast<std::uint32_t>(value);
```

This distinction matters in optimized builds because aliasing and object-lifetime rules are part of the optimizer's assumptions.

### Avoid C/C++ bit-fields for IEEE layout decoding

It is tempting to write:

```cpp
struct FloatBits {
    unsigned fraction : 23;
    unsigned exponent : 8;
    unsigned sign : 1;
};
```

and bit-cast a `float` into it.

That is not portable either. Bit-field allocation order, packing, and alignment are implementation-defined.

If you have established that the representation is IEEE binary32, masking an integer bit pattern is clearer:

```cpp
const std::uint32_t bits = std::bit_cast<std::uint32_t>(value);

const auto sign     = bits >> 31;
const auto exponent = (bits >> 23) & 0xffu;
const auto fraction = bits & 0x7fffffu;
```

### Generic IEEE decoders must not reuse binary32 constants for binary64

A subtle failure mode is to "template" the code while leaving constants such as:

```text
8 exponent bits
bias 127
23 fraction bits
```

inside the implementation.

Binary64 uses:

```text
11 exponent bits
bias 1023
52 stored fraction bits
```

A generic decoder needs a complete traits description, not just a wider integer type.

It should also use exact-width unsigned integer storage types when the layout itself is the subject:

```cpp
std::uint32_t
std::uint64_t
```

rather than `long`, whose width differs between LP64 and LLP64 ABIs.

### For numerical decomposition, use numerical APIs instead of representation APIs

If the goal is not serialization or bit-level teaching, do not decode IEEE fields manually.

`std::frexp` decomposes a floating-point value numerically:

```cpp
int exponent;
double significand = std::frexp(x, &exponent);

// x == significand * 2^exponent
// |significand| is in [0.5, 1.0) for finite nonzero x
```

`std::ldexp` composes it again:

```cpp
double x2 = std::ldexp(significand, exponent);
```

Related functions include:

```cpp
std::ilogb(x)
std::logb(x)
std::scalbn(x, n)
std::scalbln(x, n)
```

These describe numeric scale without assuming a particular object representation.

### Endianness and bit numbering are separate concerns

If you `bit_cast` an IEEE binary32 `float` to `std::uint32_t`, then use integer shifts and masks, you are reasoning about the integer's numeric bit positions.

If instead you inspect the object byte-by-byte, endianness becomes visible.

This distinction is useful:

* shifts/masks answer "which encoded bits are set?";
* byte inspection answers "how is this object laid out in memory?".

Do not confuse network byte order with IEEE field order.

**Engineering takeaway:** manual component extraction is useful for understanding formats, debugging, serialization, and specialist code. For ordinary numerical work, use the classification and decomposition functions from `<cmath>`.

---

## 3. Comparing floating-point values: "use epsilon" is incomplete advice

The comparison example correctly highlights that exact equality between independently calculated approximations is often inappropriate. The deeper lesson is that there is no universal floating-point equality predicate.

The correct comparison depends on what the numbers mean.

### Exact floating-point equality is not always wrong

This is perfectly valid:

```cpp
if (x == 0.0) {
    ...
}
```

when `x` comes from an operation for which exact zero is part of the algorithmic contract.

Exact comparison is also appropriate for:

* values copied from the same representation;
* powers of two known to be exactly representable;
* sentinels deliberately stored as floating-point values;
* testing whether a computation reached a bit-exact expected result;
* checking infinities;
* state-machine values that happen to use a floating type.

The rule should not be "never use `==` with floating point".

The rule should be:

> use exact equality only when exact equality has domain meaning.

### A global absolute epsilon is scale-dependent

This common test is usually wrong:

```cpp
std::fabs(a - b) < std::numeric_limits<double>::epsilon()
```

At magnitude `1e100`, one ULP is vastly larger than `epsilon()`.

Near zero, a purely relative comparison can fail in the opposite direction.

Therefore comparison normally needs two ideas:

* **relative tolerance** for ordinary magnitudes;
* **absolute tolerance** around zero.

A common structure is:

```cpp
bool nearly_equal(
    double a,
    double b,
    double rel_tol,
    double abs_tol)
{
    if (a == b) {
        // Handles exact equality, signed zeros, and equal infinities.
        return true;
    }

    if (std::isnan(a) || std::isnan(b)) {
        return false;
    }

    const double diff = std::fabs(a - b);
    const double scale = std::max(std::fabs(a), std::fabs(b));

    return diff <= std::max(abs_tol, rel_tol * scale);
}
```

The important design question is not the formula. It is **where `rel_tol` and `abs_tol` come from**.

They should come from:

* measurement accuracy;
* accumulated numerical error;
* physical tolerances;
* iteration stopping criteria;
* the conditioning of the algorithm;
* a protocol or file-format specification.

Multiplying machine epsilon by an arbitrary constant is sometimes fine for a very small, well-understood computation, but it is not a general numerical policy.

### Relative-only comparison fails near zero

Suppose:

```text
a = 1e-300
b = 2e-300
```

Their absolute difference is tiny, but their relative difference is 50%.

Whether they should compare "close" is a domain question.

Now compare zero with the smallest subnormal. A relative tolerance based on:

```cpp
max(abs(a), abs(b))
```

can become too strict or underflow during its own calculation.

That is why the absolute tolerance term is not optional in a general-purpose "near zero" comparison.

### Absolute-only comparison fails across scale

A tolerance such as:

```cpp
fabs(a - b) < 1e-9
```

may be reasonable for values measured in meters in one application.

It is nonsensical as a universal floating-point test.

At a scale of `1e12`, `1e-9` may be much smaller than one representable increment.

At a scale of `1e-15`, it may classify wildly different values as equal.

### ULP comparison is useful, but not automatically more meaningful

`std::nextafter` gives the adjacent representable value in a direction:

```cpp
double y = std::nextafter(x, std::numeric_limits<double>::infinity());
```

This is extremely useful for:

* boundary testing;
* exploring spacing;
* testing rounding behavior;
* defining representable neighborhoods;
* numerical debugging.

One can also compare bit patterns to measure a distance in ULPs, but this requires careful handling of:

* negative values;
* the transition through signed zero;
* NaNs;
* infinities;
* non-IEEE representations;
* different spacing on opposite sides of powers of two.

An ULP threshold answers:

> how many representable floating-point steps apart are these values?

It does **not** answer:

> are these values close enough for my physical or business problem?

### NaN destroys total ordering

For a NaN value `q`:

```cpp
q == q // false
q < q  // false
q > q  // false
```

More generally, every ordinary comparison with NaN is false except `!=`.

Use:

```cpp
std::isnan(q)
std::isunordered(a, b)
```

when NaN is part of the domain.

Since C++20, the three-way comparison of floating-point values has partial-order semantics:

```cpp
auto r = a <=> b;
```

The result can be unordered.

This is why a floating-point comparison relation is not automatically suitable as an ordering predicate for an ordered container.

### Sorting floating-point data containing NaNs needs a policy

A comparator used by `std::sort` or `std::map` must satisfy the required ordering properties.

NaNs make a naive comparator policy-sensitive.

If NaNs can occur, explicitly decide where they belong:

* before all finite values;
* after all finite values;
* grouped by payload;
* rejected before sorting.

Do not assume the hardware's ordinary `<` relation gives a total order.

IEEE 754 defines total-order concepts, but the ordinary C++ relational operators intentionally preserve unordered NaN behavior.

### Signed zero also matters to ordering policies

```cpp
-0.0 == +0.0
```

is true, but:

```cpp
std::signbit(-0.0) != std::signbit(+0.0)
```

If a serialization, cache key, total order, or reproducibility test distinguishes their representations, equality alone is not enough.

Conversely, most mathematical application code should treat them as equal.

Again, the right choice comes from semantics, not from a universal floating-point rule.

### Non-associativity affects equality even when each operation is correctly rounded

Floating-point addition is generally not associative:

```cpp
(a + b) + c != a + (b + c)
```

For example, if `a` is huge and `b` is tiny relative to `a`, `a + b` can round back to `a` before `c` is added.

Consequences include:

* different parallel reductions producing different answers;
* vectorized code differing from scalar code;
* changing loop order changing a result;
* hash/reproducibility tests failing after optimization changes.

For large reductions, consider:

* pairwise summation;
* Kahan or Neumaier compensated summation;
* sorting terms by magnitude where appropriate;
* higher-precision accumulation;
* domain-specific exact accumulators.

### Cancellation can destroy significant digits

Consider:

```cpp
double x = a - b;
```

when `a` and `b` are nearly equal large values.

The subtraction itself may be correctly rounded, yet the result can contain very few meaningful significant digits because most leading digits cancel.

This is **catastrophic cancellation**.

It is an algorithmic problem, not something that a larger comparison epsilon can repair after the fact.

Stable numerical programming often means reformulating equations to avoid cancellation.

**Engineering takeaway:** floating-point comparison is an error-model problem. Machine epsilon is a property of the type, not a substitute for understanding the calculation.

---

## 4. `<cmath>`: functions with similar names often have importantly different semantics

The `fp_functions.cpp` example surveys the floating-point library. Many functions that look like conveniences actually encode careful numerical behavior.

### The rounding functions form several different families

These functions do not all mean "round".

#### `floor`, `ceil`, and `trunc`

```cpp
std::floor(x) // toward -infinity
std::ceil(x)  // toward +infinity
std::trunc(x) // toward zero
```

They return floating-point values.

Their direction is defined independently of the current floating-point rounding mode.

#### `round`

```cpp
std::round(x)
```

rounds to the nearest integer value, with halfway cases **away from zero**.

Thus:

```text
round( 2.5) =  3
round(-2.5) = -3
```

It also ignores the current floating-point rounding mode.

#### `rint`

```cpp
std::rint(x)
```

uses the current rounding mode.

With the usual `FE_TONEAREST` mode on IEC-559 systems, halfway cases are typically rounded to even:

```text
rint(2.5) = 2
rint(3.5) = 4
```

`std::rint` may raise `FE_INEXACT` when rounding occurs.

#### `nearbyint`

`std::nearbyint` also obeys the current rounding mode, but on IEC-559 implementations it does **not** raise `FE_INEXACT`.

This difference matters in code that monitors floating-point exception flags.

#### `lround` / `llround` versus `lrint` / `llrint`

The `l*` functions return integer types.

The distinction remains:

* `lround` / `llround`: halfway away from zero, independent of current rounding mode;
* `lrint` / `llrint`: current rounding mode.

Out-of-range integer results are not something to ignore. These functions have specified error signaling behavior; plain C++ casts have a different rule discussed later.

### The rounding mode does not control a floating-to-integer cast

This surprises people who have just learned `fesetround`.

```cpp
std::fesetround(FE_UPWARD);

double x = 1.2;
int i = static_cast<int>(x);
```

`i` is still `1`.

Built-in floating-to-integer conversion discards the fractional part: it truncates toward zero.

Use the rounding functions when another policy is required.

### `std::fma` can be both more accurate and observably different

```cpp
std::fma(a, b, c)
```

computes the mathematical `a*b+c` as if with infinite intermediate precision and rounds only once to the result type.

A separate expression:

```cpp
a * b + c
```

may round once after multiplication and again after addition.

That means `std::fma` can preserve information that the two-operation form loses.

It also means they can produce different answers.

Compilers may contract expressions into fused operations under permitted rules and optimization settings, so code that depends on exact intermediate rounding must consider FP contraction settings.

This is one reason optimized numerical results can change when moving between:

* CPUs with and without hardware FMA;
* compiler versions;
* optimization levels;
* strict and fast-math modes.

### `log1p` and `expm1` exist to avoid cancellation

For very small `x`:

```cpp
std::log(1.0 + x)
```

can lose `x` entirely because `1.0 + x` rounds to `1.0`.

Use:

```cpp
std::log1p(x)
```

which computes `log(1+x)` using an algorithm designed for accuracy near zero.

Likewise:

```cpp
std::exp(x) - 1.0
```

can lose precision for small `x`.

Use:

```cpp
std::expm1(x)
```

These functions are classic examples of a broader rule:

> algebraically equivalent formulas are not numerically equivalent.

### `hypot` is better than spelling out the formula

Instead of:

```cpp
std::sqrt(x * x + y * y)
```

prefer:

```cpp
std::hypot(x, y)
```

A good implementation scales its arguments to reduce unnecessary overflow and underflow.

For example, `x*x` can overflow even when the true hypotenuse is representable.

The standard-library function communicates intent and gives the implementation an opportunity to use a robust algorithm.

### `frexp` / `ldexp` are preferable to multiplying by powers manually

```cpp
int e;
double m = std::frexp(x, &e);
double back = std::ldexp(m, e);
```

These operations work in the floating-point radix and avoid several avoidable intermediate problems associated with:

```cpp
x * std::pow(2.0, n)
```

For scaling by powers of the implementation radix, also consider:

```cpp
std::scalbn(x, n)
```

### `fmod` and `remainder` are not synonyms

For floating-point remainder calculations:

```cpp
std::fmod(x, y)
std::remainder(x, y)
```

use different quotient-selection rules.

`fmod` behaves consistently with truncation of the quotient toward zero.

`remainder` uses the nearest integer quotient, with the IEEE-style tie rule on IEC-559 implementations.

They can therefore return different signs and magnitudes.

Choose based on the mathematical operation you need, not on the function name.

### `std::isnormal` is not the opposite of `std::isnan`

`std::isnormal(x)` is true only for **normal finite nonzero** values.

It is false for:

* zero;
* subnormals;
* infinities;
* NaNs.

The complete classification API is:

```cpp
std::fpclassify(x)
std::isfinite(x)
std::isinf(x)
std::isnan(x)
std::isnormal(x)
std::signbit(x)
```

Use the predicate that expresses the actual precondition.

### Prefer `std::numbers::pi` to `M_PI` in modern C++

`M_PI` is a widely supported extension, but it is not the portable C++ constant API and historically requires implementation-specific feature macros on some systems.

Since C++20:

```cpp
#include <numbers>

double pi = std::numbers::pi;
float pif = std::numbers::pi_v<float>;
```

For generic numerical code:

```cpp
template<class T>
constexpr T pi = std::numbers::pi_v<T>;
```

is more expressive than converting a macro-defined `double`.

### Special values should usually come from `numeric_limits`

Instead of depending on macros:

```cpp
INFINITY
NAN
HUGE_VAL
```

generic C++ can use:

```cpp
auto inf = std::numeric_limits<T>::infinity();
auto qnan = std::numeric_limits<T>::quiet_NaN();
auto snan = std::numeric_limits<T>::signaling_NaN();
```

after checking the corresponding support properties if portability to non-IEC-559 implementations matters.

### Subnormals are semantically useful and can be a performance boundary

Subnormals preserve gradual underflow, allowing values below the normal range to approach zero smoothly.

Historically, some processors handled subnormal arithmetic much more slowly than normal arithmetic.

Modern performance is architecture- and instruction-dependent; do not assume a universal "10x" or "100x" penalty.

Performance-sensitive systems sometimes enable hardware modes such as:

* FTZ — flush-to-zero;
* DAZ — denormals-are-zero.

These can improve throughput on some hardware, but they change numerical semantics.

Such modes are usually outside the portable C++ floating-point environment API.

If reproducibility matters, FP control state must be treated as part of program state.

**Engineering takeaway:** `<cmath>` is not just a collection of wrappers around hardware instructions. Many functions exist specifically to encode numerically safer or semantically distinct operations.

---

## 5. Floating-point errors and the floating-point environment

The `fp_errors.cpp` example demonstrates `<cfenv>`, hardware traps, and status flags. This area contains several terminology traps.

### A floating-point "exception" is usually not a C++ exception

The standard floating-point status flags include, where supported:

```cpp
FE_DIVBYZERO
FE_INEXACT
FE_INVALID
FE_OVERFLOW
FE_UNDERFLOW
```

These are typically **sticky status flags** in the floating-point environment.

They do not imply:

```cpp
throw std::exception{};
```

and ordinary floating-point division by zero does not normally enter a C++ `catch` block.

On IEEE-style default hardware configuration, exceptional floating-point operations usually continue and produce values such as infinity or NaN while setting status flags.

### `FE_INEXACT` is normal

`FE_INEXACT` is raised for ordinary calculations whenever the exact mathematical result cannot be represented.

That includes an enormous fraction of useful floating-point work.

Therefore:

```cpp
fetestexcept(FE_INEXACT)
```

returning nonzero does not mean "the computation failed".

For most applications, inexact rounding is the expected operating mode of floating-point arithmetic.

### Floating-point flags are sticky

Once raised, an exception flag usually remains set until explicitly cleared or the environment is changed.

Therefore this pattern is wrong if you want to attribute an error to one operation:

```cpp
do_work();
if (std::fetestexcept(FE_OVERFLOW)) {
    ...
}
```

unless you first established a clean baseline.

The usual structure is:

```cpp
std::feclearexcept(FE_ALL_EXCEPT);

do_work();

const int raised = std::fetestexcept(FE_ALL_EXCEPT);
```

### The floating-point environment is thread-local, not one global process variable

The C++ floating-point environment is associated with the executing thread.

A new thread inherits an initial floating-point environment according to the implementation/platform rules, and then its state can diverge.

That means changing the rounding mode in one thread should not be modeled as a single global mathematical configuration for the whole process.

Nevertheless, compiler assumptions are a separate issue: the compiler must also be told, in a supported way, that runtime rounding state and exception observation matter.

### Runtime rounding modes and compiler optimization are inseparable

C++ exposes:

```cpp
std::fegetround()
std::fesetround()
```

with modes such as:

```cpp
FE_TONEAREST
FE_DOWNWARD
FE_UPWARD
FE_TOWARDZERO
```

But simply calling `fesetround` does not force every compiler optimization to become rounding-mode aware.

The floating-point environment is meaningful only when the implementation honors floating-environment access. The C/C++ mechanism is associated with:

```cpp
#pragma STDC FENV_ACCESS ON
```

Compiler support is uneven.

GCC and Clang also provide options such as `-frounding-math` for code that depends on dynamic rounding.

This is an important systems lesson:

> floating-point behavior is a contract between source code, compiler options, optimizer transformations, ABI, and hardware state.

### Compile-time and runtime evaluation may not use the same environment

Constant-expression floating-point evaluation is not a runtime FPU instruction.

Code such as:

```cpp
constexpr double x = ...;
```

is evaluated by the compiler's constant-evaluation machinery.

Do not expect a runtime `fesetround` call to affect a compile-time constant expression.

This can create differences between:

* a value folded at compile time;
* the same-looking expression forced to execute at runtime.

### `-ffast-math` changes semantics

On GCC, `-ffast-math` enables a collection of options that can assume or permit things such as:

* no meaningful NaNs or infinities in user-visible control flow;
* no meaningful signed-zero distinction;
* no trapping math;
* reassociation of expressions;
* reciprocal transformations;
* a non-observable dynamic rounding mode;
* faster excess-precision choices.

Therefore `-ffast-math` is not just "enable faster floating-point instructions".

It tells the optimizer that it may violate assumptions relied upon by strict IEEE-style numerical code.

Examples of transformations that become possible include effectively treating:

```cpp
(a + b) + c
```

like:

```cpp
a + (b + c)
```

even though floating-point addition is not associative.

It may also invalidate code that intentionally tests for NaN, signed zero, or floating exceptions.

Use it only when the algorithm's numerical contract explicitly permits those transformations.

### Excess precision can make "the same type" produce different intermediates

Historically, x87 calculations were often performed in 80-bit registers even when variables were nominally `double`.

A value might therefore have:

* more precision while it remains in a register;
* less precision after being spilled to memory.

That could make debugging, optimization, or adding a logging statement change a result.

Modern x86-64 code commonly uses SSE/AVX binary32/binary64 arithmetic, reducing this particular surprise, but excess-precision rules and target-specific behavior still matter.

Never assume that every intermediate is rounded exactly to the nominal source type after every operator unless the language/implementation mode guarantees it.

### FMA contraction is another source of reproducibility differences

If one build computes:

```cpp
a * b + c
```

as two rounded operations and another uses one fused multiply-add, the results can differ by an ULP or more in cancellation-heavy cases.

Neither build is necessarily "wrong".

For deterministic numerical software, record:

* target architecture;
* compiler;
* compiler version;
* optimization level;
* FP contraction settings;
* fast-math settings;
* runtime floating-point environment.

### Trapping floating-point exceptions is platform-specific

Portable C++ standardizes observation and manipulation of floating-point status flags.

It does **not** provide a universal API to unmask hardware floating-point traps.

Real systems use facilities such as:

* `feenableexcept` on some Unix/libc environments;
* x87 control words;
* SSE/AVX `MXCSR`;
* platform-specific MSVC CRT controls;
* ARM floating-point control registers.

This is why trap-enabling code is inherently platform code.

### `SIGFPE` is not "the floating-point exception signal"

Despite its name, `SIGFPE` can represent several arithmetic faults, including integer faults on POSIX systems.

The detailed reason may be exposed through values such as:

```text
FPE_INTDIV
FPE_FLTDIV
FPE_FLTOVF
FPE_FLTINV
...
```

Do not infer "floating-point" merely from the signal name.

### Recovering from a hardware FP trap is much harder than catching a C++ exception

A signal handler runs under signal-safety constraints.

Jumping out of a faulting instruction with `siglongjmp` can bypass ordinary C++ stack unwinding and object cleanup.

Therefore trap-and-recover code is best treated as:

* diagnostics;
* a controlled test harness;
* a crash-reporting mechanism;
* specialist numerical runtime infrastructure.

It is generally not a replacement for ordinary error handling.

### Save and restore FP environment state

Library code should be very cautious about leaving a changed rounding mode or exception configuration behind.

A function that calls:

```cpp
std::fesetround(FE_DOWNWARD);
```

and forgets to restore the previous mode has modified thread-local execution state for its caller.

A robust wrapper should save state and restore it on every exit path, preferably with RAII.

This is another example where floating-point state behaves more like a resource than a mere arithmetic detail.

### `errno` and floating-point exception flags are separate channels

Math functions may report errors through:

* `errno`;
* floating-point exception flags;
* special return values such as NaN or infinity.

The implementation advertises supported mechanisms through `math_errhandling`.

Do not write error-handling code that assumes every platform uses the same channel.

**Engineering takeaway:** floating-point environment code is systems programming. The source expression alone does not define the whole behavior; compiler mode and hardware control state are part of the execution model.

---

## 6. Floating-point to integer conversion: one of C++'s sharper undefined-behavior edges

The `integer_cast.cpp` example contains historical fast-conversion tricks. Modern CPUs usually have direct conversion instructions, so the most important lesson today is the language semantics around conversion.

### Built-in floating-to-integer conversion truncates toward zero

```cpp
static_cast<int>( 9.99) ==  9
static_cast<int>(-9.99) == -9
```

The current floating-point rounding mode does not change this rule.

This corresponds naturally to instructions such as x86 `cvttss2si` / `cvttsd2si`, where the "tt" denotes truncation.

### Out-of-range floating-to-integer conversion is undefined behavior

This is much more severe than many programmers expect.

If the truncated floating-point value is not representable in the destination integer type, the behavior is undefined.

That includes cases involving:

* values larger than the destination maximum;
* values smaller than the destination minimum;
* infinity;
* NaN.

Do not rely on a particular hardware instruction returning `INT_MIN`, saturating, wrapping, or setting a flag.

Those are hardware behaviors, not the portable C++ contract.

A safe conversion needs an explicit range policy.

For example:

```cpp
std::optional<int> to_int(double x)
{
    if (!std::isfinite(x)) {
        return std::nullopt;
    }

    const double t = std::trunc(x);

    if (t < static_cast<double>(std::numeric_limits<int>::min()) ||
        t > static_cast<double>(std::numeric_limits<int>::max())) {
        return std::nullopt;
    }

    return static_cast<int>(t);
}
```

For types whose integer endpoints are not exactly representable in the floating source type, the boundary check itself needs additional care. Generic conversion utilities should be designed and tested around those exact endpoint cases.

### Unsigned destinations do not give modulo semantics here

Integer-to-integer conversion to unsigned types has modulo-style semantics.

Floating-to-unsigned conversion does **not** mean "truncate and wrap modulo `2^N`".

If the truncated value cannot be represented in the unsigned destination type, behavior is undefined.

That distinction is easy to miss.

### Integer-to-floating conversion can also lose information

The reverse conversion is not automatically exact:

```cpp
std::uint64_t n = ...;
double d = n;
```

A binary64 `double` has only 53 bits of precision.

Large 64-bit integers therefore collapse onto the same floating-point value.

This is a common bug in:

* JSON pipelines;
* database adapters;
* scripting-language bridges;
* telemetry;
* distributed IDs.

If exact integer identity matters, do not pass the value through a floating-point representation that cannot represent every integer in the required range.

### A conversion can be in range but still not exact

Suppose an integer is well inside the exponent range of `double` but requires more than 53 significant binary bits.

The resulting `double` remains finite but rounds to a nearby representable value.

Range and precision are separate properties.

This is why "double can represent numbers up to about `1e308`" says almost nothing about which integers near `1e18` it represents exactly.

### Historical "magic number" conversion tricks depend on many assumptions

The classic fast conversion in the example relies on details such as:

* IEEE binary representation;
* a known exponent bias;
* a known significand width;
* chosen rounding behavior;
* exact object representation;
* endianness/bit interpretation assumptions;
* compiler behavior around type punning;
* a constrained input range.

These tricks were useful when hardware float-to-int conversion was expensive.

On modern x86, ARM, and other mainstream CPUs, direct conversion instructions are usually available and compilers already know how to emit them.

A hand-written bit trick can therefore be:

* less portable;
* undefined under the C++ object model;
* harder to vectorize;
* wrong at edge cases;
* no faster than ordinary code.

### Type punning does not become legal because it is fast

Code such as:

```cpp
int i = *reinterpret_cast<int*>(&x);
```

has the same aliasing problem discussed earlier.

For representation transfer, use `std::bit_cast`.

For numeric conversion, use numeric conversion.

These are different operations:

```cpp
std::bit_cast<std::uint32_t>(f) // copy representation bits
static_cast<int>(f)             // convert numeric value
```

Confusing the two is at the heart of many historical floating-point hacks.

### Rounding functions are safer when the rounding policy is part of the requirement

If the requirement is "nearest integer, ties away from zero":

```cpp
std::lround(x)
```

expresses that.

If the requirement is "respect the active rounding mode":

```cpp
std::lrint(x)
```

expresses that.

If the requirement is truncation:

```cpp
std::trunc(x)
```

makes the policy explicit before any range-checked integer conversion.

Do not select a conversion technique based only on which instruction you expect the compiler to generate.

### Saturating conversion needs an explicit definition

Some domains want:

```text
NaN        -> 0
+infinity  -> INT_MAX
-infinity  -> INT_MIN
too large  -> nearest endpoint
```

Others want an error.

Others want `std::optional` or `std::expected`.

C++'s built-in cast does not provide saturating semantics. If saturation is required, implement it explicitly and test boundary values including `nextafter` neighbors around the limits.

**Engineering takeaway:** the dangerous part of floating/integer conversion is usually not performance. It is the boundary contract.

---

## 7. Fast inverse square root: brilliant historical hack, poor default modern API

The famous Quake-style inverse-square-root algorithm computes an approximation of:

```text
1 / sqrt(x)
```

using an integer transformation of the floating-point bit pattern followed by Newton-Raphson refinement.

It remains one of the best demonstrations of the relationship between exponent encoding and logarithmic scale.

### Why the magic integer trick works at all

For an IEEE-like positive floating-point value, the integer interpretation of its bits is roughly affine in `log2(x)` because the exponent occupies the high bits and the fraction refines the significand.

The transformation:

```cpp
i = magic - (i >> 1);
```

therefore creates a rough approximation to:

```text
-1/2 * log2(x)
```

in the encoded domain.

Reinterpreting those bits back as a float gives an initial approximation to:

```text
x^-1/2
```

The famous constant is tuned to reduce approximation error over the intended range.

### Newton-Raphson does the real accuracy work

Once an initial estimate `y` exists, the iteration:

```cpp
y = y * (1.5f - 0.5f * x * y * y);
```

is Newton-Raphson applied to inverse square root.

A good initial estimate converges quickly.

One iteration was often enough for graphics workloads where perfect accuracy was unnecessary.

This separation is important:

* the bit hack provides a cheap initial guess;
* Newton iteration provides mathematical refinement.

Modern implementations may use a hardware reciprocal-square-root approximation as the initial guess and apply the same kind of refinement.

### The original type-punning idiom is not portable C++

A union or pointer reinterpretation is historically common in implementations of the algorithm.

Modern C++ should use `std::bit_cast` when it genuinely wants the representation:

```cpp
std::uint32_t i = std::bit_cast<std::uint32_t>(x);
i = 0x5f3759dfu - (i >> 1);
float y = std::bit_cast<float>(i);
```

Even this is only meaningful after establishing the binary32 representation assumptions.

`std::bit_cast` makes the object-model operation legal; it does not make the numerical assumptions portable.

### The algorithm assumes far more than `sizeof(float) == 4`

Four-byte storage alone does not imply:

* IEEE binary32;
* exponent field placement;
* exponent bias 127;
* 23 stored fraction bits;
* the intended NaN/infinity encoding.

A serious implementation should make its platform assumptions explicit.

### Edge cases are part of the algorithm contract

What should happen for:

```text
x = +0
x = -0
x < 0
x = +infinity
x = NaN
x = subnormal
```

?

The historical graphics use case often assumed positive finite normal inputs.

A reusable numeric function cannot leave those cases implicit.

Compare that with:

```cpp
1.0f / std::sqrt(x)
```

whose behavior is integrated with the implementation's math library, floating-point environment, and special-value handling.

### "Fewer instructions" does not mean "faster"

Modern processors may provide:

* pipelined square root;
* reciprocal-square-root estimate instructions;
* SIMD variants;
* FMA-assisted refinement;
* compiler auto-vectorization.

The cost model is completely different from late-1990s hardware.

A hand-written historical hack may lose to code as simple as:

```cpp
1.0f / std::sqrt(x)
```

or to target-specific intrinsics selected by a tuned library.

Measure on the real target with the real error tolerance.

### Performance benchmarking must prevent irrelevant work from being optimized away

A benchmark that computes values but never observes them can measure almost nothing because the optimizer removes the computation.

A numerical microbenchmark should control:

* input generation;
* warm-up effects;
* optimizer visibility;
* vectorization;
* result consumption;
* denormal behavior;
* compiler flags;
* CPU frequency scaling;
* exception/rounding modes.

It should also report **error**, not just time.

A fast approximation is only meaningful relative to an accuracy target.

Useful error measures include:

```text
absolute error
relative error
maximum ULP error
RMS error over the tested domain
worst-case error near boundaries
```

### Approximation APIs should advertise approximation

If an application really benefits from a reciprocal-square-root approximation, encode the contract in the API:

```cpp
float rsqrt_estimate(float x);
float rsqrt_refined(float x);
```

This is clearer than silently substituting an approximation for a mathematically named exact-looking function.

The caller can then decide whether the accuracy/performance trade-off is acceptable.

**Engineering takeaway:** fast inverse square root is still valuable to study because it exposes floating-point representation, logarithmic structure, Newton iteration, and the difference between source-level cleverness and modern hardware performance.

---


## 8. What C++20 and C++23 materially changed for floating-point code

The core arithmetic model did not suddenly become different in C++20 or C++23, but the language and library gained several facilities that remove old folklore, express numerical intent more precisely, and make extended floating-point types first-class.

### C++20: `std::bit_cast` replaces representation-punning folklore

Before C++20, low-level code commonly used:

```cpp
float f = 1.0f;
auto bits = *reinterpret_cast<std::uint32_t*>(&f);
```

or inactive union members.

Those techniques are not the portable C++ object-model solution.

C++20 adds:

```cpp
std::uint32_t bits =
    std::bit_cast<std::uint32_t>(f);
```

This is especially important in floating-point code because representation inspection is common in:

* ULP analysis;
* serializers;
* NaN payload experiments;
* sign/exponent/fraction demonstrations;
* historical bit hacks such as fast inverse square root.

`std::bit_cast` solves the **object-model operation**. It does not prove that the source type is IEEE binary32/binary64. Format assumptions still need to be established separately.

### C++20: `std::endian` separates byte order from floating encoding

`std::endian::native` lets code query native scalar byte order.

That is useful when inspecting the memory representation of a floating-point object, but remember:

> endianness and IEEE field layout are different properties.

If a `float` is known to be IEEE binary32, bit positions in the `std::uint32_t` obtained through `bit_cast` have the expected numeric significance regardless of how the four bytes are ordered in memory.

### C++20: `std::numbers` removes dependency on non-standard constants

Modern code can write:

```cpp
#include <numbers>

double pi = std::numbers::pi;
float pif = std::numbers::pi_v<float>;
```

rather than relying on implementation macros such as `M_PI`.

The standard constants include `e`, `pi`, `sqrt2`, `sqrt3`, `phi`, logarithmic constants, and related inverses.

The variable-template form matters in generic code because it obtains the constant directly in the target floating type:

```cpp
template<std::floating_point T>
T circle_area(T r)
{
    return std::numbers::pi_v<T> * r * r;
}
```

### C++20: `std::midpoint` is not just prettier syntax

Naively computing:

```cpp
(a + b) / 2
```

can overflow even when the mathematical midpoint is representable.

C++20 provides:

```cpp
std::midpoint(a, b)
```

For floating-point arguments, the specification is designed so that at most one inexact operation occurs.

This makes `midpoint` useful not just for integers but also for numerically careful floating-point code.

### C++20: `std::lerp` provides stronger interpolation guarantees

The obvious formula:

```cpp
a + t * (b - a)
```

looks trivial, but extreme values can expose overflow and monotonicity problems.

C++20 adds:

```cpp
std::lerp(a, b, t)
```

with useful guarantees such as exact endpoint behavior for `t == 0` and `t == 1`, and finite results for finite endpoints when `t` lies in the interpolation interval.

That makes `std::lerp` a numerical contract, not merely a spelling convenience.

### C++20: floating-point `<=>` is a partial order

For integers, three-way comparison naturally produces a strong ordering.

Floating point cannot do that because NaN is unordered.

```cpp
double nan = std::numeric_limits<double>::quiet_NaN();

auto result = nan <=> 1.0;

assert(result == std::partial_ordering::unordered);
```

This is an important type-system acknowledgement of IEEE-style comparison semantics.

A floating value is therefore not naturally `std::totally_ordered` merely because the syntax supports `<`, `<=`, and `<=>`.

If NaNs can appear in sortable data, define an explicit total-order policy.

### C++20: the `std::floating_point` concept is a category check, not a format guarantee

C++20 adds:

```cpp
template<class T>
concept std::floating_point;
```

This is useful for constraining generic numerical APIs:

```cpp
template<std::floating_point T>
T relative_error(T a, T b);
```

But satisfying `std::floating_point` does **not** imply:

* IEEE 754;
* radix 2;
* a particular number of bits;
* support for NaN or infinity;
* a particular `long double` ABI.

Use concepts for type-category constraints and `std::numeric_limits` for numerical representation properties.

### C++20: floating-point atomics gained arithmetic operations

Since C++20, floating-point specializations of `std::atomic` provide operations such as:

```cpp
std::atomic<double> total{0.0};

total.fetch_add(1.25);
total.fetch_sub(0.25);
```

This has an obscure but important numerical caveat: the floating-point environment used by the atomic operation may differ from the calling thread's floating-point environment.

So code must not assume that setting:

```cpp
std::fesetround(FE_DOWNWARD);
```

necessarily makes an atomic floating-point addition follow the same runtime rounding environment.

Atomicity and floating-point-environment control are separate contracts.

### C++20 formatting is useful for diagnostics, not a replacement for round-trip policy

`std::format` arrived in C++20 and supports floating-point presentation styles, including hexadecimal floating representation.

For diagnostic output this is much cleaner than hand-maintained `printf` format strings.

For machine interchange, however, decide explicitly whether you need:

* shortest round-trip form;
* fixed precision;
* hexadecimal exact representation;
* locale independence.

`std::to_chars` / `std::from_chars` remain particularly useful for locale-independent, non-allocating text conversion.

### C++23: `std::byteswap` helps at binary-format boundaries

C++23 adds `std::byteswap` for integer types:

```cpp
std::uint32_t bits = std::bit_cast<std::uint32_t>(value);
bits = std::byteswap(bits);
```

This is useful when a binary format specifies a byte order different from the host.

The sequence deliberately expresses two separate operations:

1. `std::bit_cast` obtains the floating-point object's representation as an integer value;
2. `std::byteswap` reverses the bytes of that integer representation.

`std::byteswap` does **not** numerically transform a floating-point value, and it does not define a file/network format by itself.

### C++23: `<stdfloat>` adds optional fixed-width floating-point types

C++23 adds optional aliases:

```cpp
std::float16_t
std::float32_t
std::float64_t
std::float128_t
std::bfloat16_t
```

with corresponding implementation macros:

```cpp
__STDCPP_FLOAT16_T__
__STDCPP_FLOAT32_T__
__STDCPP_FLOAT64_T__
__STDCPP_FLOAT128_T__
__STDCPP_BFLOAT16_T__
```

and literal suffixes such as:

```cpp
0.5f16
0.5f32
0.5f64
0.5f128
0.5bf16
```

when the corresponding optional type is supported.

The standardized properties are worth memorizing:

| Type | Storage bits | Precision bits | Exponent bits |
|---|---:|---:|---:|
| `std::float16_t` | 16 | 11 | 5 |
| `std::float32_t` | 32 | 24 | 8 |
| `std::float64_t` | 64 | 53 | 11 |
| `std::float128_t` | 128 | 113 | 15 |
| `std::bfloat16_t` | 16 | 8 | 8 |

`bfloat16` is particularly instructive: compared with binary16 it spends fewer bits on precision and more on exponent range.

### C++23 fixed-width floating aliases are not aliases for `float` or `double`

This is deliberately different from fixed-width integers.

If `std::float32_t` exists, it names an **extended floating-point type**. It is not permitted to be merely:

```cpp
using float32_t = float;
```

Likewise, `std::float64_t` is not simply a standardized spelling of `double`.

This distinction exists so extended floating-point types can coexist with the three standard floating-point types even when they have the same value representation.

Consequences show up in:

* overload resolution;
* type traits;
* template specialization;
* conversion ranking;
* ABI boundaries.

### C++23 introduced floating-point conversion rank and subrank

Before extended standard-width types became first-class, the familiar hierarchy was mostly:

```text
long double
double
float
```

C++23 formalizes **floating-point conversion rank** and **conversion subrank**.

This matters when standard and extended types have overlapping value sets.

For equal conversion rank, fixed-width floating types such as `std::float32_t` and `std::float64_t` have greater conversion subrank than standard floating types of equal rank.

These rules affect:

* usual arithmetic conversions;
* implicit conversions;
* narrowing decisions;
* overload resolution;
* common floating type selection in math functions.

This is an advanced but important consequence of `<stdfloat>`: "same width" does not mean "same type semantics".

### C++23 common math overloads account for extended floating types

The `<cmath>` overload model was updated so mixed arithmetic arguments can select a common floating-point type using the new rank/subrank system.

That means generic code should prefer the standard overload set rather than manually forcing everything through `double`.

The old habit:

```cpp
return std::sqrt(static_cast<double>(x));
```

can needlessly throw away range or precision when extended floating types are involved.

### C++23 mathematical constants extend naturally to fixed-width floating types

Where the fixed-width floating type exists, the variable-template constants can be instantiated for it:

```cpp
std::numbers::pi_v<std::float32_t>
```

This is another reason to prefer the typed `_v<T>` form in generic code.

### C++23 floating literal suffixes remove an old ambiguity

Before C++23:

```cpp
0.1f  // float
0.1   // double
0.1L  // long double
```

were the standard choices.

With supported C++23 extended types:

```cpp
0.1f32
0.1f64
0.1bf16
```

construct values directly in the intended extended type.

That matters because writing:

```cpp
std::float32_t x = 0.1;
```

first creates a `double` literal and then converts it.

A suffix can make the literal's source type match the target type directly.

### Source-level modernization in this chapter

The accompanying examples intentionally demonstrate the modern rules:

* representation examples use `std::bit_cast`, not inactive-union or unrelated-pointer punning;
* byte order is queried with `std::endian`;
* `std::numbers::pi` replaces `M_PI`;
* interpolation examples use `std::lerp` and `std::midpoint`;
* comparison demonstrates `std::partial_ordering`;
* floating atomics use the C++20 specialization explicitly;
* C++23 `<stdfloat>` code is compiled only when both the language mode and library expose it;
* the historical integer-conversion and inverse-square-root tricks remain, but only after the representation operation itself has been made standard-conforming with `std::bit_cast`.

The goal is not to erase historical techniques. It is to separate:

> **historically clever machine trick**

from:

> **portable modern C++ operation**

so the examples teach both the hardware idea and the current language contract.

---

## 9. Cross-cutting pitfalls worth remembering

The examples above point to several broader rules that apply throughout numerical C++.

### Floating-point arithmetic is not real-number arithmetic

These transformations are not generally semantics-preserving:

```text
(a + b) + c  <=>  a + (b + c)
(a * b) / b  <=>  a
x - x        <=>  0
x * 0        <=>  0
```

Why?

Because of:

* rounding;
* overflow;
* underflow;
* NaN;
* infinity;
* signed zero;
* traps and exception flags.

For example:

```cpp
double x = std::numeric_limits<double>::infinity();

x - x // NaN, not 0
```

and with NaN:

```cpp
x * 0
```

need not be zero.

These details explain why strict floating-point rules limit optimizer algebra.

### Floating-point values are a finite discrete set

Thinking "approximate real number" is useful at a high level, but at low level a floating-point type is a finite set of representable values plus special encodings.

`std::nextafter` exposes this directly.

This model helps explain:

* rounding;
* ULPs;
* quantization;
* precision loss;
* integer exactness limits;
* why tiny additions disappear.

### Adding a tiny value can do nothing forever

For sufficiently large `x`:

```cpp
x + tiny == x
```

If a loop repeatedly does:

```cpp
x += tiny;
```

and each addition individually rounds back to `x`, repeating the operation does not eventually accumulate hidden fractional state. The lost amount is gone after each rounded operation.

This is one reason numerical accumulation order matters.

### Overflow and underflow are not symmetric

Overflow usually leaves the finite range and may produce infinity on IEC-559 systems.

Underflow first enters the subnormal range, if supported, where precision gradually decreases before reaching zero.

So the low end of floating-point range is not simply the high-end story mirrored around zero.

### A subnormal has less precision than a normal value

Normal binary values have an implicit leading significand bit.

Subnormals do not.

As values approach zero through the subnormal region, the absolute spacing stays fixed while the number of meaningful leading bits falls.

Gradual underflow preserves continuity at the cost of relative precision.

### Negative zero can be created without writing `-0.0`

Examples include:

* underflow of a negative result;
* directed rounding;
* `std::copysign`;
* some transcendental functions.

Therefore testing source syntax is not enough; if zero sign matters, inspect it with `std::signbit`.

### NaN propagation is useful, but not a validation strategy

NaNs can make invalid numerical states propagate conspicuously through a computation.

But they are poor substitutes for input validation because:

* comparisons with NaN are unusual;
* payload propagation is not a portable contract;
* fast-math modes may assume NaNs do not occur;
* some APIs canonicalize them;
* NaNs do not carry structured domain-error information.

Use NaN deliberately, not accidentally.

### Reproducibility is a separate requirement from accuracy

Two implementations can both be accurate and still produce different last bits.

Bitwise reproducibility may require fixing:

* compiler and flags;
* target ISA;
* vector width;
* FMA policy;
* reduction order;
* rounding mode;
* FTZ/DAZ mode;
* math-library implementation.

If the requirement is merely numerical accuracy within a tolerance, demanding bitwise equality may be unnecessarily expensive.

If the requirement is deterministic simulation, lockstep networking, scientific regression testing, or consensus logic, those details may be essential.

### Never use floating point for consensus-critical equality without a very deliberate model

Distributed consensus, cryptographic protocols, replicated simulations, and cross-platform deterministic state machines should be especially cautious.

A value that depends on:

* implementation math libraries;
* FMA contraction;
* vectorization;
* runtime FP state;

is a poor consensus primitive unless the environment is tightly specified.

Sometimes fixed-point or integer arithmetic is the simpler engineering solution.

---

## Useful standard-library tools

For low-level and numerical floating-point work, the most useful facilities include:

```cpp
// Representation / properties
std::numeric_limits<T>
std::bit_cast
std::endian

// Classification
std::fpclassify
std::isfinite
std::isinf
std::isnan
std::isnormal
std::signbit

// Neighboring values
std::nextafter
std::nexttoward

// Decomposition / scaling
std::frexp
std::ldexp
std::ilogb
std::logb
std::scalbn
std::scalbln
std::modf

// Stable special forms
std::fma
std::hypot
std::log1p
std::expm1

// Rounding
std::floor
std::ceil
std::trunc
std::round
std::rint
std::nearbyint
std::lround
std::lrint

// Special values and sign manipulation
std::copysign
std::numeric_limits<T>::infinity()
std::numeric_limits<T>::quiet_NaN()
std::numeric_limits<T>::denorm_min()

// Floating-point environment
std::feclearexcept
std::fetestexcept
std::fegetround
std::fesetround
std::fegetenv
std::fesetenv
std::feholdexcept
std::feupdateenv

// C++20 mathematical constants
std::numbers::pi_v<T>

// Locale-independent text conversion
std::to_chars
std::from_chars
```

The important habit is to use these facilities according to their semantic purpose rather than reimplementing the behavior with bit tricks or algebraically similar formulas.


---
---

## Rules worth keeping in working memory

The central distinction in floating-point representation and arithmetic is between what the C++ type system guarantees, what the implementation commonly does, and what an API merely assumes. Carrying that distinction from declarations through operations avoids most of the surprises discussed above.

> A valid low-level operation needs a language-level contract, not just a machine-level outcome that looks plausible.

### Core rules

The following rules condense the chapter into reviewable decisions:

1. **Treat IEEE-754 properties as properties to check, not universal type guarantees.**
2. **Choose comparison tolerances from the algorithm's error model.**
3. **Account for NaN, infinities, signed zero, and nonuniform spacing.**
4. **Distinguish compiler FP optimization modes from numerical correctness guarantees.**


### Pitfalls at a glance

These recurring failures are particularly useful to recognize in code review because each starts from a plausible but insufficient assumption.

| Pitfall | What happens | Do instead |
|---|---|---|
| Global `epsilon()` tolerance | Works only near a limited scale | Use domain-specific absolute/relative tolerances |
| Float-to-int out of range | Built-in conversion has undefined behavior | Validate finite range before casting |
| Unrelated typed-pointer punning | Can violate aliasing rules | Use `std::bit_cast` for representation transfer |

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

Use this checklist when changing floating-point representation and arithmetic code or reviewing low-level interfaces:

- [ ] Have we distinguished language guarantees from platform-specific observations?
- [ ] Are all input domains and conversion or lifetime preconditions explicit?
- [ ] Can the relevant edge cases be tested without executing undefined behavior?
- [ ] Does the chosen API encode as much of the intended contract as practical?

---

## Diagnostics, useful compiler settings and extensions

Compilers can diagnose many suspicious uses of floating-point representation and arithmetic, but they cannot infer every application invariant. A clean warning build is useful evidence, not proof: tools can recognize some invalid expressions statically and others only when particular paths execute.

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

The core model of floating-point representation and arithmetic evolved incrementally; newer standards add safer vocabulary without retroactively rewriting all legacy expressions. The milestones below separate changes to the language from changes to available library interfaces.

### C++98/03: The original model

`float`, `double`, and `long double` already offered implementation-dependent precision and formats.

### C++11: Stronger types and interfaces

`constexpr` permits selected constant arithmetic and type-level checks.

### C++14: Incremental refinement

No fundamental change to binary floating semantics.

### C++17: Library and deduction evolution

Further mathematical special functions and library utilities broaden numerical APIs.

### C++20: Modern vocabulary

`std::numbers`, `std::midpoint`, `std::lerp`, `std::bit_cast`, and floating atomics become available.

### C++23: Further standard facilities

Optional `<stdfloat>` types and conversion rank/subrank rules formalize extended formats.

### C++26: Emerging improvements

Library constexpr math and other improvements depend on actual implementation support.

When documenting a facility, state its required language/library version rather than inferring support from the compiler's branding.

---

## Migration note for Java / Python / C# developers

Readers familiar with managed runtimes often expect floating-point representation and arithmetic to come with runtime metadata, automatic lifetime management, or checked failures. In C++, some of those services come from a chosen library type, while raw language mechanisms may deliberately expose more responsibility to the caller. The important translation is from a *runtime guarantee* in one language to a *type, contract, and lifetime guarantee* in C++.

### Where intuition transfers and where it breaks

The same apparent operation may have a different failure mode or storage model in each language.

| Concept | Java | Python | C# | C++ reality |
|---|---|---|---|---|
| fp | well-defined numeric runtime conventions | IEEE floating types | Binary floating point | `float`/`double` |
| Invalid access/operation | Usually throws or is checked | Usually raises an exception | Often throws in safe code | May be ill-formed, defined, unspecified, or undefined depending on the operation |
| Lifetime | Managed object reachability | Reference counting / GC | Managed GC | Automatic, dynamic, and explicitly borrowed lifetimes coexist |

The distinction matters at API boundaries: a familiar surface syntax does not imply familiar failure behavior.

### Common wrong assumptions

One tempting assumption is that a successful local test demonstrates that an operation is safe. For the low-level C++ rules in this chapter, a test can demonstrate behavior of one build, but cannot establish portability or rule out undefined behavior. Another is that an object is kept alive by every handle referring to it; non-owning pointers, references, and views do not do that. Finally, a managed-language exception should not be presumed to exist at an equivalent C++ failure point.

### Idiomatic C++ replacement

The following mappings help make intent explicit without imitating a managed runtime mechanically.

| Habit from managed languages | Idiomatic C++ |
|---|---|
| Floating precision | Design for numeric error, not bitwise equality by default |
| Serialization | Use round-trip precision or `to_chars`/`from_chars` |
| Expect automatic runtime bounds/lifetime checks | Select an owning container or checked API; validate preconditions explicitly |

Use these choices because they express the program's requirements, not merely because they resemble familiar constructs from another language.

---

## Further reading

These references document the underlying language rules and library contracts. They are starting points for checking precise preconditions; the chapter's examples explain how the rules interact.

### Standard and language reference

* [language/types](https://en.cppreference.com/w/cpp/language/types)
* [types/numeric_limits](https://en.cppreference.com/w/cpp/types/numeric_limits)
* [numeric/math](https://en.cppreference.com/w/cpp/numeric/math)
