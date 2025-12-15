// enums_scoped_and_forward.cpp
// Focus: scoped enums, fixed underlying types, forward declaration rules, and evolution.
// Target: C++11..C++20 (uses enum class + fixed underlying types)

#include <cassert>
#include <cstdint>
#include <iostream>
#include <type_traits>

// ---------------------------------------------
// Key facts (C++11+):
// - enum class (scoped enum) does NOT implicitly convert to int.
// - Enumerators do NOT leak into the enclosing scope (unlike unscoped enums).
// - Underlying type can be specified: enum class E : std::uint8_t { ... }
// - Scoped enums have a fixed underlying type if specified; otherwise still integral but chosen by compiler.
// - Forward declaration:
//     * scoped enums can always be forward-declared (with or without underlying type)
//       but to be a complete type, it must be defined.
//     * unscoped enums can only be forward-declared if you specify the underlying type.
//
// Evolution notes:
// - C++98: no enum class, enumerators injected into scope, implicit conversions everywhere.
// - C++11: enum class, fixed underlying types, forward decl rules, better type safety.

enum class TrafficLight : std::uint8_t { Green = 1, Yellow = 2, Red = 3 };

// Forward decl scoped enum: OK
enum class ErrorCode; // incomplete type until definition

// Forward decl unscoped enum: requires underlying type (C++11+)
enum LegacyStatus : unsigned int;

enum class ErrorCode : int
{
    Ok = 0,
    NotFound = 404,
    PermissionDenied = 403
};

enum LegacyStatus : unsigned int
{
    LS_Off = 0,
    LS_On = 1
};

template <typename E>
constexpr auto to_underlying(E e) noexcept -> typename std::underlying_type<E>::type
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}

static void scoped_enum_basics()
{
    TrafficLight t = TrafficLight::Green;

    // No implicit conversion:
    // int x = t; // ERROR
    int x = static_cast<int>(t);
    assert(x == 1);

    // Enumerators are scoped:
    // Green; // ERROR
    (void)t;
}

// Unscoped enums leak enumerators into enclosing scope, causing collisions.
enum Color { Red, Green, Blue };

static void name_injection_collisions()
{
    // Potential collision with TrafficLight::Red if you did "using enum TrafficLight" (C++20)
    // or used unscoped enums extensively in headers.

    Color c = Red; // OK: injected into scope
    (void)c;
}

// Underlying type facts
static void underlying_type_facts()
{
    static_assert(std::is_same<std::underlying_type<TrafficLight>::type, std::uint8_t>::value, "");
    static_assert(sizeof(TrafficLight) == sizeof(std::uint8_t), "usually true if fixed underlying type");

    auto raw = to_underlying(TrafficLight::Yellow);
    assert(raw == 2);
}

static void forward_decl_rules()
{
    // You can declare pointers/references to incomplete enum types:
    ErrorCode* p = nullptr;
    (void)p;

    // But you cannot sizeof an incomplete type:
    // sizeof(ErrorCode); // ERROR if only forward-declared
}

int main()
{
    scoped_enum_basics();
    name_injection_collisions();
    underlying_type_facts();
    forward_decl_rules();

    std::cout << "enums_scoped_and_forward.cpp: OK\n";
    return 0;
}
