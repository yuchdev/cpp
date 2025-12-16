/*
bits_radix_and_data_layout.cpp
Deep coverage: stable radix sort for 32-bit unsigned integers using byte-wise passes,
counting histograms, and why this is "bitwise-friendly" (no comparisons).

Fixes the prior radix_sort.cpp: avoids pow/div, uses shifts & masks, and is stable.

Build (C++17):
  g++ -std=c++17 -O2 -Wall -Wextra -pedantic bits_radix_and_data_layout.cpp -o bits_radix
*/

// Common helper: print bits of unsigned integers in binary (no external deps).
// Works in C++17+. Uses std::bitset with width = sizeof(T)*CHAR_BIT.
// For signed values, cast to the corresponding unsigned type first.

#include <bitset>
#include <climits>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <vector>
#include <algorithm>
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

static void radix_sort_u32(std::vector<std::uint32_t>& v)
{
    if (v.empty()) return;

    std::vector<std::uint32_t> tmp(v.size());

    // 4 passes of 8 bits each (least significant byte first): stable LSD radix sort
    for (unsigned shift = 0; shift < 32; shift += 8)
    {
        std::size_t count[256] = {0};

        for (std::uint32_t x : v)
            ++count[(x >> shift) & 0xFFu];

        std::size_t pos[256];
        pos[0] = 0;
        for (int i = 1; i < 256; ++i)
            pos[i] = pos[i - 1] + count[i - 1];

        for (std::uint32_t x : v)
            tmp[pos[(x >> shift) & 0xFFu]++] = x;

        v.swap(tmp);
    }
}

static bool is_sorted_u32(const std::vector<std::uint32_t>& v)
{
    return std::is_sorted(v.begin(), v.end());
}

static void demo_radix_sort()
{
    std::cout << "== radix sort demo (u32) ==\n";
    std::mt19937 rng(12345);
    std::uniform_int_distribution<std::uint32_t> dist(0, 0xFFFF'FFFFu);

    std::vector<std::uint32_t> v;
    v.reserve(50);
    for (int i = 0; i < 50; ++i) v.push_back(dist(rng));

    std::vector<std::uint32_t> ref = v;

    radix_sort_u32(v);
    std::sort(ref.begin(), ref.end());

    std::cout << "sorted? " << is_sorted_u32(v) << "\n";
    std::cout << "matches std::sort? " << (v == ref) << "\n";

    // Print first few:
    for (int i = 0; i < 5; ++i)
        std::cout << "v[" << i << "]=" << v[i] << " bits=" << bits_u(v[i]) << "\n";
}

int main()
{
    demo_radix_sort();
    std::cout << "\n(bits_radix_and_data_layout.cpp) OK\n";
    return 0;
}
