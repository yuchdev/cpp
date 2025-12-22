/*
================================================================================
08_function_pointers_and_member_pointers.cpp

Advanced topic: function pointers vs member pointers + std::invoke.

Key facts:
  - function pointer: usually a code address
  - member function pointer: ABI-encoded descriptor (may include this-adjustment)
  - member object pointer: offset-like descriptor (ABI-defined)
  - member pointers may be larger than void*, cannot be safely cast to/from void*
  - std::invoke unifies invocation for all callable categories

This file demonstrates:
  1) free function pointer usage
  2) member function pointer usage
  3) member object pointer usage
  4) std::invoke on each
  5) sizeof differences (ABI hint)
================================================================================
*/

#include <iostream>
#include <functional>

template <class... Ts>
void println(Ts&&... xs) { ((std::cout << xs), ...) << "\n"; }

int add(int a, int b) { return a + b; }

struct X {
    int base{0};
    int plus(int v) const { return base + v; }
};

int main() {
    int (*pf)(int,int) = &add;
    println("pf(2,3) = ", pf(2,3));
    println("invoke(pf,10,20) = ", std::invoke(pf, 10, 20));

    int (X::*pmf)(int) const = &X::plus;
    X x{7};
    println("(x.*pmf)(5) = ", (x.*pmf)(5));
    println("invoke(pmf,x,5) = ", std::invoke(pmf, x, 5));

    int X::*pmd = &X::base;
    println("x.*pmd = ", x.*pmd);
    x.*pmd = 99;
    println("after set: x.base = ", x.base);

    println("\nABI hint sizes:");
    println("sizeof(function ptr) = ", sizeof(pf));
    println("sizeof(member fn ptr)= ", sizeof(pmf));
    println("sizeof(member obj ptr)= ", sizeof(pmd));
    return 0;
}
