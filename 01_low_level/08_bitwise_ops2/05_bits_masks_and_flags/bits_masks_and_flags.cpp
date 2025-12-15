/*
bits_masks_and_flags.cpp
Deep coverage: robust bitmask/flag design in C++ using enum class, operators,
masking pitfalls (~), and best practices for APIs and serialization.

Build (C++17):
  g++ -std=c++17 -O2 -Wall -Wextra -pedantic bits_masks_and_flags.cpp -o bits_flags
*/

// Common helper: print bits of unsigned integers in binary (no external deps).
// Works in C++17+. Uses std::bitset with width = sizeof(T)*CHAR_BIT.
// For signed values, cast to the corresponding unsigned type first.

#include <bitset>
#include <climits>
#include <cstdint>
#include <iostream>
#include <limits>
#include <cstdint>
#include <string>
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

template <class E>
constexpr auto to_underlying(E e) noexcept -> typename std::underlying_type<E>::type
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}

enum class Perm : std::uint32_t
{
    None  = 0,
    Read  = 1u << 0,
    Write = 1u << 1,
    Exec  = 1u << 2,

    // Recommended: define All to make masking safe.
    All = Read | Write | Exec
};

constexpr Perm operator|(Perm a, Perm b) noexcept
{
    return static_cast<Perm>(to_underlying(a) | to_underlying(b));
}
constexpr Perm operator&(Perm a, Perm b) noexcept
{
    return static_cast<Perm>(to_underlying(a) & to_underlying(b));
}
constexpr Perm operator^(Perm a, Perm b) noexcept
{
    return static_cast<Perm>(to_underlying(a) ^ to_underlying(b));
}
constexpr Perm operator~(Perm a) noexcept
{
    // PITFALL: ~ flips all bits of the underlying integer.
    // Correct approach: mask with Perm::All.
    return static_cast<Perm>(~to_underlying(a));
}
inline Perm& operator|=(Perm& a, Perm b) noexcept { a = a | b; return a; }
inline Perm& operator&=(Perm& a, Perm b) noexcept { a = a & b; return a; }

constexpr bool has(Perm value, Perm flag) noexcept
{
    return (to_underlying(value & flag) != 0);
}

static std::string to_string(Perm p)
{
    std::string s;
    if (p == Perm::None) return "None";
    if (has(p, Perm::Read))  s += "R";
    if (has(p, Perm::Write)) s += "W";
    if (has(p, Perm::Exec))  s += "X";
    return s;
}

static void demo_flags()
{
    std::cout << "== enum class flags ==\n";
    Perm p = Perm::None;
    p |= Perm::Read;
    p |= Perm::Write;

    std::cout << "p=" << to_string(p) << " raw=" << to_underlying(p) << "\n";
    std::cout << "p bits=" << bits_u(to_underlying(p)) << "\n";

    // Demonstrate ~ pitfall:
    auto inv_raw = to_underlying(~Perm::Read);
    std::cout << "~Read raw bits=" << bits_u(inv_raw) << " (WRONG without masking)\n";

    // Correct: mask to known bits:
    Perm inv = static_cast<Perm>(to_underlying(~Perm::Read) & to_underlying(Perm::All));
    std::cout << "masked ~Read => " << to_string(inv) << " raw=" << to_underlying(inv) << "\n";
}

static void api_boundary_hint()
{
    std::cout << "\n== API boundary hint ==\n";
    // Serialize flags as underlying integer:
    std::uint32_t wire = to_underlying(Perm::Read | Perm::Exec);
    std::cout << "serialize: " << wire << "\n";

    // Validate when reading from external input:
    std::uint32_t incoming = 0xFFFF'FFFFu;
    std::uint32_t masked = incoming & to_underlying(Perm::All);
    Perm safe = static_cast<Perm>(masked);
    std::cout << "incoming masked => " << to_string(safe) << "\n";
}

int main()
{
    demo_flags();
    api_boundary_hint();
    std::cout << "\n(bits_masks_and_flags.cpp) OK\n";
    return 0;
}
