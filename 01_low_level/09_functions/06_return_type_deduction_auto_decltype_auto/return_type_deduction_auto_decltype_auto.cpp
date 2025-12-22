/*
================================================================================
06_return_type_deduction_auto_decltype_auto.cpp

Advanced topic: return type deduction traps (auto vs decltype(auto)).

Key facts:
  - `auto` return strips refs/cv -> returns copies for lvalues.
  - `decltype(auto)` uses decltype rules and can preserve references.
  - Parentheses matter: `return x;` vs `return (x);`
  - Returning references to locals is still UB (dangling), even if deduced.

This file demonstrates:
  1) auto returns a copy; decltype(auto) returns a reference.
  2) Call-site types decide copying vs binding.
  3) A forwarding-return helper.
================================================================================
*/

#include <iostream>
#include <type_traits>
#include <utility>

template <class... Ts>
void println(Ts&&... xs) { ((std::cout << xs), ...) << "\n"; }

int global_x = 10;

auto returns_copy() { return global_x; }

decltype(auto) returns_ref() { return (global_x); }

decltype(auto) returns_dangling_ref() {
    int local = 7;
    return (local); // would dangle if called
}

template <class T>
decltype(auto) forward_return(T&& t) { return std::forward<T>(t); }

static void demo_basic() {
    println("== auto vs decltype(auto) ==");

    auto a = returns_copy(); // copy
    auto& b = returns_ref(); // ref

    println("initial: global_x=", global_x, " a=", a, " b=", b);
    a = 1;
    b = 2;
    println("after:   global_x=", global_x, " a=", a, " b=", b);
}

static void demo_forwarding() {
    println("\n== forwarding return ==");

    int x = 5;
    auto&& r1 = forward_return(x); // int&
    r1 = 99;
    println("x after r1 = ", x);

    auto r2 = forward_return(123); // copies into auto
    println("r2 = ", r2);
}

int main() {
    demo_basic();
    demo_forwarding();
    println("\nNote: returns_dangling_ref() exists as a warning example; do not call.");
    return 0;
}
