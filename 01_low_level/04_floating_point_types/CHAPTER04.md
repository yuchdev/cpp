# Chapter 04. Floating-Point Types: Representation, Error, and the Cost of Approximation

Most C++ programmers learn early that `0.1` is not exactly representable in ordinary binary floating point. That observation is correct, but it is not a satisfactory model for numerical software. A system can fail even when every operand is exactly representable, every instruction behaves as specified, and no compiler diagnostic appears. Changing the order of a reduction may change the result. A conversion can be undefined before its destination ever receives a value. A comparison intended to detect a negligible difference may reject two adjacent representable values or accept two values that are far apart relative to their scale.

Floating point is not a defective implementation of real arithmetic. It is a finite arithmetic system with representation, rounding, and exceptional-value rules. Using it correctly requires understanding the distinction between a mathematical expression, a C++ expression, and the execution permitted by the selected compiler, library, and floating-point environment.

The companion programs examine [representation](00_floating_point_representation/floating_point_representation.cpp), [sign/exponent/significand extraction](01_extract_fp_components/extract_fp_components.cpp), [comparison](02_compare_fp/compare_fp.cpp), [mathematical functions](03_fp_functions/fp_functions.cpp), [floating-point errors and the environment](04_fp_errors/fp_errors.cpp), [integer conversions](05_fast_integer_cast/integer_cast.cpp), and [fast inverse square root](06_fast_reverse_sqrt/reverse_sqrt.cpp). The repository's baseline is C++20, with guarded demonstrations of C++23 facilities. We will use the same technical examples to build a more general engineering model.

## 1. A floating-point type is not a particular binary format

C++ defines three standard floating-point types—`float`, `double`, and `long double`—but does not require the first to be IEEE binary32, the second to be IEEE binary64, or the third to provide a specific extended format. Their storage sizes are ordered:

```cpp
sizeof(float) <= sizeof(double);
sizeof(double) <= sizeof(long double);
```

On common targets, `float` uses binary32, with 24 binary digits of precision in 32 bits of storage, and `double` uses binary64, with 53 binary digits in 64 bits. Those are very useful expectations when working with mainstream ABIs, not universal guarantees of the source language.

The third type is particularly variable. MSVC commonly gives `long double` the same effective representation as `double`; many x86 Unix ABIs support the x87 80-bit extended format, often padded to 12 or 16 bytes in memory; some platforms use IEEE binary128; and certain PowerPC environments have used double-double representations. Storage size alone does not tell us the number of significant digits.

An implementation advertises its floating-point properties through `std::numeric_limits`:

```cpp
#include <limits>

using L = std::numeric_limits<double>;

static_assert(L::is_iec559); // require IEC 559/IEEE-style semantics
static_assert(L::radix == 2);
static_assert(L::digits == 53);
static_assert(sizeof(double) == 8);
```

These assertions are appropriate only when an algorithm genuinely requires those properties. `is_iec559` indicates that the implementation claims the relevant IEC 559 characteristics, but a bit-level decoder should still check the exact encoding and layout assumptions it depends on. Code that performs ordinary arithmetic and uses numerical decomposition functions needs fewer such restrictions.

The common radix is two, but `FLT_RADIX` from `<cfloat>` is not required to equal two. This matters immediately to an algorithm based on a binary exponent field: an assumed layout of one sign bit, eight exponent bits, and 23 stored fraction bits describes IEEE binary32, not C++ `float` in the abstract.

### 1.1 Precision and scale occupy different parts of the representation

For a normal positive binary32 value, the encoded sign, exponent, and fraction represent

\[
(-1)^s(1+f)2^{E-127},
\]

where \(f\) denotes the fractional value of the 23 stored fraction bits and \(E\) is the encoded exponent. The leading one is implicit for normal values, so the 23 stored fraction bits provide 24 significant bits. Binary64 follows the same general model with 52 fraction bits, 53 significant bits, and exponent bias 1023.

The significand's precision is fixed while the exponent changes its scale. Consequently, representable floating-point values are not uniformly spaced across the real line. For a binary format with \(p\) significant bits, the spacing between ordinary normal values in a binade near \(2^e\) is \(2^{e-(p-1)}\). For binary64 the gap immediately above one is \(2^{-52}\), while the gap immediately above two is twice as large. At very large magnitudes, even adjacent values can be separated by an enormous absolute interval.

This is the reason normal binary floating point provides approximately *relative* precision, not fixed absolute precision. A unit in the last place, or ULP, describes a representational step at a particular magnitude. It is not a universal absolute distance.

The much-cited constant

```cpp
std::numeric_limits<double>::epsilon()
```

is the difference between 1.0 and the next larger representable value—approximately \(2.220446049250313 \times 10^{-16}\) for binary64. It is not the smallest nonzero number, not the smallest spacing anywhere in the format, and not the acceptable error for an arbitrary computation. Under the usual round-to-nearest model the unit roundoff for one correctly rounded normal operation is commonly taken as approximately half that epsilon; an algorithm's accumulated error is a separate subject.

The `min()` member of `numeric_limits` is an equally common trap. For a floating type it denotes the smallest positive *normal* value. The most negative finite value is returned by `lowest()`, while `denorm_min()` gives the least positive subnormal when such values are supported. An integer programmer who reads `min()` as “most negative” can accidentally reject almost the entire negative half of the domain.

### 1.2 Exact integers, inexact fractions, and decimal text

With \(p\) binary significant digits, all integers of magnitude at most \(2^p\) are exactly representable. Above that boundary, consecutive integers begin to collapse onto the same stored value. For binary32 the boundary is \(2^{24}=16\,777\,216\):

```cpp
float a = 16'777'216.0f;
float b = 16'777'217.0f;
assert(a == b); // on the expected binary32 model
```

For binary64 it is \(2^{53}\). A `double` may represent values near \(10^{308}\), but it cannot uniquely represent every 64-bit integer. This is consequential for database identifiers, telemetry sequence numbers, timestamps, hash keys, financial import/export, JSON pipelines, and JavaScript interfaces.

