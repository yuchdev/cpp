#include <utilities/defines.h>
#include <utilities/bitwise.h>
SUPPRESS_PRAGMA_WARNINGS()

#include <iostream>
#include <cstdint>

void fixed_size_types()
{
    // Problem of C++ borrowed from C: we have lots of types
    // whose sizes all depend on system or architecture
    // That's why fixed-size (or guaranteed size) types are provided

    // signed integer type with width of exactly 8, 16, 32 and 64 bits respectively
    // with no padding bits and using 2's complement for negative values
    int8_t i8 {};
    int16_t i16 {};
    int32_t i32 {};
    int64_t i64 {};
    // same for uint8_t, uint16_t, uint32_t, uint64_t
    // {} is the default initialization for basic types

    std::cout
        << "sizeof(int8_t) = " << sizeof(int8_t)
        << "sizeof(int16_t) = " << sizeof(int16_t)
        << "sizeof(int32_t) = " << sizeof(int32_t)
        << "sizeof(int64_t) = " << sizeof(int64_t)
        << '\n';

    // fastest signed integer type with width of at least 8, 16, 32 and 64 bits respectively
    // For example, uint32_t is not even guaranteed to exist
    // uint_fast32_t states your intent clearly: 
    // it's a type of at least 32 bits which is the best from a performance point-of-view
    // uint32_t or uint16_t may end up having 64 bit size
    // Example: in a 64-bit machine, where one would expect to have int as 64-bit, 
    // a compiler may use a mode with 32-bit int compilation for compatibility.
    // In this mode, int_fast16_t could be 64-bit
    int_fast8_t if8 {};
    int_fast16_t if16 {};
    int_fast32_t if32 {};
    int_fast64_t if64 {};

    std::cout
        << "sizeof(int_fast8_t) = " << sizeof(int_fast8_t)
        << "sizeof(int_fast16_t) = " << sizeof(int_fast16_t)
        << "sizeof(int_fast32_t) = " << sizeof(int_fast32_t)
        << "sizeof(int_fast64_t) = " << sizeof(int_fast64_t)
        << '\n';

    // smallest signed integer type with width of at least 8, 16, 32 and 64 bits respectively
    // is needed only if one believes they will ever port to an exotic architecture 
    // where CHAR_BIT is larger than eight, which means it won't be even POSIX compatible,
    // which totally exist though (ClearPath, OS2200)
    // It features 36-bit words, 9-bit char, 72-bit non-IEEE floating point,
    // separate address space for code and data, word-addressed
    // and don't have dedicated stack pointer
    // https://en.wikipedia.org/wiki/OS_2200
    // https://www.unisys.com/offerings/high-end-servers/clearpath-systems/clearpath-dorado-systems
    int_least8_t li8 {};
    int_least16_t li16 {};
    int_least32_t li32 {};
    int_least64_t li64 {};

    std::cout
        << "sizeof(int_least8_t) = " << sizeof(int_least8_t)
        << "sizeof(int_least16_t) = " << sizeof(int_least16_t)
        << "sizeof(int_least32_t) = " << sizeof(int_least32_t)
        << "sizeof(int_least64_t) = " << sizeof(int_least64_t)
        << '\n';

    // (un)signed integer type capable of holding a pointer
    // intptr_t can't be assigned a nullptr, it's just an int size of pointer
    intptr_t int_ptr = 0;

    // maximum-width (un)signed integer type
    intmax_t imax {};

    std::cout
        << "sizeof(intptr_t) = " << sizeof(intptr_t)
        << "sizeof(intmax_t) = " << sizeof(intmax_t)
        << '\n';

    // Macro constants defined in header <cstdint>
    // INT8_MIN/INT8_MAX, INT16_MIN/INT16_MAX etc

    // Function-style macros
    // INT16_C, INT32_C
    // expands to an integer constant expression having the value specified by its argument 
    // and whose type is the promoted type of int_least16_t, int_least32_t etc
    // Example: expands to a literal of type uint_least64_t and value 0xdeadbeef
    uint64_t my_uint = UINT64_C(0xdeadbeef);
#if __cplusplus >= 202002L
    std::format("my_uint = %d, sizeof(my_uint) = %d") % my_uint, sizeof(my_uint);
#endif
}

int main(int argc, char* argv[])
{
    fixed_size_types();
    return 0;
}
