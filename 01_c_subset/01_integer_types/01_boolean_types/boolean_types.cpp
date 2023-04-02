#include <utilities/defines.h>
SUPPRESS_PRAGMA_WARNINGS()
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma ide diagnostic ignored "UnusedLocalVariable"
#pragma ide diagnostic ignored "UnusedValue"
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
#pragma ide diagnostic ignored "cert-str34-c"

#include <utilities/bitwise.h>
#include <utilities/defines.h>
SUPPRESS_UNSIGNED_COUNTEREXAMPLE_WARNINGS()

#include <iostream>
#include <cstdint>

// Fundamental C++ types
// https://en.cppreference.com/w/cpp/language/types

// void - type with an empty set of values
// There are no arrays of void, nor references to void. However, pointers to void and functions returning type void.
// Type void have use in template metaprogramming
void boolean_type()
{
    // C++11: bool
    // C99: _Bool
    // The value of sizeof(bool) is implementation defined and might differ from 1
    // Bitwise representation is implementation-defined
    // Naming of bool often express its boolean nature, starting from "is"
    bool is_root = true;
    std::cout
        << "sizeof(bool) = " << sizeof(is_root)
        << "; "
        << "bitwise(bool) = " << bitwise(is_root) << '\n';

    // Operator sizeof() yields size in bytes of the object representation of type or expression
    // sizeof() cannot be used with function types, incomplete types, or bit-field values
}


int main(int argc, char* argv[])
{
    boolean_type();
    return 0;
}