The problem with decimal `0.1` is related but different. A binary floating format represents fractions whose denominators are powers of two. Thus `0.5` and `0.125` are exact, while one decimal tenth has an infinite repeating binary expansion. The stored `double` value is a nearby representable binary rational. Decimal floating point has its own representational limitations; the appropriate radix depends on the domain rather than on a universal notion of an “exact computer number.” Money may be better modeled using integer minor units or a decimal arithmetic type with an explicit rounding contract.

Text conversion introduces another precision boundary. For binary64:

```cpp
std::numeric_limits<double>::digits       // 53 binary digits
std::numeric_limits<double>::digits10     // typically 15
std::numeric_limits<double>::max_digits10 // typically 17
```

`digits10` concerns a count of decimal digits that can survive a decimal-to-binary-to-decimal round trip under its specification. `max_digits10` is the digit count sufficient to print any value of the type and recover it from parsing. For explicit diagnostic serialization one may write:

```cpp
std::cout
    << std::setprecision(
           std::numeric_limits<double>::max_digits10)
    << value;
```

This provides a useful round-trip precision policy, subject to a compatible conversion implementation and format. Locale-independent `std::to_chars` and `std::from_chars` are attractive for machine interchange, while hexadecimal floating notation can directly expose a binary value's significand and exponent. Persistent formats need to specify parsing, rounding, NaN treatment, and signed zero as well as the number of decimal digits.

### 1.3 Fixed-width floating types are not ordinary typedefs

C++23 introduced optional types in `<stdfloat>`, including `std::float16_t`, `std::float32_t`, `std::float64_t`, `std::float128_t`, and `std::bfloat16_t`. These names are available only where the implementation provides the corresponding extended floating-point formats. Unlike a fixed-width integer alias, `std::float32_t` is not merely another spelling for ordinary `float`, even on a platform where their representations are equivalent. It names a distinct extended floating type.

The standardized format characteristics help explain why these types exist:

| Type | Storage bits | Precision bits | Exponent bits |
|---|---:|---:|---:|
| `float16_t` | 16 | 11 | 5 |
| `float32_t` | 32 | 24 | 8 |
| `float64_t` | 64 | 53 | 11 |
| `float128_t` | 128 | 113 | 15 |
| `bfloat16_t` | 16 | 8 | 8 |

The bfloat16 design devotes relatively few bits to precision and more to exponent range, making it a substantially different trade-off from IEEE binary16. An algorithm that needs a particular format should test for the relevant `__STDCPP_FLOAT16_T__`, `__STDCPP_FLOAT32_T__`, `__STDCPP_FLOAT64_T__`, `__STDCPP_FLOAT128_T__`, or `__STDCPP_BFLOAT16_T__` support macro before using it.

C++23 also formalized floating-point conversion rank and subrank. Extended types can have equal value range and precision to standard types without being the same types; fixed-width types of equal rank receive the specified subrank preference. This influences implicit conversions, usual arithmetic conversions, overload resolution, narrowing decisions, and common-type selection in mathematical functions. It is precisely why introducing `<stdfloat>` cannot be understood merely as adding attractive aliases.

## 2. The bit representation: where normal arithmetic stops

A representation diagram showing sign, exponent, and fraction is useful, but a decoder must understand all the encodings, not just normal finite values. For binary32, the bit pattern for \(1.5\) can be written as

```text
0 01111111 10000000000000000000000
```

The exponent field is 127, making the unbiased exponent zero. The stored fraction contributes 0.5; the implicit leading one produces the significand 1.5.

An exponent field of zero changes the interpretation. For binary32 subnormals, the value is

\[
(-1)^s f\,2^{-126},
\]

with no implicit leading one. Zero is the special case in which that fraction is also zero. Subnormals bridge the gap from the smallest normal magnitude to zero, a mechanism called *gradual underflow*. The absolute spacing is fixed through this region, but relative precision is progressively lost.

At the other end, an all-ones exponent is reserved. In binary32, exponent 255 with zero fraction denotes positive or negative infinity according to the sign bit; a nonzero fraction denotes a NaN. A decoder that treats this field as an ordinary huge finite exponent is wrong. The numerical classification can be obtained directly with `std::fpclassify` and its `FP_ZERO`, `FP_SUBNORMAL`, `FP_NORMAL`, `FP_INFINITE`, and `FP_NAN` results.

### 2.1 Signed zero and NaN carry representation details

On IEEE-conforming implementations, positive and negative zero compare equal:

```cpp
assert(+0.0 == -0.0);
```

Their behavior is nonetheless not identical. Division by positive or negative zero normally produces correspondingly signed infinity under the default nontrapping IEC-559 behavior. The sign can matter to complex-number branch cuts, intervals, directional limits, and serialization. A test such as `x < 0` cannot find negative zero; `std::signbit(x)` can.

NaN encodings can include a sign bit, quiet/signaling distinctions, and payload bits. Those details can be useful in controlled diagnostics or specialized protocols, but a portable application should not assume that arithmetic, conversion, formatting, or optimization preserves a NaN payload or gives its sign ordinary numeric meaning. Some hardware and compilers canonicalize NaN representations.

### 2.2 Observing bits without violating the C++ object model

Several historical ways of reinterpreting a floating object are invalid as portable C++:

```cpp
float f = 1.0f;
// auto bits = *reinterpret_cast<std::uint32_t*>(&f); // bad
```

Dereferencing an unrelated integer pointer can violate type-access and aliasing rules. Reading an inactive union member is likewise not a general-purpose portable type-punning mechanism. Mapping a bit-field struct onto a floating format is unreliable because bit-field packing and allocation order are implementation-defined:

```cpp
struct FloatBits {
    unsigned fraction : 23;
    unsigned exponent : 8;
    unsigned sign : 1;
};
```

Since C++20, `std::bit_cast` provides the intended object-representation operation between equal-size trivially copyable types:

```cpp
static_assert(sizeof(float) == sizeof(std::uint32_t));

float f = 1.0f;
std::uint32_t bits = std::bit_cast<std::uint32_t>(f);
```

