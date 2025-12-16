// enums_flags_and_operators.cpp
// Focus: using enums as bit flags, operator overloading, and safety patterns.
// Target: C++11..C++20

#include <cassert>
#include <cstdint>
#include <iostream>
#include <type_traits>

template <typename E>
constexpr auto to_underlying(E e) noexcept -> typename std::underlying_type<E>::type
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}

// ---------------------------------------------
// Key facts:
// - Unscoped enums can be used as bitmasks too, but implicit int conversion makes accidents common.
// - Scoped enum bitmasks are much safer but require operator overloads.
// - Always choose an unsigned underlying type for bitmasks to avoid sign pitfalls.
// - Be careful with ~ (bitwise NOT): it sets all higher bits too; you typically want to mask.

enum class FilePerm : std::uint32_t
{
    None  = 0,
    Read  = 1u << 0,
    Write = 1u << 1,
    Exec  = 1u << 2
};

// Enable bit ops for this enum (manual, per-enum style).
constexpr FilePerm operator|(FilePerm a, FilePerm b) noexcept
{
    return static_cast<FilePerm>(to_underlying(a) | to_underlying(b));
}

constexpr FilePerm operator&(FilePerm a, FilePerm b) noexcept
{
    return static_cast<FilePerm>(to_underlying(a) & to_underlying(b));
}

constexpr FilePerm operator^(FilePerm a, FilePerm b) noexcept
{
    return static_cast<FilePerm>(to_underlying(a) ^ to_underlying(b));
}

constexpr FilePerm operator~(FilePerm a) noexcept
{
    // Pitfall: ~ flips all bits of the underlying type, not just known flags.
    // We'll return it raw here (educational); production code often masks with "All".
    return static_cast<FilePerm>(~to_underlying(a));
}

inline FilePerm& operator|=(FilePerm& a, FilePerm b) noexcept { a = (a | b); return a; }
inline FilePerm& operator&=(FilePerm& a, FilePerm b) noexcept { a = (a & b); return a; }

constexpr bool has(FilePerm value, FilePerm flag) noexcept
{
    return (to_underlying(value & flag) != 0);
}

static void flags_usage()
{
    FilePerm p = FilePerm::None;
    p |= FilePerm::Read;
    p |= FilePerm::Write;

    assert(has(p, FilePerm::Read));
    assert(has(p, FilePerm::Write));
    assert(!has(p, FilePerm::Exec));

    // Printing: prefer underlying conversion
    std::cout << "perm raw: " << to_underlying(p) << "\n";
}

// Demonstrate "~" pitfall with bitmask enums.
static void bitwise_not_pitfall()
{
    // Suppose only lower 3 bits are meaningful. ~Read will set everything else, too.
    FilePerm inv = ~FilePerm::Read;
    auto raw = to_underlying(inv);

    // raw is not "Write|Exec" – it is (0xFFFF...FFFE) with uint32_t.
    std::cout << "~Read raw: 0x" << std::hex << raw << std::dec << "\n";

    // Common mitigation: define All = Read|Write|Exec and mask: (~x) & All
}

int main()
{
    flags_usage();
    bitwise_not_pitfall();
    std::cout << "enums_flags_and_operators.cpp: OK\n";
    return 0;
}
