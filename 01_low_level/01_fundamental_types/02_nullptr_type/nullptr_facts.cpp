// ReSharper disable All
#include <utilities/defines.h>
#include <utilities/functional.h>
SUPPRESS_PRAGMA_WARNINGS()

#include <iostream>
#include <type_traits>
#include <cassert>

/// Prior to C++11: C-macro NULL
/// Since C++11: nullptr (type std::nullptr_t)
/// nullptr prevents overload ambiguity between pointer and integer overloads.
void accept_null(int)
{
    std::cout << "accept_null(int)\n";
}
void accept_null(void*)
{
    std::cout << "accept_null(void*)\n";
}
void accept_null(int*)
{
    std::cout << "accept_null(int*)\n";
}
void accept_null(std::nullptr_t)
{
    std::cout << "accept_null(std::nullptr_t)\n";
}

/// Fundamental C++ type: std::nullptr_t / nullptr
/// https://en.cppreference.com/w/cpp/types/nullptr_t
void nullptr_type()
{
    // NULL / 0 may select int overload or become ambiguous depending on overload set
    accept_null(0);

    // nullptr is a null pointer literal with distinct type std::nullptr_t
    int* ptr1 = nullptr;
    void* ptr2 = nullptr;

    accept_null(nullptr); // picks nullptr_t overload (best match)
    accept_null(ptr1);    // int*
    accept_null(ptr2);    // void*

    // nullptr is a constant expression
    // NOTE: sizeof(nullptr_t) is implementation-defined.
    // (Many implementations use pointer-sized storage, but don't rely on it.)
    std::cout << "sizeof(std::nullptr_t) = " << sizeof(std::nullptr_t) << '\n';

    // Since C++14, std::is_null_pointer<T> exists
    std::cout
        << "std::is_null_pointer<int*>::value = " << std::is_null_pointer<int*>::value << '\n'
        << "std::is_null_pointer<std::nullptr_t>::value = " << std::is_null_pointer<std::nullptr_t>::value << '\n';

    // Obscure: cannot take address of nullptr (it is a pure literal)
    // auto p = &nullptr; // ill-formed
}

int main(int argc, char* argv[])
{
    nullptr_type();
    return 0;
}