Before C++20, `std::memcpy` provides a valid route:

```cpp
std::uint32_t bits;
std::memcpy(&bits, &f, sizeof bits);
```

Neither operation *proves* that the source uses binary32. Once that independent assumption is established, integer shifts and masks expose the fields:

```cpp
auto sign = bits >> 31;
auto exponent = (bits >> 23) & 0xffu;
auto fraction = bits & 0x7fffffu;
```

For binary64 the decoder must instead use an appropriate 64-bit unsigned representation, 11 exponent bits, bias 1023, and 52 stored fraction bits. Simply templating the 32-bit code without replacing those constants yields a broken “generic” decoder. Choosing `long` for bit storage is an additional portability error because LP64 and LLP64 assign different widths to it.

An integer obtained by bit-casting represents numeric bit positions independent of how the bytes are placed in memory; shifts and masks operate on that integer value. Byte-by-byte inspection of the object exposes endianness. These are different questions. C++20 `std::endian` helps query byte order; C++23 `std::byteswap` can reverse bytes in an appropriate integer representation. Neither facility defines the IEEE encoding by itself or decides the serialization format for you.

If the application merely needs a floating number's scale, manual encoding access is unnecessary. `std::frexp` decomposes a finite nonzero value into a significand and exponent:

```cpp
int e{};
double m = std::frexp(x, &e);
// x == m * 2^e, with 0.5 <= abs(m) < 1 for binary x
double reconstructed = std::ldexp(m, e);
```

The functions `std::ilogb`, `std::logb`, `std::scalbn`, and `std::scalbln` provide related numerical operations without assuming field widths or memory layout. Use bit decoding for representation questions and numerical decomposition for numerical questions.

## 3. Equality is an application policy, not a library constant

Floating-point equality has acquired a misleading reputation. The advice “never use `==` with floats” is almost as unhelpful as “always compare using epsilon.” Exact equality is appropriate when the domain requires exact representation equality or when the algorithm guarantees an exact sentinel or value:

```cpp
if (x == 0.0) {
    // valid when zero is an exact algorithmic state
}
```

It can be correct for copied values, powers of two known to be exactly representable, explicit sentinels, equality against infinity, and tests requiring an exact stored result. The problem is comparing two independently rounded calculations as though the real-number algebra guaranteed the same bit pattern.

A global test such as

```cpp
std::fabs(a - b) < std::numeric_limits<double>::epsilon()
```

is generally not a useful numerical policy. At magnitude \(10^{100}\), adjacent binary64 values are separated by far more than epsilon near one. Near zero, the opposite scaling problem arises: a relative comparison can be excessively strict or unstable.

### 3.1 Absolute, relative, and representational tolerance

A common comparison combines absolute and relative tolerances:

```cpp
bool nearly_equal(double a, double b,
                  double rel_tol, double abs_tol)
{
    if (a == b)
        return true; // also equal infinities and signed zeros

    if (!std::isfinite(a) || !std::isfinite(b))
        return false;

    const double diff = std::fabs(a - b);
    const double scale = std::max(std::fabs(a), std::fabs(b));
    return diff <= std::max(abs_tol, rel_tol * scale);
}
```

This is a useful *structure*, not a universal formula. Even here, `a-b` or `rel_tol * scale` can overflow when arguments approach representational extremes; a production generic comparison must define its finite-domain and overflow policy.

The essential question is where the tolerances come from. They should derive from measurement resolution, expected numerical error, dimensional scale, conditioning, iteration stopping criteria, or protocol requirements. An arbitrarily chosen multiple of machine epsilon might be justified for one tightly bounded computation but cannot stand in for analysis.

For example, \(10^{-300}\) and \(2\times10^{-300}\) differ by a tiny absolute amount but by 50 percent relative to the larger. Whether they count as equivalent depends entirely on the problem. An absolute tolerance that makes sense for meters may be meaningless for astronomical distances, and a relative-only tolerance is often unsuitable when comparing values close to zero.

The `std::nextafter` function permits direct exploration of adjacent representable values:

```cpp
double next = std::nextafter(
    x, std::numeric_limits<double>::infinity());
```

Counting ULP steps is useful for boundary tests, debugging, rounding analysis, and comparisons between two implementations of the same operation. But a threshold of “within four ULPs” says how far apart representations are, not whether a difference is physically or financially acceptable. ULP-distance algorithms also need careful sign handling, signed-zero treatment, powers-of-two boundaries, NaNs, infinities, and format guarantees.

### 3.2 NaN changes the logic of ordering

An ordinary quiet NaN is unordered with every floating value, including itself:

```cpp
q == q // false for NaN
q < q  // false
q > q  // false
q != q // true
```

The classification functions `std::isnan` and `std::isunordered` make this explicit. C++20's three-way comparison returns a partial ordering:

```cpp
auto relationship = a <=> b;
// can equal std::partial_ordering::unordered
```

The distinction matters in sorting and ordered containers. A comparator that uses ordinary `<` on data containing NaNs may violate the strict weak ordering expected by `std::sort` and `std::map`. An application should reject NaNs or give them an explicit position in the order—before finite values, after them, or using a defined payload-sensitive scheme. IEEE 754 has total-order concepts, but ordinary C++ relational operators deliberately do not impose such an order.

Signed zero introduces a similar, though narrower, policy decision. `-0.0 == +0.0` is true; `std::signbit` distinguishes them. Most numerical comparisons should treat them as equal, whereas a bit-exact cache key or serialization may choose otherwise.

### 3.3 Non-associativity and cancellation are algorithmic effects

Addition is not generally associative in floating point:

```cpp
(a + b) + c
a + (b + c)
```

may differ even when every individual operation is correctly rounded. If `a` is enormous compared with `b`, `a+b` may round back to `a` before `c` is added. Reordering a loop, vectorizing it, or distributing a reduction across threads can therefore alter the last bits—or more, when cancellation is severe.

