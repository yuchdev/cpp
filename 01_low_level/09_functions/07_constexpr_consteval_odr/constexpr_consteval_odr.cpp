/*
================================================================================
07_constexpr_consteval_odr.cpp

Advanced topic: constexpr / consteval and ODR/inline implications.

Key facts:
  - constexpr function can run at compile time OR runtime.
  - consteval function MUST run at compile time.
  - constexpr functions are implicitly inline (safe in headers).
  - Non-inline non-static free function definitions in headers can violate ODR.

This file demonstrates:
  1) constexpr used in static_assert and at runtime
  2) consteval compile-time enforcement
================================================================================
*/

#include <iostream>

template <class... Ts>
void println(Ts&&... xs) { ((std::cout << xs), ...) << "\n"; }

constexpr int square(int x) { return x * x; }
consteval int id(int x) { return x; }

int runtime_square(int x) { return x * x; }

int main() {
    constexpr int a = square(5);
    static_assert(a == 25);

    constexpr int b = id(7);
    static_assert(b == 7);

    println("square(9) runtime = ", square(9));
    println("runtime_square(9) = ", runtime_square(9));

    // Uncomment to see compile error:
    // int x = 3;
    // int r = id(x);

    println("\nNotes:");
    println("* constexpr implies inline -> okay in headers.");
    println("* consteval forces compile-time evaluation.");
    return 0;
}
