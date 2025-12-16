/*
bits_shifts_and_ub.cpp
Deep coverage: shift operator rules, UB/implementation-defined behavior, safe patterns,
and why you should shift unsigned.

Build (C++17):
  g++ -std=c++17 -O2 -Wall -Wextra -pedantic bits_shifts_and_ub.cpp -o bits_shift
*/

// Common helper: print bits of unsigned integers in binary (no external deps).
// Works in C++17+. Uses std::bitset with width = sizeof(T)*CHAR_BIT.
// For signed values, cast to the corresponding unsigned type first.

#include <bitset>
#include <climits>
#include <cstdint>
#include <iostream>
#include <limits>
#include <type_traits>
#include <cstddef>
#include <stdexcept>

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

template <class U>
U safe_shl(U x, unsigned k)
{
    static_assert(std::is_unsigned<U>::value, "safe_shl expects unsigned");
    constexpr unsigned W = sizeof(U) * CHAR_BIT;
    if (k >= W) return 0; // defined policy: return 0 for overshift
    return static_cast<U>(x << k);
}

template <class U>
U safe_shr(U x, unsigned k)
{
    static_assert(std::is_unsigned<U>::value, "safe_shr expects unsigned");
    constexpr unsigned W = sizeof(U) * CHAR_BIT;
    if (k >= W) return 0;
    return static_cast<U>(x >> k);
}

static void demonstrate_shift_rules()
{
    std::cout << "== Shift rules overview ==\n";
    std::uint32_t u = 1u;
    std::cout << "u=1 bits=" << bits_u(u) << "\n";
    u = u << 3;
    std::cout << "u<<3 bits=" << bits_u(u) << " val=" << u << "\n";

    // UB examples (do NOT run; left as comments):
    // 1) shifting by >= width is UB for built-in shifts
    // u = u << 32; // UB for uint32_t

    // 2) left shift on signed that overflows is UB
    // int s = 1;
    // s = s << 31; // UB (overflow / sign bit)

    // 3) right shift of negative signed is implementation-defined
    // int neg = -8;
    // std::cout << (neg >> 1) << "\n"; // arithmetic shift on most compilers, not guaranteed pre-C++20
}

static void demonstrate_signed_right_shift_is_not_portable()
{
    std::cout << "\n== Signed right shift: implementation-defined for negatives ==\n";
    int neg = -8;
    std::cout << "neg=" << neg << " bits(as unsigned)=" << bits_u(static_cast<std::uint32_t>(neg)) << "\n";
    int r = (neg >> 1);
    std::cout << "neg>>1=" << r << " bits(as unsigned)=" << bits_u(static_cast<std::uint32_t>(r)) << "\n";
    std::cout << "(Most targets do arithmetic shift, but do not rely on it for portability.)\n";
}

static void safe_shift_helpers_demo()
{
    std::cout << "\n== Safe shift helpers (policy: overshift -> 0) ==\n";
    std::uint8_t x = 0b10110000u;
    std::cout << "x bits=" << bits_u(x) << "\n";
    std::cout << "x<<2 bits=" << bits_u(safe_shl<std::uint8_t>(x, 2)) << "\n";
    std::cout << "x>>3 bits=" << bits_u(safe_shr<std::uint8_t>(x, 3)) << "\n";
    std::cout << "x>>99 bits=" << bits_u(safe_shr<std::uint8_t>(x, 99)) << " (policy)\n";
}

static void shift_and_mask_common_patterns()
{
    std::cout << "\n== Shift+mask patterns ==\n";
    std::uint32_t word = 0xDEADBEEFu;
    std::cout << "word=0xDEADBEEF bits=" << bits_u(word) << "\n";

    // Extract middle byte:
    std::uint32_t mid = (word >> 8) & 0xFFu;
    std::cout << "byte1 (bits 8..15) = 0x" << std::hex << mid << std::dec << "\n";

    // Compose:
    std::uint32_t a = 0x12u, b = 0x34u, c = 0x56u, d = 0x78u;
    std::uint32_t packed = (a << 24) | (b << 16) | (c << 8) | d;
    std::cout << "packed=0x" << std::hex << packed << std::dec << "\n";
}

int main()
{
    demonstrate_shift_rules();
    demonstrate_signed_right_shift_is_not_portable();
    safe_shift_helpers_demo();
    shift_and_mask_common_patterns();

    std::cout << "\n(bits_shifts_and_ub.cpp) OK\n";
    return 0;
}