Compensated summation (Kahan or Neumaier), pairwise summation, appropriately ordered accumulation, wider intermediate precision, or exact accumulators can reduce error where the domain requires it. Which method is appropriate depends on conditioning, cost, and the desired reproducibility model.

Subtracting nearly equal values can produce *catastrophic cancellation*: the leading significant digits cancel, leaving a result whose relative error is large because the inputs were already rounded. Increasing a comparison tolerance afterward does not restore the lost information. Stable numerical programming often means reformulating the computation before the subtraction occurs.

## 4. Mathematical functions carry numerical contracts

The header `<cmath>` is not simply a collection of convenient spellings for formulas. Many facilities exist precisely because straightforward algebra can lose accuracy or violate exceptional-value expectations.

Consider rounding. `std::floor` rounds toward negative infinity, `std::ceil` toward positive infinity, and `std::trunc` toward zero. These functions return floating-point values and their directions do not depend on the current floating-point rounding mode.

`std::round` rounds to the nearest integer-valued floating value with halfway cases away from zero, independently of the environment. Hence `round(2.5)` is 3 and `round(-2.5)` is -3. `std::rint`, by contrast, follows the active rounding mode; under typical `FE_TONEAREST` semantics the halfway cases select an even integer, so `rint(2.5)` is 2 and `rint(3.5)` is 4. `std::rint` may raise `FE_INEXACT`; `std::nearbyint` follows the active mode while avoiding that flag on IEC-559 implementations.

Functions with an `l` or `ll` prefix in names such as `lround`, `llround`, `lrint`, and `llrint` return integer types. The `round` family uses ties away from zero, while the `rint` family follows the current mode. Invalid or out-of-range results have their specified domain/error signaling behavior; none should be substituted casually for the language's plain floating-to-integer cast.

In particular, the rounding environment does not change the built-in conversion rule:

```cpp
std::fesetround(FE_UPWARD);
int n = static_cast<int>(1.2); // still 1
```

The cast discards the fractional part toward zero. If an API promises another rounding policy, it must say so explicitly.

### 4.1 One rounding is different from two

The operation `std::fma(a,b,c)` computes the fused multiply-add, conceptually evaluating \(ab+c\) without rounding the multiplication intermediate and then rounding the final result once. The expression `a*b+c` can round after multiplication and again after addition. FMA is often more accurate, especially under cancellation, yet it may produce a *different* value.

Compilers may also contract a multiply-and-add expression into a fused operation under their permitted rules and options. Architecture, instruction selection, optimization level, and floating-point contraction policy can therefore affect bitwise results. Accurate computation and reproducible computation are not identical requirements.

### 4.2 Stable expressions are rarely the most obvious ones

For small `x`, forming `1.0 + x` may round to exactly 1.0, causing `std::log(1.0+x)` to return zero when the desired \(\log(1+x)\) is nonzero. `std::log1p(x)` uses algorithms designed to retain accuracy in this regime. Similarly, `std::expm1(x)` avoids the cancellation in `std::exp(x)-1.0\).

The naive hypotenuse formula

```cpp
std::sqrt(x * x + y * y)
```

can overflow an intermediate even when the true hypotenuse is finite, or underflow small squared terms unnecessarily. `std::hypot(x,y)` communicates the desired numerical operation and allows a carefully scaled implementation.

`std::frexp` and `std::ldexp` decompose and scale using radix powers, and `std::scalbn` is useful when the desired scaling matches the implementation radix. They are usually preferable to spelling scaling as `x * std::pow(2.0,n)` when the intent is manipulation of the floating exponent.

The remainder functions `std::fmod` and `std::remainder` are not synonyms. `fmod` uses a quotient truncated toward zero, while `remainder` selects a nearest-integer quotient with the defined halfway rule. The magnitude and sign of the remainder can differ. Choosing one by name alone can silently change an algorithm.

### 4.3 Classification and constants

`std::isnormal` does not mean “not NaN.” It is true only for finite, normal, nonzero numbers; zero, subnormals, infinities, and NaNs all fail. Use `std::fpclassify`, `std::isfinite`, `std::isinf`, `std::isnan`, `std::isnormal`, and `std::signbit` to test the actual property an algorithm requires.

For constants, C++20's `<numbers>` is preferable to non-standard macros such as `M_PI`:

```cpp
double pi = std::numbers::pi;
float pi_f = std::numbers::pi_v<float>;

template<std::floating_point T>
T circle_area(T r) {
    return std::numbers::pi_v<T> * r * r;
}
```

The same vocabulary includes `e`, `sqrt2`, `sqrt3`, `phi`, logarithmic constants, and useful inverses. For special values, query `numeric_limits<T>` for infinity, quiet or signaling NaN, and subnormal support before assuming that a non-IEC implementation provides the desired semantics.

Subnormals deserve attention both numerically and operationally. Gradual underflow preserves useful small magnitudes but loses relative precision. Some processors and instruction paths historically handled subnormal arithmetic much more slowly; the size of any performance penalty is hardware- and workload-dependent. Hardware controls often called FTZ (flush-to-zero) and DAZ (denormals-are-zero) can change performance at the cost of changing numerical semantics. They are not general portable C++ floating-environment controls.

## 5. The floating-point environment is part of program state

A floating-point “exception” usually does not mean a C++ exception object. Where supported, the floating-point status flags include `FE_DIVBYZERO`, `FE_INEXACT`, `FE_INVALID`, `FE_OVERFLOW`, and `FE_UNDERFLOW`. On an IEC-559-style implementation under the normal nontrapping mode, an operation may return infinity or NaN and set a status flag without ever entering a C++ `catch` block.

Inexact is the ordinary condition of approximate arithmetic. Many entirely successful calculations cannot be rounded exactly and raise `FE_INEXACT`. Treating this flag as “the computation failed” would make ordinary floating-point arithmetic appear broken.

Status flags are generally sticky. A flag set by an earlier operation remains set until cleared or the environment is changed. To attribute observed flags to a computation, establish a clean baseline:

```cpp
std::feclearexcept(FE_ALL_EXCEPT);
do_work();
int flags = std::fetestexcept(FE_ALL_EXCEPT);
```

Even this requires an implementation and compilation mode that respect floating-environment access. The environment is associated with the executing thread, so modifying rounding state in one thread is not a request to change the process's arithmetic everywhere. Initial thread environments and inheritance details depend on the platform, and compilation assumptions remain a separate concern.

### 5.1 Rounding modes and compiler options cannot be separated

The standard library exposes `std::fegetround` and `std::fesetround`, with supported modes such as `FE_TONEAREST`, `FE_DOWNWARD`, `FE_UPWARD`, and `FE_TOWARDZERO`. But calling `fesetround` does not magically undo compiler transformations made under an assumed default rounding mode.

The mechanism traditionally associated with observable environment access is `#pragma STDC FENV_ACCESS ON`, whose implementation support varies. GCC/Clang options such as `-frounding-math` and MSVC's strict floating-point modes may constrain optimizations, but documented behavior and actual tests are necessary. C++ runtime rounding state, optimizer assumptions, and instruction behavior jointly determine the result.

