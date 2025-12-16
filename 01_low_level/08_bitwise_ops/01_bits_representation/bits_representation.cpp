/*
bits_integer_representation.cpp
Deep coverage: integer representation, signed vs unsigned, two's complement assumptions,
value bits/padding bits, conversions, and "bitwise belongs to unsigned".

Build (C++17):
  g++ -std=c++17 -O2 -Wall -Wextra -pedantic bits_integer_representation.cpp -o bits_repr

Notes:
- C++20 strengthened guarantees around two's complement for signed integers in practice on major platforms,
  but for portable *bit-level reasoning* across standards, prefer unsigned and explicit casts.
*/

// Common helper: print bits of unsigned integers in binary (no external deps).
// Works in C++17+. Uses std::bitset with width = sizeof(T)*CHAR_BIT.
// For signed values, cast to the corresponding unsigned type first.

#include <bitset>
#include <climits>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <limits>
#include <type_traits>

template <class T>
using make_unsigned_t = typename std::make_unsigned<T>::type;

template <class T>
constexpr bool is_integral_not_bool_v =
    std::is_integral<T>::value && !std::is_same<T, bool>::value;

template <class U>
std::string bits_u(U value)
{
    static_assert(is_integral_not_bool_v<U>, "bits_u requires an integral (non-bool) type");
    using UU = make_unsigned_t<U>;
    constexpr std::size_t W = sizeof(UU) * CHAR_BIT;
    return std::bitset<W>(static_cast<UU>(value)).to_string();
}

template <class S>
std::string bits_s(S value)
{
    static_assert(is_integral_not_bool_v<S>, "bits_s requires an integral (non-bool) type");
    using U = make_unsigned_t<S>;
    return bits_u(static_cast<U>(value));
}

template <class T>
void show_limits(const char* name)
{
    static_assert(std::is_integral<T>::value, "integral only");
    std::cout << name
              << "  sizeof=" << sizeof(T)
              << "  digits=" << std::numeric_limits<T>::digits
              << "  is_signed=" << std::numeric_limits<T>::is_signed
              << "  min=" << +std::numeric_limits<T>::min()
              << "  max=" << +std::numeric_limits<T>::max()
              << "\n";
}

static void signed_vs_unsigned_model()
{
    std::cout << "== Integer model / limits ==\n";
    show_limits<std::uint8_t>("uint8_t ");
    show_limits<std::int8_t>("int8_t  ");
    show_limits<std::uint32_t>("uint32_t");
    show_limits<std::int32_t>("int32_t ");

    std::cout << "\n== Why bitwise belongs to unsigned ==\n";
    std::int32_t s = -5;
    std::uint32_t u = static_cast<std::uint32_t>(s); // well-defined: modulo 2^32
    std::cout << "s = " << s << "  bits(s as unsigned) = " << bits_u(u) << "\n";

    // In general: converting signed -> unsigned is well-defined (mod 2^N).
    // Converting unsigned -> signed is implementation-defined if out of range.
}

static void twos_complement_identities_carefully()
{
    std::cout << "\n== Two's complement identities (with caveats) ==\n";

    // For unsigned types, two's complement math is guaranteed modulo 2^N.
    std::uint32_t x = 123456789u;
    std::uint32_t neg = ~x + 1u; // == 0 - x modulo 2^32
    std::cout << "x        = " << x   << "  bits=" << bits_u(x)   << "\n";
    std::cout << "~x + 1   = " << neg << "  bits=" << bits_u(neg) << "\n";
    std::cout << "0u - x   = " << (0u - x) << "  (same as ~x+1 modulo)\n";

    // For signed types, ~x+1 == -x is true on two's complement machines,
    // but left to implementation details historically. Prefer converting through unsigned
    // if you need bit-precise transformations.
}

static void value_bits_padding_bits()
{
    std::cout << "\n== Value bits vs padding bits ==\n";
    // digits == number of value bits. For unsigned, usually digits == width.
    // For signed, digits excludes sign bit.
    std::cout << "uint32_t digits=" << std::numeric_limits<std::uint32_t>::digits
              << " width=" << (sizeof(std::uint32_t)*CHAR_BIT) << "\n";
    std::cout << "int32_t  digits=" << std::numeric_limits<std::int32_t>::digits
              << " width=" << (sizeof(std::int32_t)*CHAR_BIT) << " (sign bit excluded)\n";
}

static void safe_bit_cast_pattern_for_printing()
{
    std::cout << "\n== Safe bit-cast pattern for signed values ==\n";
    std::int32_t s = -1;
    auto su = static_cast<std::uint32_t>(s);
    std::cout << "s=-1 as uint32_t: " << su << " bits=" << bits_u(su) << "\n";
}

int main()
{
    signed_vs_unsigned_model();
    twos_complement_identities_carefully();
    value_bits_padding_bits();
    safe_bit_cast_pattern_for_printing();

    std::cout << "\n(bits_integer_representation.cpp) OK\n";
    return 0;
}
