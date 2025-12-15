// ReSharper disable All
#include <utilities/defines.h>
#include <utilities/functional.h>
#include <utilities/bitwise.h>
SUPPRESS_PRAGMA_WARNINGS()

#include <iostream>
#include <type_traits>
#include <cassert>

/// Fundamental C++ type: bool
/// https://en.cppreference.com/w/cpp/language/types
void boolean_type()
{
    // sizeof(bool) is implementation-defined (but >= 1)
    // Object representation for bool is implementation-defined
    bool is_root = true;

    std::cout
        << "sizeof(bool) = " << sizeof(is_root)
        << "; bitwise(bool) = " << bitwise(is_root) << '\n';

    // Old/new style initialization
    bool b1 = true;
    bool b2{false};

    // Conversion to int: value-preserving (true->1, false->0)
    int i1 = b1;
    int i2{b1};
    int i3 = 5 + b1;

    std::cout << "b1=" << b1 << " i1=" << i1 << " i2=" << i2 << " i3=" << i3 << '\n';

    // Pointer -> bool conversion (common pitfall: "if(ptr)" checks non-null, not “validity")
    int* pint = nullptr;
    if (pint)
    {
        std::cout << "never here\n";
    }

    // Obscure: vector<bool> is specialized (proxy reference), not a real bool&
    // Not shown here to keep file minimal, but keep in mind for generic code.
}

int main(int argc, char* argv[])
{
    boolean_type();
    return 0;
}