Floating-point constant evaluation takes place in the compiler, not by executing runtime instructions in the calling thread's current FPU state. A `constexpr` result and the runtime evaluation of a visually identical expression need not respond to `fesetround` in the same way.

The `-ffast-math` family makes the distinction dramatic. It can permit reassociation, reciprocal substitution, assumptions that signed zero or NaNs do not matter, nontrapping arithmetic, and other transformations incompatible with strict numerical reasoning. It is not merely permission to choose a faster hardware instruction. If an algorithm tests NaNs, distinguishes zero signs, monitors flags, or requires a fixed evaluation order, fast-math may invalidate its intended contract.

Excess precision introduces another historical source of differences. The x87 arithmetic unit could keep intermediate `double` calculations in wider 80-bit registers and round them only when a value was stored. Debugging, register allocation, or a logging change could alter where rounding occurred. Mainstream x86-64 code generally uses SSE/AVX arithmetic for binary32/binary64, reducing this particular phenomenon, but target-dependent excess precision still matters. FMA contraction similarly changes where rounding occurs.

To specify deterministic numerical behavior, one may need to fix the compiler and version, optimization settings, target ISA, vector width, intermediate precision, FMA policy, math-library implementation, reduction order, and runtime rounding/FTZ/DAZ state. Two systems can both be numerically accurate and still disagree in their last bits.

### 5.2 Traps are not portable C++ exception handling

Standard C++ provides interfaces for querying flags and managing the floating-point environment; it does not supply a universal API to enable processor traps for particular flags. Some Unix environments provide `feenableexcept`; x86 hardware has x87 control words and SSE/AVX `MXCSR`; MSVC CRT facilities and ARM control registers supply other platform-specific mechanisms.

The POSIX signal `SIGFPE` can report integer as well as floating arithmetic faults, with reasons such as `FPE_INTDIV`, `FPE_FLTDIV`, `FPE_FLTOVF`, and `FPE_FLTINV`. Its name alone does not identify the fault. Recovering after a hardware trap is difficult: signal handlers have signal-safety restrictions, and jumping out with `siglongjmp` may bypass C++ stack unwinding and object cleanup. Such handlers are better used for diagnostics, controlled test harnesses, crash capture, or specialist runtimes than as ordinary arithmetic error handling.

Code that temporarily changes the environment should save and restore it on all paths, preferably through RAII:

```cpp
std::fenv_t previous;
std::fegetenv(&previous);
// change floating environment, perform controlled work
std::fesetenv(&previous);
```

For code that needs to suppress, save, and later re-raise flags, `std::feholdexcept` and `std::feupdateenv` provide additional tools. A library function that leaves the caller's rounding mode modified has leaked thread-local numerical state.

Finally, `errno`, floating-point flags, and special results such as NaN or infinity are distinct error-reporting channels. The `math_errhandling` facility advertises supported mechanisms for mathematical library functions. Portable error handling cannot assume that every libc uses the same channel.

## 6. Floating-to-integer conversion: an undefined-behavior boundary

The built-in conversion from a floating-point value to an integer truncates toward zero:

```cpp
static_cast<int>( 9.99) //  9
static_cast<int>(-9.99) // -9
```

This is independent of the active floating-point rounding mode. On x86 a compiler may select truncating conversion instructions such as `cvttss2si` or `cvttsd2si`, but instruction choice is not the language contract.

If the truncated value cannot be represented in the destination integer type, the behavior is undefined. The danger includes values above the integer maximum, below the minimum, infinities, and NaNs. A particular CPU may return a sentinel such as `INT_MIN`, raise a hardware flag, saturate, or trap; none of those observations makes the C++ cast portable for an invalid value.

Unlike integer-to-unsigned conversion, floating-to-unsigned conversion has no general modulo rescue. A negative or excessively large truncated result that cannot be represented is still invalid.

A checked conversion must therefore establish a policy:

```cpp
std::optional<int> to_int(double x)
{
    if (!std::isfinite(x))
        return std::nullopt;

    double t = std::trunc(x);
    // These bounds are correct when the int endpoints convert
    // exactly to double; generic code needs stronger boundary care.
    if (t < static_cast<double>(std::numeric_limits<int>::min()) ||
        t > static_cast<double>(std::numeric_limits<int>::max()))
        return std::nullopt;

    return static_cast<int>(t);
}
```

The subtle point is the comparison's own representation. For a wide destination integer type, converting `max()` to the floating source type can round *up* to a representable floating value outside the integer range. A check that appears to accept only values at or below that converted maximum could accidentally permit undefined behavior. A robust generic conversion should use carefully chosen exclusive boundaries, `nextafter` where appropriate, or a wider/exact comparison strategy, and must test neighboring values around both endpoints.

The reverse integer-to-floating conversion can also lose information. A large `std::uint64_t` may convert to a finite `double` while losing low bits because binary64 has only 53 significant binary digits. A value can therefore be well within the *range* of the floating type yet fail an exactness requirement. C++ bridges to JSON, JavaScript, metrics, and database systems are frequent victims.

