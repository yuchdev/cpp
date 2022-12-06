#include <iostream>
#include <cstdint>

namespace cpp
{

    class test_me
    {
    public:
        void point_to_me() {}
    };

    void point_to_me_static() {}

} // namespace cpp


// * Pointers can be converted to intptr_t and back
// * ptrdiff_t is closely tied to the machine pointer type, so as size_t
// * size_t holds sizes of objects, whereas ptrdiff_t the differences of addresses within the objects
// * Legit use of intmax_t is for formatting, and probably serialization


// Facts about pointers
void pointers_facts()
{
    // fact: a pointer to a non-array dynamic object behaves the same as a pointer to the
    // first element of an array of length one

    // x86 of today use little-endian storage for all types of data(integer, floating point, binary-coded decimal)
    long example = 1L;
    uint8_t* explore_bytes = reinterpret_cast<uint8_t*>(&example);

    for (size_t i = 0; i < sizeof(example); ++i) {
        std::cout << i << " byte of long " << example << " is " << explore_bytes++ << '\n';
    }

    // Every object has an address
    // Guaranteed that there are no objects with NULL address
    int a {};
    int* ptr_a = &a;

    // TODO fact: pointer sizes

    // Standard output prints pointers as hexadecimal
    std::cout << "ptr_a = " << ptr_a << '\n';

    // Always take notice about constant semantic of POINTER or OBJECT,
    // which pointer points on

    // pointer is constant, integer does not
    int* const ptr1 = &a;

    // Integer is constant, pointer does not
    const int* ptr2 = &a;
    // .. basically the same
    int const* ptr3 = &a;

    // Both pointer and integer are constant
    const int* const ptr4 = &a;

    // Pointer arithmetic
    int some_array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int* ptr_to_first = some_array;

    // 1.pointer + integer: moves forward from the pointer
    int* fifth_element = ptr_to_first + 5;

    // 2.pointer - integer: moves backward from the pointer
    int* second_element = ptr_to_first - 3;

    // 3.pointer - pointer: is important and means distance between pointers
    // Pointer distance has a special type, defined in <cstddef> include file
    ptrdiff_t some_distance = fifth_element - second_element;

    // 4.++ or -- moves to the pointee size!
    ++ptr_to_first; // moves 4 bytes forward!
    ptrdiff_t other_distance = ptr_to_first - some_array;

    // Exception from this rule is void*
    void* vp = reinterpret_cast<void*>(some_array);
    // ++vp; there's no increment/decrement for void*

    // 4.pointer + pointer: why do we even need this?
}

void nullptr_type()
{
    // nullptr_t
    // std::nullptr_t is the type of the null pointer literal, nullptr
    // It is a distinct type that is not itself a pointer type or a pointer to member type
    int* inull { nullptr };
    void* vnull { nullptr };

    // pointer to function
    void(*static_func_ptr)() = &cpp::point_to_me_static;

    void(cpp::test_me:: * method_ptr)() = &cpp::test_me::point_to_me;

    std::cout
        << "Function pointer void(*static_func_ptr)() = &cpp::point_to_me_static = "
        << static_func_ptr << '\n';
    std::cout
        << "Static method pointer void(cpp::test_me:: * method_ptr)() = &cpp::test_me::point_to_me = "
        << method_ptr << '\n';

    // It's okay to assign nullptr to function pointers and static method pointers
    static_func_ptr = nullptr;
    method_ptr = nullptr;
}

void pointer_conversions()
{
    std::cout << "C++ version: " << __cplusplus << '\n';

    char** p = 0;

    // error: level 2 more cv-qualified but level 1 is not const
    // const char** p1 = p; 

    // OK: level 2 more cv-qualified and const added at level 1
    const char* const* p2 = p;

    // OK: level 2 more cv-qualifiers and const added at level 1
    volatile char* const* p3 = p;

    // OK: 2 more cv-qualifiers and const was already at 1
    volatile const char* const* p4 = p2;

    double* a[2][3] = {};
    // error: level 2 more cv-qualifiers but level 1 is not const
    // double const * const (*ap)[3] = a;

    // TODO: error: level 2 more cv-qualifiers and const added at level 1
#if 0
#if __cplusplus >= 20
    double* const (*ap1)[] = a; // OK since C++20
#endif
#endif
}

void show_const_pointers()
{
    // Pointer to constant
    const int *pp1 = new const int(3);

    // Update: almost recently uninitialized const free memory, both pointers and arrays
    // Before we could create a useless const pointer to a const int or array of ints,
    // and leave it uninitialized.
    // TODO: try on different standards
#if 0
    const int* pp2 = new const int;

    // You can initialize it with any legal way
    // Even const_cast will not work
    //const_cast<int>(*pp2) = 1;

    // You can create uninitialized const dynamic array
    // You can't even use default initialization, and can't change its values in any way
    const int* pp3 = new const int[100];
#endif

    // Standard permits dynamic array of zero size
    int *pp5 = new int[0];

    // unlike regular array
    //int p6[0];

    // Windows-specific function _msize allows to determine real size of dynamic array
    // ()
#if defined(_WIN32) || defined(_WIN64)
    size_t sz = _msize(pp5);
#endif

    delete pp1;
#if 0
    delete pp2;
    delete[] pp3;
#endif
}

int main()
{
    // TODO: insert any function call
    return 0;
}
