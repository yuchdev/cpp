/*
bits_arithmetic_tricks.cpp
Deep coverage: "Hacker's Delight"-style arithmetic using bitwise ops,
but written with C++ UB rules in mind. Includes addition, subtraction, abs,
branchless min/max, and overflow detection for unsigned.

Build (C++17):
  g++ -std=c++17 -O2 -Wall -Wextra -pedantic bits_arithmetic_tricks.cpp -o bits_arith
*/

// Common helper: print bits of unsigned integers in binary (no external deps).
// Works in C++17+. Uses std::bitset with width = sizeof(T)*CHAR_BIT.
// For signed values, cast to the corresponding unsigned type first.

#include <bitset>
#include <climits>
#include <cstdint>
#include <iostream>
#include <limits>
#include <climits>
#include <cstdint>
#include <tuple>
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


// Addition without '+': works for unsigned and signed in two's complement practice,
// but we implement the core on unsigned to make wrap semantics defined.
template <class U>
U add_u(U a, U b)
{
    static_assert(std::is_unsigned<U>::value, "add_u expects unsigned");
    while (b != 0)
    {
        U carry = a & b;
        a = a ^ b;
        b = static_cast<U>(carry << 1);
    }
    return a;
}

template <class U>
U sub_u(U a, U b)
{
    // a - b == a + (~b + 1) in modulo arithmetic
    static_assert(std::is_unsigned<U>::value, "sub_u expects unsigned");
    return add_u(a, add_u(static_cast<U>(~b), static_cast<U>(1)));
}

template <class U>
std::pair<U, bool> add_u_with_overflow(U a, U b)
{
    // For unsigned, overflow is defined wrap; detect carry-out:
    U s = a + b;
    bool overflow = (s < a); // if wrap occurred
    return {s, overflow};
}

// Branchless absolute value for signed two's complement is common,
// but right-shift of negative is implementation-defined pre-C++20.
// We'll provide two versions:
// - portable: uses conditional (may compile to branchless anyway)
// - bit-trick: enabled when you accept arithmetic shift behavior

int abs_portable(int x)
{
    return (x < 0) ? -x : x;
}

int min_portable(int a, int b) { return (a < b) ? a : b; }
int max_portable(int a, int b) { return (a < b) ? b : a; }

// Safer "branchless" for unsigned:
template <class U>
U min_u_branchless(U a, U b)
{
    static_assert(std::is_unsigned<U>::value, "min_u_branchless expects unsigned");
    // mask = all-ones if a < b else 0
    U mask = static_cast<U>(-static_cast<int>(a < b));
    return (a & mask) | (b & ~mask);
}

template <class U>
U max_u_branchless(U a, U b)
{
    static_assert(std::is_unsigned<U>::value, "max_u_branchless expects unsigned");
    U mask = static_cast<U>(-static_cast<int>(a < b));
    return (b & mask) | (a & ~mask);
}

static void demo_add_sub()
{
    std::cout << "== add/sub in modulo arithmetic (unsigned) ==\n";
    std::uint32_t a = 4000000000u;
    std::uint32_t b = 500000000u;

    auto [s, of] = add_u_with_overflow(a, b);
    std::cout << "a=" << a << " b=" << b << " sum=" << s << " overflow=" << of << "\n";
    std::cout << "sum bits=" << bits_u(s) << "\n";

    std::uint32_t d = sub_u(a, b);
    std::cout << "a-b=" << d << " bits=" << bits_u(d) << "\n";
}

static void demo_min_max()
{
    std::cout << "\n== min/max branchless patterns (unsigned) ==\n";
    std::uint32_t a = 123u, b = 77u;
    std::cout << "min=" << min_u_branchless(a,b) << " max=" << max_u_branchless(a,b) << "\n";
}

static void demo_abs()
{
    std::cout << "\n== abs: portable vs bit-trick caution ==\n";
    int x = -123;
    std::cout << "abs_portable(" << x << ")=" << abs_portable(x) << "\n";
    // Note: abs(INT_MIN) overflows for signed int; that's UB in C++.
    // Use wider type or handle separately.
}

static void demo_negate_unsigned_equivalence()
{
    std::cout << "\n== negate in unsigned domain ==\n";
    std::uint32_t x = 42u;
    std::uint32_t neg1 = static_cast<std::uint32_t>(0u - x);
    std::uint32_t neg2 = static_cast<std::uint32_t>(~x + 1u);
    std::cout << "0-x=" << neg1 << " (~x+1)=" << neg2 << " equal=" << (neg1==neg2) << "\n";
}

int main()
{
    demo_add_sub();
    demo_min_max();
    demo_abs();
    demo_negate_unsigned_equivalence();

    std::cout << "\n(bits_arithmetic_tricks.cpp) OK\n";
    return 0;
}