When the API requires nearest-integer rounding with ties away from zero, `std::lround` expresses that policy; `std::lrint` uses the active rounding mode; `std::trunc` explicitly represents truncation before a checked conversion. If saturation is required instead, specify every case: for example NaN to zero, positive infinity and excessive positive values to the maximum, and negative counterparts to the minimum. Some domains should reject all nonfinite inputs; others may return `std::optional`, an error code, or `std::expected`. None of these policies is supplied by an unchecked cast.

### 6.1 The historical magic-number conversion

Older processors sometimes made float-to-integer conversion sufficiently costly that programmers developed techniques based on adding carefully selected “magic” constants, exploiting mantissa layout, and reinterpreting bits. Those tricks depend on exact precision and exponent assumptions, rounding mode, byte order, representation access, and a constrained input domain. Several were elegant responses to the hardware of their day.

On modern x86, ARM, and other mainstream platforms, CPUs provide direct conversion instructions and compilers are familiar with them. A hand-coded bit trick may now be slower, less vectorizable, nonportable, and wrong at edges. Replacing an illegal type-punning operation with `std::bit_cast` can repair the object-model problem, but it cannot repair a wrong numeric algorithm.

The two operations must be distinguished:

```cpp
auto representation = std::bit_cast<std::uint32_t>(f);
auto numeric_value  = static_cast<int>(f);
```

The first copies bits between compatible representations; the second converts a number and inherits the conversion's range and rounding rules. They do not have interchangeable meaning.

## 7. Fast inverse square root: what made the trick work

The historical fast inverse square-root algorithm, widely associated with graphics code of the late 1990s, approximates \(1/\sqrt{x}\) by transforming the integer interpretation of an IEEE-like floating bit pattern and then applying Newton-Raphson refinement.

The striking operation is

```cpp
bits = magic - (bits >> 1);
```

For a positive normal IEEE binary floating value, high-order exponent bits and lower-order significand bits make the corresponding unsigned integer approximately affine in \(\log_2(x)\). Subtracting half the integer encoding from a tuned constant approximates a transform proportional to \(-\frac12\log_2(x)\). Reinterpreting that result as a float provides a rough reciprocal-square-root estimate.

The approximation becomes substantially more useful with a Newton step:

```cpp
y = y * (1.5f - 0.5f * x * y * y);
```

The bit manipulation supplies a cheap initial guess; Newton-Raphson supplies the accuracy improvement. With a suitable initial estimate, one iteration was often adequate for historical graphics applications where moderate error was acceptable.

Modern C++ can express the representation step without aliasing undefined behavior:

```cpp
std::uint32_t i = std::bit_cast<std::uint32_t>(x);
i = 0x5f3759dfu - (i >> 1);
float y = std::bit_cast<float>(i);
```

This is still a *specialized IEEE binary32 algorithm*, not a portable general formula for every `float`. It assumes far more than `sizeof(float) == 4`: exact exponent width, bias, fraction placement, special-value encodings, and the intended positive finite normal input range.

For a reusable API, the cases `+0`, `-0`, negative values, positive infinity, NaNs, and subnormals need explicit semantics. The straightforward `1.0f / std::sqrt(x)` follows the platform library's published arithmetic and special-value behavior, while the historical approximation may give arbitrary or unsuitable results at those boundaries unless handled.

### 7.1 Instruction count is not a performance argument

Modern CPUs offer pipelined square root, reciprocal-square-root approximation instructions, SIMD variants, and FMA-assisted refinement. The cost model is completely different from that of late-1990s processors. Fewer source expressions or an ingenious integer constant do not guarantee a faster implementation.

Benchmark the intended target and input distribution, compare ordinary `1/std::sqrt(x)` against a purpose-built approximation or relevant intrinsics, and measure both speed and accuracy. A benchmark must keep results observable so the optimizer cannot discard irrelevant work; it should account for warmup, vectorization, compiler modes, CPU scaling, subnormal behavior, and FP environment. Report absolute, relative, RMS, worst-case, and ULP error as appropriate to the domain, especially at boundaries.

When an approximate reciprocal square root is genuinely desired, an API such as `rsqrt_estimate` or `rsqrt_refined` is clearer than silently replacing a fully specified mathematical operation. Approximation is a contract, not an implementation secret.

## 8. What modern C++ adds to numerical interfaces

The C++20 and C++23 additions are valuable not because they replace floating-point arithmetic, but because they express distinctions that older code often left hidden.

The most important representation additions are `std::bit_cast` and `std::endian`. The former transfers object representation without unrelated-pointer dereferences; the latter reports byte order independently of the meaning of the encoded numerical fields. C++23's `std::byteswap` operates on integer bit representations at file and wire boundaries, not on floating numeric values.

In mathematical code, `std::numbers::pi_v<T>` and related constants replace non-standard macros, while `std::midpoint` and `std::lerp` replace common formulas with useful guarantees. `std::midpoint(a,b)` avoids intermediate overflow for appropriate arithmetic types and is designed to incur at most one inexact operation for floating arguments. `std::lerp(a,b,t)` provides interpolation behavior with precise endpoint and monotonicity properties and stronger finite-result expectations for finite endpoints with interpolation parameters in the supported interval.

C++20's floating-point `<=>` acknowledges unordered NaN results through `std::partial_ordering`. Its `std::floating_point` concept expresses a type-category requirement, not a format requirement: satisfying the concept does not prove IEC-559, radix two, a particular precision, or NaN support. For those properties, use `numeric_limits` and explicit feature requirements.

Floating-point specializations of `std::atomic` gained arithmetic operations in C++20:

```cpp
std::atomic<double> total{0.0};
total.fetch_add(1.25);
total.fetch_sub(0.25);
```

Atomicity does not settle the floating arithmetic environment. In particular, the floating-point environment used by an atomic operation may differ from that of the calling thread; setting a thread's rounding mode does not guarantee the atomic addition uses it. The operation also does not supply a deterministic order of additions across concurrent threads, so bit-for-bit reduction reproducibility remains a separate design problem.

