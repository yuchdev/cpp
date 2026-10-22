// enums_core_and_sizes.cpp
// Focus: C++98-style enums, underlying type, range, size, conversions, and common traps.
// Target: C++98..C++20 (no required C++20-only features here)

#include <cassert>
#include <cstddef>
#include <iostream>
#include <limits>
#include <type_traits>

// ---------------------------------------------
// Key facts (C++98+):
// - An enum is a distinct type, but it implicitly converts to int (unscoped enum).
// - Underlying type for unscoped enums is implementation-defined (usually int).
// - Enumerators are integral constant expressions.
// - A non-fixed enum has a restricted value range. Casting an out-of-range
//   integer to it has undefined behavior (CWG 1766). An in-range but unnamed
//   value is possible; fixed-underlying enums can hold every underlying value.
// - Size of an unscoped enum is implementation-defined (but at least enough to represent values).
//
// Evolution:
// - C++11 introduced scoped enums (enum class) and fixed underlying types.
// - C++11 also allows specifying underlying type for unscoped enums too: enum E : unsigned char { ... };

enum PlainSmall { ps0, ps1 };                // typically fits in int
enum PlainExplicitVals { pe_a = 3, pe_b = 9 };
enum PlainBigger { pb_a = 3, pb_b = 900 };

static void show_sizes()
{
    std::cout << "sizeof(PlainSmall)        = " << sizeof(PlainSmall) << "\n";
    std::cout << "sizeof(PlainExplicitVals) = " << sizeof(PlainExplicitVals) << "\n";
    std::cout << "sizeof(PlainBigger)       = " << sizeof(PlainBigger) << "\n";
}

// Demonstrate implicit conversions and overload hazards.
static void takes_int(int) { std::cout << "takes_int(int)\n"; }
static void takes_plain(PlainSmall) { std::cout << "takes_plain(PlainSmall)\n"; }

static void implicit_conversion_hazards()
{
    PlainSmall e = ps1;

    // Unscoped enum converts to int implicitly:
    takes_int(e);

    // Overload resolution can surprise:
    takes_plain(e); // exact match
    // but if you had overloads like takes_int(long) etc. it can drift.

    // Another classic hazard:
    // - Unscoped enums participate in arithmetic directly (often unintended).
    int x = e + 100; // allowed
    (void)x;
}

// Casting integers to enums.
static void casting_outside_enumerators()
{
    PlainSmall ok = static_cast<PlainSmall>(0);
    assert(ok == ps0);

    // A non-fixed enum with values 1, 2, 4 has a range that includes 3.
    // The cast is in range and creates an unnamed, well-defined enum value.
    enum Flags { read = 1, write = 2, execute = 4 };
    Flags combined = static_cast<Flags>(3);
    assert(static_cast<int>(combined) == 3);

    // static_cast<PlainSmall>(20) would be undefined behavior: 20 is
    // outside PlainSmall's language-defined value range.
    // Validate external integers before converting to a non-fixed enum.
}

// Demonstrate why "enums are not just ints" (types differ even if representation matches).
static void enums_are_distinct_types()
{
    PlainSmall a = ps0;
    PlainExplicitVals b = pe_a;

    // a = b; // ERROR: different enum types
    (void)a; (void)b;
}

// Enumerators are integral constant expressions: usable in switch/case, array bounds, etc.
static void enumerators_are_constants()
{
    int arr[pe_b]; // pe_b == 9, OK (constant expression)
    static_assert(pe_b == 9, "enumerator is constant expression");
    (void)arr;
}

int main()
{
    show_sizes();
    implicit_conversion_hazards();
    casting_outside_enumerators();
    enums_are_distinct_types();
    enumerators_are_constants();

    std::cout << "enums_core_and_sizes.cpp: OK\n";
    return 0;
}
