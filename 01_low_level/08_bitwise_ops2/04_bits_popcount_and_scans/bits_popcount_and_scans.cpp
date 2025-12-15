/*
bits_popcount_and_scans.cpp
Deep coverage: Hamming weight (popcount), bit scans, low/high bit extraction,
Kernighan, SWAR, lookup tables, De Bruijn, and C++20 <bit> equivalents (guarded).

Build (C++17):
  g++ -std=c++17 -O2 -Wall -Wextra -pedantic bits_popcount_and_scans.cpp -o bits_pop

Build (C++20) to enable <bit>:
  g++ -std=c++20 -O2 -Wall -Wextra -pedantic bits_popcount_and_scans.cpp -o bits_pop20
*/

// Common helper: print bits of unsigned integers in binary (no external deps).
// Works in C++17+. Uses std::bitset with width = sizeof(T)*CHAR_BIT.
// For signed values, cast to the corresponding unsigned type first.

#include <bitset>
#include <climits>
#include <cstdint>
#include <iostream>
#include <limits>
#include <array>
#include <cstdint>
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

// --- SWAR popcount for 32-bit (classic)
static std::uint32_t popcount_swar32(std::uint32_t x)
{
    x = x - ((x >> 1) & 0x55555555u);
    x = (x & 0x33333333u) + ((x >> 2) & 0x33333333u);
    x = (x + (x >> 4)) & 0x0F0F0F0Fu;
    x = x + (x >> 8);
    x = x + (x >> 16);
    return x & 0x3Fu;
}

// --- Kernighan (good when few bits set)
static std::uint32_t popcount_kernighan(std::uint32_t x)
{
    std::uint32_t c = 0;
    while (x) { x &= (x - 1); ++c; }
    return c;
}

// --- Lookup table for bytes
static std::array<std::uint8_t, 256> make_byte_popcount()
{
    std::array<std::uint8_t, 256> t{};
    for (int i = 0; i < 256; ++i)
    {
        std::uint8_t v = static_cast<std::uint8_t>(i);
        std::uint8_t c = 0;
        for (int b = 0; b < 8; ++b) c += (v >> b) & 1u;
        t[i] = c;
    }
    return t;
}

static std::uint32_t popcount_table32(std::uint32_t x)
{
    static const auto T = make_byte_popcount();
    return T[(x >> 0) & 0xFFu] +
           T[(x >> 8) & 0xFFu] +
           T[(x >> 16) & 0xFFu] +
           T[(x >> 24) & 0xFFu];
}

// --- Low/high bit helpers
static bool is_power_of_two(std::uint32_t x)
{
    return x != 0 && ((x & (x - 1)) == 0);
}

static std::uint32_t isolate_lsb(std::uint32_t x)
{
    // x & -x works in two's complement; for unsigned it's well-defined modulo arithmetic:
    return x & (0u - x);
}

// Index of least significant set bit (0..31), undefined for x==0 (so we guard).
static int ctz_debruijn32(std::uint32_t x)
{
    static const int index[32] = {
        0, 1, 28, 2, 29, 14, 24, 3,
        30, 22, 20, 15, 25, 17, 4, 8,
        31, 27, 13, 23, 21, 19, 16, 7,
        26, 12, 18, 6, 11, 5, 10, 9
    };
    std::uint32_t lsb = isolate_lsb(x);
    return index[(lsb * 0x077CB531u) >> 27];
}

// Count leading zeros for 32-bit using a simple loop (portable baseline).
static int clz_loop32(std::uint32_t x)
{
    if (x == 0) return 32;
    int n = 0;
    for (int i = 31; i >= 0; --i)
    {
        if ((x >> i) & 1u) break;
        ++n;
    }
    return n;
}

static void demo_popcount()
{
    std::cout << "== popcount demos ==\n";
    std::uint32_t x = 0b10110100100101010101000111100011u;
    std::cout << "x bits=" << bits_u(x) << "\n";
    std::cout << "swar32      = " << popcount_swar32(x) << "\n";
    std::cout << "kernighan   = " << popcount_kernighan(x) << "\n";
    std::cout << "table32     = " << popcount_table32(x) << "\n";
}

static void demo_scans()
{
    std::cout << "\n== bit scan demos ==\n";
    std::uint32_t x = 0x00104000u; // two bits
    std::cout << "x=0x00104000 bits=" << bits_u(x) << "\n";
    std::cout << "is_power_of_two? " << is_power_of_two(x) << "\n";
    std::cout << "isolate_lsb bits=" << bits_u(isolate_lsb(x)) << "\n";
    std::cout << "ctz_debruijn (x!=0) = " << ctz_debruijn32(x) << "\n";
    std::cout << "clz_loop32          = " << clz_loop32(x) << "\n";
}

#if __cplusplus >= 202002L
  #if __has_include(<bit>)
    #include <bit>
    static void demo_cpp20_bitops()
    {
        std::cout << "\n== C++20 <bit> demos ==\n";
        std::uint32_t x = 0x00104000u;
        std::cout << "popcount     = " << std::popcount(x) << "\n";
        std::cout << "countr_zero  = " << std::countr_zero(x) << "\n";
        std::cout << "countl_zero  = " << std::countl_zero(x) << "\n";
        std::cout << "rotl(x,5)    = 0x" << std::hex << std::rotl(x, 5) << std::dec << "\n";
        std::cout << "rotr(x,5)    = 0x" << std::hex << std::rotr(x, 5) << std::dec << "\n";
    }
  #endif
#endif

int main()
{
    demo_popcount();
    demo_scans();

#if __cplusplus >= 202002L
  #if __has_include(<bit>)
    demo_cpp20_bitops();
  #else
    std::cout << "\n(C++20 <bit> not available in this stdlib)\n";
  #endif
#else
    std::cout << "\n(Compile with -std=c++20 to enable <bit> demos)\n";
#endif

    std::cout << "\n(bits_popcount_and_scans.cpp) OK\n";
    return 0;
}