C++20's `std::format` offers modern numeric presentation, including hexadecimal floating styles. That makes diagnostics more convenient but does not choose the application's precision and round-trip policies. For locale-independent conversions with explicit handling of formatting, `std::to_chars` and `std::from_chars` are important interfaces; library availability of floating overloads has historically varied.

C++23's `<stdfloat>` types, their guarded support macros, and literal suffixes such as `0.5f16`, `0.5f32`, `0.5f64`, `0.5f128`, and `0.5bf16` let code request a specific extended type directly. Before these suffixes, initializing a fixed-width destination from `0.1` first produced a `double` literal and then performed a conversion, which could matter for rounding. The new conversion rank/subrank rules and updated common mathematical overload model mean generic code should avoid gratuitously forcing every calculation through `double`, particularly when extended types have wider range or greater precision.

The modernization principle is to use standard facilities for *the operation actually intended*. `bit_cast` for representation transfer, `static_cast` for numeric conversion, `frexp` for numeric decomposition, `isnan` for classification, `midpoint` for a robust midpoint, and explicit typed floating constants for typed mathematical formulas. Historical bit hacks remain valuable to study, but historical machine behavior should not be presented as the portable C++ contract.

## 9. Numerical accuracy and reproducibility require different guarantees

A floating-point type represents a finite discrete set, not a continuum. `std::nextafter` exposes neighboring values, and the spacing grows with magnitude. A small value added to a much larger number can round away:

```cpp
x + tiny == x
```

Repeating `x += tiny` does not secretly retain fractions that were discarded by each individual rounding operation. Changing accumulation order or using compensated summation may be necessary.

Algebraic identities familiar from real arithmetic are not universally valid in floating point:

```text
(a + b) + c  versus  a + (b + c)
(a * b) / b  versus  a
x - x        versus  0
x * 0        versus  0
```

Overflow, underflow, rounding, NaNs, infinities, signed zero, and floating-point flags can all invalidate the equivalence. For example infinity minus itself is NaN on the usual IEC-559 model. The restrictions that strict floating-point compilation imposes on optimizers are therefore semantic restrictions, not irrational hostility toward algebra.

Overflow and underflow are also asymmetric. Overflow may leave the finite range and produce infinity; gradual underflow enters the subnormal region, preserving smaller magnitudes at reduced relative precision before reaching zero. Negative zero can arise through underflow, directed rounding, `std::copysign`, and some library operations without appearing literally as `-0.0` in source.

NaNs can make numerical faults propagate visibly, but they are not structured domain errors. Comparison behavior is unusual, payloads may not survive transformations, fast-math may assume NaNs do not occur, and a NaN value alone rarely explains why an input was invalid. Applications should validate inputs and define error policy deliberately.

Two implementations may both satisfy an accuracy tolerance and nevertheless produce different last bits. To obtain bitwise reproducibility, one may have to constrain compiler flags, library versions, CPU instructions, SIMD vector widths, FMA contraction, reduction order, rounding, and FTZ/DAZ settings. That cost is justified in some deterministic simulations, lockstep networking, scientific regressions, and consensus protocols, but not necessarily in ordinary calculations judged by an error bound. Where cross-platform consensus requires exact arithmetic, fixed-point or integer representations can sometimes provide a simpler model.

### 9.1 The numerical toolbox is deliberately varied

The essential standard-library facilities correspond to the different questions a numerical program asks. `numeric_limits<T>`, `bit_cast`, and `endian` concern representation and format. `fpclassify`, `isfinite`, `isinf`, `isnan`, `isnormal`, and `signbit` concern value classes. `nextafter` and `nexttoward` concern representable neighbors.

For decomposition and scaling, use `frexp`, `ldexp`, `ilogb`, `logb`, `scalbn`, `scalbln`, and `modf`. For stable special forms, consider `fma`, `hypot`, `log1p`, and `expm1`. For rounding, distinguish `floor`, `ceil`, `trunc`, `round`, `rint`, `nearbyint`, `lround`, and `lrint`. For signs and special values, use `copysign` and the relevant `numeric_limits` members.

The floating environment has its own vocabulary: `feclearexcept`, `fetestexcept`, `fegetround`, `fesetround`, `fegetenv`, `fesetenv`, `feholdexcept`, and `feupdateenv`. Text and constants use `to_chars`, `from_chars`, and `numbers::pi_v<T>`. A standard library offers all these separate tools because their contracts are genuinely distinct.

## 10. Diagnostics, compiler modes, and numerical testing

A compiler can identify some suspicious conversions and comparisons but cannot infer a domain's numerical error budget. GCC and Clang commonly use `-Wall -Wextra -Wpedantic -Wconversion -Wfloat-conversion -Wfloat-equal` for relevant diagnostics. The last flag deserves judgment: exact floating equality can be entirely correct. MSVC `/W4 /permissive-` and selectively enabled additional warnings expose conversions such as those reported by C4244 and C4305; `/Wall` can produce substantial incidental system-header noise.

Clang/GCC's `-fsanitize=float-cast-overflow` can detect executed out-of-range floating-to-integer conversions. `-fsanitize=undefined` covers selected additional undefined operations, but support and included checks vary by compiler. Neither is a numerical-accuracy oracle. `<cfenv>` can reveal exception flags when the platform and compilation mode honor them. Boundary tests with `nextafter`, `isfinite`, `isnan`, and `signbit` can exercise cases that instrumentation cannot reason about semantically.

Compiler flags deserve explicit documentation. GCC/Clang `-ffast-math` changes permissible floating semantics, `-ffp-contract=off/on/fast` controls contraction opportunities with toolchain-specific details, and GCC `-frounding-math` addresses some dynamic-rounding assumptions without guaranteeing comprehensive support. MSVC `/fp:strict`, `/fp:precise`, and `/fp:fast` represent different numerical contracts. GNU language modes such as `-std=gnu++20` may permit extensions rejected under ISO `-std=c++20`.

