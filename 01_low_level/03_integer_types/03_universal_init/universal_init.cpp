// ReSharper disable All
#include <iostream>
#include <vector>

// Demonstrates different initialization syntaxes and auto-deduction nuances
static void universal_init()
{
    // Google's auto guidelines
    // https://google.github.io/styleguide/cppguide.html#auto

    // An initializer can use one of four syntactic styles:
    int a1 { 1 };
    int a2 = { 1 };
    int a3 = 1;
    int a4(1);

    (void)a1; (void)a2; (void)a3; (void)a4; // silence unused warnings

    // {}-initialization does not allow narrowing

    // Important note: z1 is an `initializer_list<int>` in C++11 and `int` in C++17 and later
    auto z1 { 99 };

    // z2 is an int
    auto z2 = 99;

    // C++17 and later: z3 is an int
    auto z3{99};

    std::cout << std::dec << "z1 = " << z1 << "; z2 = " << z2 << "; z3 = " << z3 << '\n';

    // The classical example is a vector of integers:
    std::vector<int> v1 { 99 }; // v1 is a vector of 1 element with the value 99
    std::vector<int> v2(99);    // v2 is a vector of 99 elements each with the default value 0

    // We can decorate a deduced type with specifiers and modifiers, such as const and &
    for (const auto& x : v1) {
        std::cout << x << std::endl;
    }

    // Use v2 minimally to avoid unused warning in some compilers
    std::cout << "v2 size = " << v2.size() << '\n';
}

int main()
{
    universal_init();
    return 0;
}

