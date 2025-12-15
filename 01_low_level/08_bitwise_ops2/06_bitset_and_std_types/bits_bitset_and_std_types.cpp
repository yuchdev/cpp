/*
bits_bitset_and_std_types.cpp
Deep coverage: std::bitset, std::byte, and the infamous std::vector<bool>.
Also shows bridging between raw integers and bitset, and common pitfalls.

Build (C++17):
  g++ -std=c++17 -O2 -Wall -Wextra -pedantic bits_bitset_and_std_types.cpp -o bits_std
*/

// Common helper: print bits of unsigned integers in binary (no external deps).
// Works in C++17+. Uses std::bitset with width = sizeof(T)*CHAR_BIT.
// For signed values, cast to the corresponding unsigned type first.

#include <bitset>
#include <climits>
#include <cstdint>
#include <cstddef>
#include <limits>
#include <string>
#include <vector>
#include <iostream>
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

static void bitset_basics()
{
    std::cout << "== std::bitset basics ==\n";
    std::bitset<16> b(0x00F3u);
    std::cout << "b       = " << b << "\n";
    b.flip(0);
    b.set(15);
    std::cout << "after   = " << b << "\n";
    std::cout << "count   = " << b.count() << "\n";
    std::cout << "any/all = " << b.any() << "/" << b.all() << "\n";

    // Conversions:
    unsigned long v = b.to_ulong(); // throws std::overflow_error if doesn't fit (for large bitsets)
    std::cout << "to_ulong=0x" << std::hex << v << std::dec << "\n";
}

static void bitset_as_mask_tool()
{
    std::cout << "\n== bitset as mask tool (fixed width, no decay) ==\n";
    std::bitset<8> flags;
    flags.set(2);
    flags.set(5);
    std::cout << "flags=" << flags << "\n";
    if (flags.test(5)) std::cout << "bit 5 is set\n";
}

static void std_byte_basics()
{
    std::cout << "\n== std::byte (C++17) ==\n";
    std::byte b{0x2Au};
    // std::byte is not an integer; it's a distinct type for raw byte operations.
    // You need explicit conversions:
    auto u = std::to_integer<unsigned>(b);
    std::cout << "byte as unsigned=" << u << "\n";

    // Bitwise ops exist for std::byte:
    std::byte c = b << 1;
    std::cout << "b<<1 as unsigned=" << std::to_integer<unsigned>(c) << "\n";
}

static void vector_bool_trap()
{
    std::cout << "\n== std::vector<bool> trap ==\n";
    std::vector<bool> v(8);
    v[0] = true;

    // v[0] is NOT bool&. It's a proxy object (bit reference).
    auto ref = v[0];
    bool x = ref; // converts to bool
    std::cout << "v[0]=" << x << "\n";

    // This surprises generic code expecting T& references.
    // If you need a real container of bools, use std::vector<unsigned char> or std::vector<std::uint8_t>.
}

int main()
{
    bitset_basics();
    bitset_as_mask_tool();
    std_byte_basics();
    vector_bool_trap();

    std::cout << "\n(bits_bitset_and_std_types.cpp) OK\n";
    return 0;
}
