// ReSharper disable All
#include <utilities/defines.h>
SUPPRESS_PRAGMA_WARNINGS()

#include <iostream>
#include <cstdint>

// Fundamental C++ types
// https://en.cppreference.com/w/cpp/language/types

#include <cstdint>
#include <cstdio>
#include <limits>
#include <type_traits>

/*
Numeric conversions (NOT promotions) — what can go wrong:

Key idea:
  - Promotions are mostly "safe widening" (e.g., uint8_t -> int).
  - Conversions are the dangerous ones: they may change the value, lose precision,
    become implementation-defined, or (rarely) be undefined behavior.

What the standard generally guarantees (high-level):
  - Integer -> integer (different type): value may change (truncation / modulo / impl-defined)
  - Floating -> integer: truncation toward zero; if out of range: undefined behavior
  - Integer -> floating: may lose precision; if magnitude too large: result may be +/-inf (IEEE),
    but exact behavior depends on implementation and FP environment
  - Signedness flips: negative -> unsigned is well-defined (modulo), unsigned -> signed may be
    implementation-defined if not representable
  - List-initialization with {} rejects narrowing at compile time (where it can detect it)

Practical rule:
  - Treat "implicit numeric conversions" as bugs unless you have a reason.
  - Prefer explicit checked conversions (range checks) or brace-init to force diagnostics.

NOTE:
  - uint8_t is typically an alias of unsigned char. This matters for printing and overloads.
*/

static uint8_t safe_avg(uint8_t a, uint8_t b)
{
    // Avoid overflow that would happen with (a + b) / 2 in uint8_t arithmetic.
    // Here we still rely on integer promotions for the division, but the *returned* type is uint8_t.
    return static_cast<uint8_t>(a / 2 + b / 2);
}

// Danger: silently truncates (well-defined modulo for unsigned target)
static uint8_t truncate_u8(uint32_t u)
{
    // Implicit conversion: uint32_t -> uint8_t.
    // For unsigned -> unsigned narrowing: value becomes u mod 256.
    // Many compilers do not warn unless you enable -Wconversion / -Wimplicit-int-conversion.
    return static_cast<uint8_t>(u);
}

static int32_t truncate_i32(uint64_t u)
{
    // Unsigned -> signed:
    // If u cannot be represented in int32_t, the result is implementation-defined
    // (or might be defined via two's complement behavior on your platform, but don't rely on it).
    return static_cast<int32_t>(u);
}

static uint32_t neg_to_unsigned(int32_t s)
{
    // Signed -> unsigned is well-defined: result is s modulo 2^32.
    return static_cast<uint32_t>(s);
}

static int32_t unsigned_to_signed(uint32_t u)
{
    // If u > INT32_MAX, this conversion is implementation-defined.
    return static_cast<int32_t>(u);
}

static int fp_to_int(double x)
{
    // Floating -> integer truncates toward zero.
    // If x is NaN or out of range for int, behavior is undefined.
    return static_cast<int>(x);
}

static double int_to_fp(uint64_t x)
{
    // Integer -> floating can lose precision.
    // double has 53 bits of precision; integers above 2^53 cannot be represented exactly.
    return static_cast<double>(x);
}

enum class Color8 : uint8_t { Red = 1, Green = 2, Blue = 3 };

static void print_u8(const char* label, uint8_t v)
{
    // Printing uint8_t directly is often printed as a char; cast to unsigned for numeric.
    std::printf("%s = %u (0x%02X)\n", label, static_cast<unsigned>(v),
                static_cast<unsigned>(v));
}

