/*
================================================================================
03_function_types_decltype_decay_overload.cpp

Advanced topic: function types vs pointers, decltype, overload sets, templates.

Key facts:
  - Overload sets are not values. You must pick an overload to take its address.
  - Functions decay to pointers in many contexts (esp. template deduction).
  - decltype(f) vs decltype(&f) differs; overload sets complicate decltype.
  - Unary + on a non-capturing lambda forces conversion to function pointer.

This file demonstrates:
  1) Overload disambiguation via static_cast
  2) How templates "see" function names (often as pointers)
  3) decltype pitfalls around overload sets
  4) std::invoke with function pointers
================================================================================
*/

#include <iostream>
#include <type_traits>
#include <functional>

template <class... Ts>
void println(Ts&&... xs) { ((std::cout << xs), ...) << "\n"; }

int    over(int x)    { return x + 1; }
double over(double x) { return x + 0.5; }

template <class T>
static void type_report(const char* label) {
    println(label,
            " pointer=", std::is_pointer_v<T> ? "yes" : "no",
            " function=", std::is_function_v<T> ? "yes" : "no",
            " ref=", std::is_reference_v<T> ? "yes" : "no");
}

template <class T>
void sink(T) {
    type_report<T>("  deduced T:");
}

static void demo_overload_address() {
    println("== Overload set disambiguation ==");

    using FnInt = int(*)(int);
    using FnDbl = double(*)(double);

    FnInt pi = static_cast<FnInt>(&over);
    FnDbl pd = static_cast<FnDbl>(&over);

    println("pi(10) = ", pi(10));
    println("pd(10) = ", pd(10));
}

static void demo_decay_and_templates() {
    println("\n== Decay + template deduction ==");

    auto nc = +[](int x) { return x * 2; }; // forces function-pointer conversion
    sink(nc);

    auto closure = [](int x) { return x * 3; };
    sink(closure);
}

static void demo_decltype_tricks() {
    println("\n== decltype tricks ==");

    using FnInt = int(*)(int);
    auto pi = static_cast<FnInt>(&over);

    type_report<decltype(pi)>("decltype(pi):");
    type_report<decltype(static_cast<FnInt>(&over))>("decltype(&over as FnInt):");

    println("std::invoke(pi, 7) = ", std::invoke(pi, 7));
}

int main() {
    demo_overload_address();
    demo_decay_and_templates();
    demo_decltype_tricks();
    println("\nTakeaway: overload sets are not values; decay and decltype rules drive many pitfalls.");
    return 0;
}