Compiler-specific `__builtin_fma`, `_Float16`, `__float128`, `__FAST_MATH__`, and environment pragmas can be useful when isolated and supported, but they are not substitutes for portable `std::fma`, optional standard extended types, and documented mode requirements. Use standard feature checks and targeted builds rather than inferring support from a broad compiler version.

For more demanding tests, Boost.Multiprecision offers higher or controlled precision, while MPFR can provide explicitly rounded high-precision reference calculations. clang-tidy and other static analyses help locate questionable conversions, and Compiler Explorer helps inspect the actual instructions, FMA contraction, and conversion paths emitted for a particular target. A robust numerical test suite includes normal values, neighboring representable values, cancellation, overflow, underflow, subnormals, signed zero, infinities, NaNs, and precise conversion boundaries.

The ordering of engineering effort matters: first state the error contract and select a suitable algorithm, then test it under the intended numerical modes, and only then optimize measured bottlenecks.

## 11. How the C++ floating-point vocabulary evolved

**C++98/03** established `float`, `double`, `long double`, their numeric-limits properties, fundamental floating conversions, and the foundational `<cmath>` facilities. Precision, radix, extended formats, and important environment details remained dependent on the implementation.

**C++11** added major numerical functions and classification tools including `std::isnan`, `std::isfinite`, `std::fma`, `std::nextafter`, `std::log1p`, and `std::expm1`, alongside `std::hexfloat` presentation. Code could increasingly express stable operations and exceptional-value handling directly.

**C++14** added digit separators, improving readability of precise numeric constants but not changing the fundamental approximation model or the need for error analysis.

**C++17** introduced locale-independent `std::to_chars` and `std::from_chars` conversion interfaces; floating overload availability depended on standard-library implementation maturity. The three-argument `std::hypot` overload also broadened stable magnitude calculations.

**C++20** introduced `std::bit_cast`, `std::endian`, `std::numbers`, `std::midpoint`, `std::lerp`, floating `<=>` with partial ordering, the `std::floating_point` concept, floating atomic arithmetic operations, and `std::format`. These facilities separate representation, ordering, numerical intent, synchronization, and presentation more clearly.

**C++23** added optional fixed-width extended floating types in `<stdfloat>`, new literal suffixes, and the conversion rank/subrank model. `std::byteswap` also simplifies integer representation handling at binary boundaries. Actual availability requires implementation and library support checks.

**C++26** continues to evolve the standard numerical library, but this chapter's fundamental guarantees do not depend on a speculative format or unverified proposal. When documenting a newer facility, specify its adopted standard revision and test the library feature rather than assuming support from the language mode alone.

This trajectory is not a replacement of floating-point arithmetic by “safer math.” It is a gradual improvement in the vocabulary for describing exactly which numerical operation and which representation-level action is intended.

## 12. Translating expectations from Java, Python, and C#

Experienced developers from other language ecosystems often know IEEE binary64 well, but the type name and failure semantics do not transfer mechanically. Java and C# define familiar double-precision floating representations, while Python's built-in `float` is ordinarily implemented using the platform C double. C++ leaves the exact representation of its three standard floating types to the implementation.

One especially consequential difference is floating-to-integer conversion. Java defines specific results for out-of-range narrowing from floating to integer, Python's `int(float)` can raise exceptions for nonfinite values, and C# has checked/unchecked conversion rules. In C++, a built-in floating-to-integer conversion whose truncated result does not fit invokes undefined behavior. An apparently harmless cast can therefore create an optimizer-level correctness problem, not merely a surprising numeric result.

Similarly, a tolerance derived from one application's measurement units does not automatically generalize, and `numeric_limits<T>::epsilon()` is not a universal comparison margin. Cross-platform reproducibility also cannot be inferred from matching source expressions when the compiler and platform control contraction, rounding, extended intermediates, or math-library choices.

The idiomatic approach is to state each assumption: check format requirements with `numeric_limits`, design comparisons around an error budget, validate finite ranges before integer conversion, use explicit round-trip formatting policies, and pin the numerical environment only when determinism requires it.

## 13. A durable mental model

A floating-point result depends on the set of representable values, the chosen rounding operation, the evaluation order, and the permitted transformations of the compiler and library. Correctness consequently has several levels: a representation operation may be legal under the C++ object model; one arithmetic operation may be correctly rounded; an algorithm may meet its error bound; and a distributed computation may or may not be bitwise reproducible. None of these properties implies all the others.

Before reviewing a numerical interface, ask which floating format is assumed, how comparison tolerances were chosen, what the code does with NaNs and signed zeros, whether conversion endpoints are checked without rounding holes, and which compiler and environment settings are part of the result contract. If the code serializes values, ask whether it promises numerical equivalence or exact bitwise round-trip. If it uses a historical performance trick, ask whether the real target still benefits and whether the approximation is represented honestly in the API.

> Floating-point correctness belongs to the algorithm and its error budget, not to the spelling of `double`.

## Further reading

The relevant C++ standard revision is authoritative for language guarantees. Useful reference material includes [floating-point and arithmetic types](https://en.cppreference.com/w/cpp/language/types), [`std::numeric_limits`](https://en.cppreference.com/w/cpp/types/numeric_limits), [mathematical functions](https://en.cppreference.com/w/cpp/numeric/math), [`std::bit_cast`](https://en.cppreference.com/w/cpp/numeric/bit_cast), [`<cfenv>`](https://en.cppreference.com/w/cpp/header/cfenv), [`<stdfloat>`](https://en.cppreference.com/w/cpp/header/stdfloat), and [character conversions](https://en.cppreference.com/w/cpp/utility/from_chars).

David Goldberg's [*What Every Computer Scientist Should Know About Floating-Point Arithmetic*](https://docs.oracle.com/cd/E19957-01/806-3568/ncg_goldberg.html) remains an essential deeper discussion of numerical representation and rounding. [WG21 P1467](https://wg21.link/P1467) documents the extended floating-point type work that led to the modern optional fixed-width vocabulary.
