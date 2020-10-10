#include <iostream>
#include <cstdint>

// Facts about pointers
void pointers_facts()
{
    // fact: a pointer to a non-array dynamic object behaves the same as a pointer to the
    // first element of an array of length one

    // x86 of today use little-endian storage for all types of data(integer, floating point, BCD)
    long example = 1L;
    uint8_t* explore_bytes = reinterpret_cast<uint8_t*>(&example);

    for (size_t i = 0; i < sizeof(example); ++i) {
        std::cout << i << " byte of long " << example << " is " << explore_bytes++ << '\n';
    }

    // Every object has an address
    // Guaranteed that there are no objects with NULL address
    int a{};
    int* ptr_a = &a;

    // fact: pointer size

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
    int some_array[10] = {0,1,2,3,4,5,6,7,8,9};
    int* ptr_to_first = some_array;


    // 1.pointer + integer: moves forward from the pointer
    int* fifth_element = ptr_to_first + 5;

    // 2.pointer - integer: moves backward from the pointer
    int* second_element = ptr_to_first -3;

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

void pointer_conversions()
{
char** p = 0;
const char** p1 = p; // error: level 2 more cv-qualified but level 1 is not const
const char* const * p2 = p; // OK: level 2 more cv-qualified and const added at level 1
volatile char * const * p3 = p; // OK: level 2 more cv-qual and const added at level 1
volatile const char* const* p4 = p2; // OK: 2 more cv-qual and const was already at 1

double *a[2][3];
double const * const (*ap)[3] = a; // OK
double * const (*ap1)[] = a; // OK since C++20
}

int main()
{
    pointers_facts();
    return 0;
}
