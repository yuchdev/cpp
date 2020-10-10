#include <iostream>
#include <cstdint>
#include <bitwise.h>

void integer_types()
{
    // void - type with an empty set of values
    // There are no arrays of void, nor references to void. However, pointers to void and functions returning type void.
    // Type void have use in template metaprogramming

    // The value of sizeof(bool) is implementation defined and might differ from 1
    // Naming of bool often express its boolean nature, starting from "is"
    bool is_root = true;

    // TODO: nullptr_t

/*
    signed char - type for signed character representation.
    unsigned char - type for unsigned character representation.
    Also used to inspect object representations (raw memory).
    char - type for character representation
    It has the same representation and alignment as either signed char or unsigned char, but is always a distinct type
*/

    // int, short and pointer types are system dependent
    // On any system it's guaranteed to have a width of at least 16 bits
    // On 32/64 bit systems it is almost exclusively guaranteed 
    // to have width of at least 32 bits (see below)
    int a = 42;

    // long in practice almost always 32 bit

/*
Data models

The choices made by each implementation about the sizes of the fundamental types 
are collectively known as data model. Four data models found wide acceptance:

        LP32 or 2/4/4 (int is 16-bit, long and pointer are 32-bit)
            16 bit systems
            Win16 API

        ILP32 or 4/4/4 (int, long, and pointer are 32-bit);
            32 bit systems
            Win32 API
            Unix and Unix-like systems (32-bit Linux, macOS)

        LLP64 or 4/4/8 (int and long are 32-bit, pointer is 64-bit)
            64 bit systems
            Win64 API

        LP64 or 4/8/8 (int is 32-bit, long and pointer are 64-bit)
            64 bit systems
            Unix and Unix-like systems (64-bit Linux, macOS)

Other models are very rare. For example, ILP64 (8/8/8: int, long, and pointer are 64-bit)
only appeared in some 64-bit Unix systems
*/
}

void signed_types()
{
    // signed - target type will have signed representation (this is the default if omitted) 
    // unsigned - target type will have unsigned representation 

    // Note: as with all type specifiers, any order is permitted: 
    // unsigned long long int and long int unsigned long name the same type
}

void numeric_promotions()
{
    /*
    Numeric types could be implicitly converted to larger type. Such convertion called promotion
    Promotion never changes the value of converted number

    signed char or signed short can be converted to int;
    char8_t, unsigned char or unsigned short can be converted to int if it can hold its entire value range, and unsigned int otherwise;
    signed char or unsigned char char can be converted to int or unsigned int respectively;
    wchar_t, char16_t, and char32_t can be converted to the first type from the following list able to hold their entire value range:
    int, unsigned int, long, unsigned long, long long, unsigned long long (since C++11);
    an unscoped enumeration type whose underlying type is not fixed can be converted to the first type from the following list
    able to hold their entire value range:
    int, unsigned int, long, unsigned long, long long, or unsigned long long
    an unscoped enumeration type whose underlying type is fixed can be converted to its underlying type
    type bool can be converted to int with the value false becoming 0 and true becoming 1

    all other conversions are not promotions, but rather convertions;
    for example, overload resolution chooses char -> int (promotion) over char -> short (conversion)
    Unlike the promotions, numeric conversions may change the values, with potential loss of precision
    Narrowing convertion - from larger to smaller.
    Integers types are usually cut; floating-point narrowing convertion is UB.
    fp -> integer leads to truncation
    */
}

void fixed_size_types()
{
    // signed integer type with width of exactly 8, 16, 32 and 64 bits respectively
    // with no padding bits and using 2's complement for negative values
    int8_t i8{};
    int16_t i16{};
    int32_t i32{};
    int64_t i64{};
    // same for uint8_t, uint16_t, uint32_t, uint64_t
    // {} is the default initialization for basic types

    // fastest signed integer type with width of at least 8, 16, 32 and 64 bits respectively
    int_fast8_t if8{};
    int_fast16_t if16{};
    int_fast32_t if32{};
    int_fast64_t if64{};


    // smallest signed integer type with width of at least 8, 16, 32 and 64 bits respectively
    int_least8_t li8{};
    int_least16_t li16{};
    int_least32_t li32{};
    int_least64_t li64{};


    // (un)signed integer type capable of holding a pointer
    // intptr_t can't be assigned a nullptr, it's just an int size of pointer
    intptr_t iptr = 0;

    // maximum-width (un)signed integer type
    intmax_t imax{};

    // Macro constants defined in header <cstdint>
    // INT8_MIN/INT8_MAX, INT16_MIN/INT16_MAX etc

    // Function-style macros
    // INT16_C, INT32_C
    // expands to an integer constant expression having the value specified by its argument 
    // and whose type is the promoted type of int_least16_t, int_least32_t etc
    // Example: expands to a literal of type uint_least64_t and value 0xdeadbeef
    uint64_t myuint = UINT64_C(0xdeadbeef);
}

int main()
{
    return 0;
}