static void numeric_conversions()
{
    std::puts("== Integer narrowing & truncation (unsigned -> unsigned) ==");

    {
        uint32_t trunc_me = 0x0000FFFFu; // 65535
        uint8_t t = truncate_u8(trunc_me);
        // well-defined: 65535 mod 256 = 255
        print_u8("truncate_u8(0x0000FFFF)", t);
    }
    {
        uint32_t trunc_me = 0x12345678u;
        uint8_t t = truncate_u8(trunc_me);
        // well-defined modulo behavior
        print_u8("truncate_u8(0x12345678)", t);
    }

    std::puts("\n== Signedness changes ==");

    {
        int32_t s = -1;
        uint32_t u = neg_to_unsigned(s);
        // well-defined: -1 becomes UINT32_MAX
        std::printf("neg_to_unsigned(-1) = %u (0x%08X)\n", u, u);
    }
    {
        uint32_t u = 0x80000000u; // 2147483648, one above INT32_MAX
        int32_t s = unsigned_to_signed(u);
        // implementation-defined: could become -2147483648 on two's complement platforms
        std::printf("unsigned_to_signed(0x80000000) = %d (0x%08X) [impl-defined]\n",
                    s, static_cast<uint32_t>(s));
    }

    std::puts("\n== Unsigned -> signed truncation via larger type ==");

    {
        uint64_t huge = 0x1'0000'0000ULL; // 2^32
        int32_t s = truncate_i32(huge);
        // implementation-defined: not representable in int32_t
        std::printf("truncate_i32(2^32) = %d [impl-defined]\n", s);
    }

    std::puts("\n== Integer -> floating precision loss ==");

    {
        uint64_t exact = (1ULL << 53);       // exactly representable in double
        uint64_t inexact = (1ULL << 53) + 1; // NOT exactly representable

        double d1 = int_to_fp(exact);
        double d2 = int_to_fp(inexact);

        // d1 prints as 9007199254740992
        // d2 becomes the same value when rounded to nearest representable double
        std::printf("double(2^53)     = %.0f\n", d1);
        std::printf("double(2^53 + 1) = %.0f  [lost +1]\n", d2);
    }

    std::puts("\n== Floating -> integer truncation (and UB on out-of-range) ==");

    {
        double a = 1.9;
        double b = -1.9;
        std::printf("fp_to_int( 1.9) = %d (trunc toward 0)\n", fp_to_int(a));
        std::printf("fp_to_int(-1.9) = %d (trunc toward 0)\n", fp_to_int(b));
    }
    {
        // WARNING: The following are examples of *undefined behavior* if executed:
        // double huge = 1e300;
        // int x = fp_to_int(huge); // UB: out of range
        // double nan = std::numeric_limits<double>::quiet_NaN();
        // int y = fp_to_int(nan); // UB
        std::puts("Skipping UB examples: fp_to_int(NaN/out-of-range) is undefined.");
    }

    std::puts("\n== Brace-init rejects narrowing (compile-time pitfall prevention) ==");

    {
        // Uncomment to see compile-time narrowing diagnostics:
        // int i1 = 3.14;    // allowed (value-changing), but probably a bug
        // int i2{3.14};     // ERROR: narrowing (good!)
        // uint8_t u1 = 300; // allowed: truncation to 44 (if uint8_t is unsigned char)
        // uint8_t u2{300};  // ERROR: narrowing (good!)
        std::puts("List-init demo left commented: int{3.14} and uint8_t{300} are ill-formed.");
    }

    std::puts("\n== bool conversions (value collapse) ==");

    {
        int x = 123;
        int y = 0;
        bool bx = static_cast<bool>(x); // true
        bool by = static_cast<bool>(y); // false
        std::printf("bool(123)=%d, bool(0)=%d (all non-zero collapse to true)\n", bx, by);
    }

    std::puts("\n== Enum underlying type pitfalls ==");

    {
        // enum class with explicit uint8_t underlying type can truncate easily
        Color8 c = Color8::Red;
        uint8_t raw = static_cast<uint8_t>(c);
        print_u8("uint8_t(Color8::Red)", raw);

        // Dangerous: converting arbitrary integer to enum is allowed but can create "invalid" enum values.
        // (Still a valid object representation; but semantically nonsense.)
        uint8_t nonsense = 250;
        Color8 bad = static_cast<Color8>(nonsense);
        (void)bad;
        print_u8("nonsense enum value", nonsense);
        std::puts("Note: enums can hold values not listed in the enumerators (semantic pitfall).");
    }

    std::puts("\n== Averaging pitfall: overflow vs truncation ==");

    {
        // Naive average overflows if done in uint8_t after narrowing back.
        auto naive = [](uint8_t a, uint8_t b) -> uint8_t {
            // (a + b) promotes to int, but if you later cast back, you still might overflow the intended domain.
            return static_cast<uint8_t>((a + b) / 2);
        };

        uint8_t a = 250, b = 250;
        print_u8("naive_avg(250,250)", naive(a, b)); // ok here, but shows the pattern
        print_u8("safe_avg(250,250)", safe_avg(a, b));

        // The "classic" overflow risk is when doing (a + b) in a narrow type.
        // With uint8_t, you usually get promotion to int, so the *addition* won't overflow,
        // but the *conversion back* still can truncate elsewhere in real code.
    }

    std::puts("\n== Printing/IO pitfall with uint8_t ==");

    {
        uint8_t u = 65;
        // If uint8_t is unsigned char, printing it as %c shows 'A'.
        std::printf("uint8_t 65 as char: '%c'\n", static_cast<unsigned char>(u));
        std::printf("uint8_t 65 as int : %u\n", static_cast<unsigned>(u));
    }

    std::puts("\n== Range-checking pattern (recommended) ==");

    {
        auto checked_u32_to_u8 = [](uint32_t v) -> uint8_t {
            if (v > std::numeric_limits<uint8_t>::max()) {
                std::puts("checked_u32_to_u8: out of range!");
                return 0;
            }
            return static_cast<uint8_t>(v);
        };

        print_u8("checked_u32_to_u8(200)", checked_u32_to_u8(200));
        print_u8("checked_u32_to_u8(300)", checked_u32_to_u8(300));
    }
}

int main()
{
    numeric_conversions();
    return 0;
}
