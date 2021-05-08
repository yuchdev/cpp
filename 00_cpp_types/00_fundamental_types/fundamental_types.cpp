#include <iostream>
#include <bitwise.h>

// Fundamental C++ types
// https://en.cppreference.com/w/cpp/language/types

// void - type with an empty set of values
// There are no arrays of void, nor references to void. However, pointers to void and functions returning type void.
// Type void have use in template metaprogramming
void boolean_type()
{
    // The value of sizeof(bool) is implementation defined and might differ from 1
    // Bitwise representation is implementation-defined
    // Naming of bool often express its boolean nature, starting from "is"
    bool is_root = true;
    std::cout 
        << "sizeof(bool) = " << sizeof(is_root)
        << "; "
        << "bitwise(bool) = " << bitwise(is_root) << '\n';

    // Operator sizeof() yelds size in bytes of the object representation of type or expression
    // sizeof() cannot be used with function types, incomplete types, or bit-field glvalues
}

int main()
{
    // TODO: insert any function call
    return 0;
}
