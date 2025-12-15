// enums_switch_and_evolution_cpp20.cpp
// Focus: switch handling, unknown values, exhaustiveness patterns, and C++20 'using enum'.
// Target: C++11..C++20 (C++20 feature guarded)

#include <cassert>
#include <cstdint>
#include <iostream>
#include <type_traits>

enum class Mode : std::uint8_t
{
    Idle = 0,
    Run  = 1,
    Stop = 2
};

template <typename E>
constexpr auto to_underlying(E e) noexcept -> typename std::underlying_type<E>::type
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}

// Switch “exhaustiveness” in C++ is not enforced by the language.
// Compilers can warn, but correctness requires an explicit policy.
static const char* to_string(Mode m)
{
    switch (m)
    {
        case Mode::Idle: return "Idle";
        case Mode::Run:  return "Run";
        case Mode::Stop: return "Stop";
    }
    // If m holds a value not equal to any enumerator, we land here.
    // This can happen if you cast from int, read from wire, corrupted memory, etc.
    return "<unknown>";
}

// Pattern: validate / clamp when converting from external input.
static bool try_parse_mode(int raw, Mode& out)
{
    switch (raw)
    {
        case 0: out = Mode::Idle; return true;
        case 1: out = Mode::Run;  return true;
        case 2: out = Mode::Stop; return true;
        default: return false;
    }
}

static void switch_and_unknown_values()
{
    Mode m = static_cast<Mode>(99); // value not among enumerators
    assert(std::string(to_string(m)) == "<unknown>");

    Mode parsed{};
    assert(try_parse_mode(1, parsed) && parsed == Mode::Run);
    assert(!try_parse_mode(123, parsed));
}

#if __cplusplus >= 202002L
// C++20: 'using enum' brings enumerators into scope (for scoped enums too).
// This reduces verbosity but can reintroduce collisions if overused in headers.
static void using_enum_demo()
{
    using enum Mode; // imports Idle, Run, Stop into current scope
    Mode m = Run;    // instead of Mode::Run
    assert(to_underlying(m) == 1);

    // Beware of collisions with other enums that have enumerators named Run, Stop, etc.
}
#endif

int main()
{
    switch_and_unknown_values();

#if __cplusplus >= 202002L
    using_enum_demo();
#else
    std::cout << "(C++20 using enum demo skipped; compile with -std=c++20)\n";
#endif

    std::cout << "enums_switch_and_evolution_cpp20.cpp: OK\n";
    return 0;
}
