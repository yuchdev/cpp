#include <iostream>

// Demonstrates why function overloading works in C++: **name mangling**.
//
// Overloads produce distinct linker symbols encoding parameter types.
// Inspect symbols:
//   g++ -std=c++20 01_name_mangling_and_overload.cpp -o ex1
//   nm ex1 | grep print
//   nm -C ex1 | grep print     // demangle for readability
//
// Mangling is ABI-specific (Itanium ABI on GCC/Clang; different on MSVC).

void print(int x)
{
    std::cout << "print(int): " << x << "\n";
}

void print(double x)
{
    std::cout << "print(double): " << x << "\n";
}

static void show_overload()
{
    print(42);     // calls print(int)
    print(3.14);   // calls print(double)
}

// In C you cannot declare both overloads in the same scope:
// they would map to the same linker symbol `print` and collide.

int main()
{
    show_overload();
    return 0;
}
