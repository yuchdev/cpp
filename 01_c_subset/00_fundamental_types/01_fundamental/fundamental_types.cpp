#include <utilities/defines.h>
#include <utilities/functional.h>
SUPPRESS_PRAGMA_WARNINGS()
#pragma ide diagnostic ignored "UnusedValue"
#pragma ide diagnostic ignored "UnusedLocalVariable"
#pragma ide diagnostic ignored "modernize-use-auto"

#include <iostream>
#include <type_traits>
#include <cassert>

#include <utilities/bitwise.h>

/// Fundamental C++ types
/// https://en.cppreference.com/w/cpp/language/types
void boolean_type()
{
    // The value of sizeof(bool) is implementation defined and might differ from 1
    // Bitwise representation is implementation-defined
    // Naming of bool variables and function often express its boolean nature, starting from "is"
    bool is_root = true;
    std::cout
        << "sizeof(bool) = " << sizeof(is_root)
        << "; "
        << "bitwise(bool) = " << bitwise(is_root) << '\n';

    // Operator sizeof() yields size in bytes of the object representation of type or expression
    // sizeof() cannot be used with function types, incomplete types, or bit-field l-values

    // Old and new style of initialization
    bool b1 = true;
    bool b2 { false };

    // narrowing in expression (cast to 1)
    int i1 = b1;
    int i2 { b1 };

    // narrowing in expression (cast to 1)
    int i3 = 5 + b1;

    // narrowing to pointers (cast to nullptr)
    int* pint = nullptr;
    if (pint) {/*never be there*/ }
}

/// Prior to C++11 standard null pointer was served by C-macro NULL
/// C++11 standard null pointer is defined as nullptr
/// The problem with NULL is that when we pass integer 0, we can't say if it's a pointer or integer
/// It potentially could be a problem during function overloading over types void* and int,
/// and template metaprogramming
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
    std::cout << "accept_intptr(int*)\n";
}

void accept_null(std::nullptr_t)
{
    std::cout << "accept_null(std::nullptr_t)\n";
}

/// 2. nullptr_type
void nullptr_type()
{
    // Trying to call with NULL
    // error: call to 'accept_null' is ambiguous
    // accept_null(NULL);
    // accept_null(0);

    accept_null(0);

    // Since C++11, we have a null pointer literal, nullptr, of std::nullptr_t type
    // If two or more overloads accept different pointer types (void* and int* in our case),
    // an overload for std::nullptr_t is necessary to accept a null pointer argument.
    // Otherwise, we got an error: call to 'accept_null' is ambiguous
    int* ptr1 = nullptr;
    void* ptr2 = nullptr;
    accept_null(nullptr);
    accept_null(ptr1);
    accept_null(ptr2);

    // nullptr is a constant expression, equal to size of void*
    assert(sizeof(std::nullptr_t) == sizeof(void*));

    // Since C++14, we have a template struct is_null_pointer()
    std::cout
        << "std::is_null_pointer<int*>::value="
        << std::is_null_pointer<int*>::value << '\n';
}

/// void is type with an empty set of values
/// It is an incomplete type that cannot be completed
/// (consequently, objects of type void are disallowed)
/// There are no arrays of void, nor references to void.
/// However, pointers to void and functions returning type void.
/// Type void have use in template metaprogramming
/// to show that a type is not known at compile time
void void_type()
{
    // Pointer of any type can be implicitly converted to pointer to void, without changing its value
    // The reverse conversion requires static_cast
    int n = 0;
    int* p_int = &n;
    void* p_void = p_int;
    int* p_int2 = static_cast<int*>(p_void);
    assert(p_int == p_int2);
}

int main(int argc, char* argv[])
{
    constexpr FunctionMap function_map{
        { "boolean_type", boolean_type },
        { "nullptr_type", nullptr_type },
        { "void_type", void_type }
    };
    boolean_type();
    nullptr_type();
    void_type();
    return 0;
}
