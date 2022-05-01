#include <utilities/defines.h>
SUPPRESS_PRAGMA_WARNINGS()
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma ide diagnostic ignored "UnusedLocalVariable"
#pragma ide diagnostic ignored "UnusedValue"
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
#pragma ide diagnostic ignored "cert-str34-c"

#include <iostream>
#include <cstdint>

#include <utilities/bitwise.h>
#include <utilities/defines.h>
SUPPRESS_UNSIGNED_COUNTEREXAMPLE_WARNINGS()

// Fundamental C++ types
// https://en.cppreference.com/w/cpp/language/types

// If less than C++20
#if (__cplusplus <= 201703L)
using char8_t = unsigned char;
#endif

#pragma region 01.boolean_type

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
    // sizeof() cannot be used with function types, incomplete types, or bit-field glvalues
}

#pragma endregion

#pragma region 02.character_type

/** @brief char - type for character representation
 * It has the same representation and alignment as signed char or unsigned char
 * the choice of signed or unsigned for a char is implementation-defined (char might be signed or unsigned)
 * but it is distinct type
 * There were "exotic" char sizes over history, sometimes need to me noticed on legacy systems
 * CDC 6x00 char==6bit
 * Unisys ClearPath/OS2200 char==9bit
 * PDP-6/10, IBM 70xx char==9bit
 * PDP-8 char==12bit
 * PDP-11 char==16bit
 * TexasInstruments C54/62/64x char==16bit
 * Windows CE char==16bit
 * Motorola dsp56k char==24bit
 * The exact number being recorded in macro CHAR_BIT
 */

void character_types()
{
    // Pointer to unsigned char often used to represent raw memory, as a pointer to byte
    char c1 = 'A';
    signed char c2 = -10;
    unsigned char c3 = 0xcc;

    // Output of char types
    std::cout
            << "char c1 = " << c1
            << "signed char c2 = " << c2
            << "unsigned char c3 = " << c3
            << '\n';

    // The functional cast notation int{c} gives the integer value for a character c
    std::cout << c1 << " == " << int{c1} << '\n';

    // wchar_t - type for wide character representation
    // Required to be large enough to represent any supported character code point
    // (32 bits on systems that support Unicode)
    // A notable exception is Windows, where wchar_t is 16 bits and holds UTF-16 characters
    wchar_t w1 = L'A';

    // Output of wide char types requires special stream object
    std::wcout
            << L"wchar_t w1 =" << w1
            << L"; "
            << "sizeof(wchar_t) = " << sizeof(wchar_t)
            << L'\n';

    // Literals of fixed size are presented as sequences
    // of two, four or eight hexadecimal digits
    // Wide characters (16 and 32 bit) preceded by a u'' or U''
    // 16-bit Unicode symbols hold most of alphabetical and pseudographic symbols
    // 32-bit Unicode symbol may hold emoji and other extended symbols
    // See also https://en.wikipedia.org/wiki/UTF-32

    // type for UTF - 8 character representation; Same size as unsigned char
    char8_t c5 = 'c';
    // type for UTF-16 character representation; Same size as std::uint_least16_t
    char16_t c6 = u'\u00df';
    // type for UTF-32 character representation; Same size as std::uint_least32_t
    char32_t c7 = U'\U0001f34c';

    static_assert(sizeof(c5) == sizeof(char), "char8_t is not char");
    static_assert(sizeof(c6) == sizeof(char) * 2, "char16_t has unexpected size");
    static_assert(sizeof(c7) == sizeof(char) * 4, "char32_t has unexpected size");

    // There's no system-independent and consistent way to out all Unicode symbols in console
}

#pragma endregion

#pragma region 03.integer_types

void integer_types()
{
    //nullptr is a special value of type nullptr_t
    // It was introduces to avoid ambiguous NULL (void* or int)

    // int, short and pointer types are system dependent
    // On any system it's guaranteed to have a width of at least 16 bits
    // On 32/64-bit systems it is almost exclusively guaranteed
    // to have width of at least 32 bits
    int a = 42;
    std::cout
        << "int a = " << a
        << "; "
        << "sizeof(int) = " << sizeof(int)
        << '\n';

    // long in practice almost always 32 bit, however also depends on data models (see below)
    long b = 0xffcc;
    std::cout
        << "long b = " << b
        << "; "
        << "sizeof(long) = " << sizeof(long)
        << '\n';

    // x86/64 processors use "Little Endian" byte order
    // "Little Endian" means that the low-order byte of the number 
    // is stored in memory at the lowest address, and the high-order byte 
    // at the highest address, i.e. the little end comes first

    // Let's use variable of type unsigned long long and pointer to unsigned char
    unsigned long long little_endian = 0x8899aabbccddeeff;
    unsigned char* p_byte = reinterpret_cast<unsigned char*>(&little_endian); // NOLINT(modernize-use-auto)
    for (size_t i = 0; i < sizeof(unsigned long long); ++i, ++p_byte) {
        std::cout << "Byte " << i << " = " << std::hex << *p_byte << '\n';
    }

    // Look once again on bitwise representation
    std::cout << "bitwise(little_endian) = " << bitwise(little_endian) << '\n';

#if (__cplusplus > 201703L)
    // Purpose of the little-endian popularity is that LE system can read from memory 
    // at different lengths without using different addresses
    // For example, a 64-bit memory location with content [4A 00 00 00 00 00 00 00] 
    // can be read at the same address as either 8-bit (value = 4A), 16-bit (004A), or 32-bit (0000004A)
    union {
        uint8_t u8; 
        uint16_t u16; 
        uint32_t u32; 
        uint64_t u64;
    } u = { .u64 = 0x4A };
    
    std::cout
        << "8-bit value = " << u.u8
        << "16-bit value = " << u.u16
        << "32-bit value = " << u.u32
        << "64-bit value = " << u.u64
        << '\n';
#endif

    // Some big-endian architectures include the IBM z/Architecture, AVR32, SPARC, and OpenRISC
    // Most of ARM processors support switching endianness, being effectively double-endian

    // Regarding the size Standard guarantees
    // 1 == sizeof(char) <= sizeof(short) <= sizeof(int) <= sizeof(long) <= sizeof(long long)

/*
    Data models

    The choices made by each implementation about the sizes of the fundamental types
    are collectively known as data model. Four data models found wide acceptance:

    1.  LP32 or 2/4/4 (int is 16-bit, long and pointer are 32-bit)
        16 bit systems, Win16 API
    2.  ILP32 or 4/4/4 (int, long, and pointer are 32-bit): 
        32 bit systems
        Win32 API
        Unix and Unix-like systems (32-bit Linux, macOS)
    3.  LLP64 or 4/4/8 (int and long are 32-bit, pointer is 64-bit)
        64 bit systems
        Win64 API
    4.  LP64 or 4/8/8 (int is 32-bit, long and pointer are 64-bit)
        64 bit systems
        Unix and Unix-like systems (64-bit Linux, macOS)

    Other models are very rare. For example, ILP64 (8/8/8: int, long, and pointer are 64-bit)
    only appeared in some 64-bit Unix systems (UNICOS Cray Unix)
    https://en.wikipedia.org/wiki/UNICOS
*/

#if _WIN32 || _WIN64
#if _WIN64
    std::cout << "Current system is Win64\n";
#else
    std::cout << "Current system is Win32\n";
#endif
#endif

    std::cout
        << "Size of short, int, long and pointer on the system: \n"
        << "sizeof(short) = " << sizeof(short)
        << "sizeof(int) = " << sizeof(int)
        << "sizeof(long) = " << sizeof(long)
        << "sizeof(void*) = " << sizeof(void*)
        << '\n';
}

#pragma endregion

#pragma region 04.unsigned_types

void unsigned_types()
{
    // Signed is the default if 'unsigned' omitted
    // Unsigned effectively "move" bottom of the numerical range to 0
    // Signed long: [-2147483648; 2147483647]
    // Unsigned long: [0; 4294967295]

    // Signed bitwise representation
    // It has a sign bit in it
    long signed_longs[] = { 1, 255, 2147483647, -1 };
    for (auto sig : signed_longs) {
        std::cout
            << "sig = " << sig << " bitwise(sig) = " << bitwise(sig)
            << '\n';
    }

    // Unsigned bitwise representation
    unsigned long unsigned_longs[] = { 1, 255, 2147483647, 4294967295 };
    for (auto unsig : unsigned_longs) {
        std::cout
            << "unsig = " << unsig << " bitwise(unsig) = " << bitwise(unsig)
            << '\n';
    }

    // Even though 'unsigned' allows to use new range of numbers within the same type, 
    // using unsigned during actual mathematical computations can introduce programming errors 
    // that are really hard to spot, and it's usually better to use signed int just to avoid them
    
    // 1.
    for (unsigned i = 5; i >= 0; i--) {
        std::cout << i << '\n';
    }

    // 2.
    for (unsigned i = 0; i < 10; i++) {
        for (unsigned j = 0; j < 10; j++) {
            if (i - j >= 4) {
                std::cout
                    << "i = " << i
                    << "; "
                    << "j = " << j
                    << '\n';
            }
        }
    }

    // 3. (this one is being caught with the compiler warning)
    unsigned int i1 = 1;
    int j1 = -1;

    // i1 == 1, j1 == -1, arithmetically it's bigger, right?
    // In Soviet C++ unsigned type cast you!
    if (j1 < i1) {
        std::cout << "Unreachable;";
    }

    // Unreachable section will never be reached, 
    // because unsigned(-1) is being casted to maxint == 2147483647 == 0xffffffff
    // Using unsigned introduces the potential for these sorts of bugs, and there's not really any upside

    // Historically, unsigned is being used for size, offset, bitmask and other values of non-negative nature
    // Note: as with all type specifiers, any order is permitted: 
    // 'unsigned long long' int and 'long int unsigned long' name the same type
    unsigned long long unsigned_ll{};
    long int unsigned long uint_ll{};
    static_assert(sizeof(unsigned_ll) == sizeof(uint_ll), "unsigned_ll and uint_ll are not the same size");
}

#pragma endregion

#pragma region 05.numeric_promotion

namespace cpp {
enum MyEnum : long long
{
    First = 1,
    Second  = 2,
    Total = 3
};
} // namespace cpp

void numeric_promotions()
{
    // https://en.cppreference.com/w/cpp/language/implicit_conversion
    // Numeric types could be implicitly converted to larger type.
    // Such conversion called promotion.
    // Promotion never changes the value of converted number

    // signed char or signed short can be converted to int;
    signed char sc = -127;
    short ss = -32768;
    int promote_to_int = sc;
    promote_to_int = ss;

    // unsigned char or unsigned short can be converted to unsigned int
    unsigned char uc = 0xff;
    unsigned short us = 0xffff;
    unsigned int promote_to_uint = sc;

    // wchar_t, char16_t, and char32_t, or any unscoped enumeration type can be converted 
    // to the first type from the following list able to hold their entire value range:
    // int, unsigned int, long, unsigned long, long long, unsigned long long

    char32_t c32 = U'\U0001e36d';
    promote_to_int = c32;
    promote_to_uint = c32;
    // clang-tidy Promoting from char32_t to long is implementation-defined
    long promote_to_long = c32;
    unsigned long promote_to_ulong = c32;
    long long promote_to_ll = c32;
    unsigned long long promote_to_ull = c32;

    // unscoped enumeration type whose underlying type is fixed can be converted to its underlying type
    cpp::MyEnum e = cpp::First;
    long long promote_to_underlying = e;

    // type bool can be converted to int with the value false becoming 0 and true becoming 1
    int weird_promotion = true;

    // all other conversions are not promotions, but rather conversions
    // for example, overload resolution chooses char -> int (promotion) over char -> short (conversion)
}

#pragma endregion

#pragma region 06.numeric_conversions

void numeric_conversions()
{
    // Unlike the promotions, numeric conversions may change the values, with potential loss of precision
    
    // Narrowing conversion - from larger to smaller
    
    // warning: conversion with possible loss of data, even 1 is perfectly fit
    long long wide_ll = 1;

    // Clang-tidy: narrowing conversion from long long to int is implementation-defined
    long narrow_l = wide_ll;

    // When signed integer arithmetic operation overflows (the result does not fit), the behavior is undefined:
    // it may cut value, it may trap on some platforms or due to compiler options (e.g. -ftrapv in GCC and Clang), 
    // it can sometimes saturate to minimal or maximal value (on many DSPs), 
    // or may be completely optimized out by the compiler

    // floating-point narrowing conversion loses precision, implementation-defined
    // If narrow type can't represent wide type, it is UB
    float narrow_f = 1.5f;
    double wide_d = narrow_f;

    // fp -> integer leads to truncation
    int narrow_i = narrow_f;
}

#pragma endregion

#pragma region 07.fixed_size_types

void fixed_size_types()
{
    // Problem of C++ borrowed from C: we have lots of types 
    // whose sizes are all depend on system or architecture
    // That's why fixed-size (or guaranteed size) types are provided

    // signed integer type with width of exactly 8, 16, 32 and 64 bits respectively
    // with no padding bits and using 2's complement for negative values
    int8_t i8{};
    int16_t i16{};
    int32_t i32{};
    int64_t i64{};
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
    int_fast8_t if8{};
    int_fast16_t if16{};
    int_fast32_t if32{};
    int_fast64_t if64{};

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
    int_least8_t li8{};
    int_least16_t li16{};
    int_least32_t li32{};
    int_least64_t li64{};

    std::cout
        << "sizeof(int_least8_t) = " << sizeof(int_least8_t)
        << "sizeof(int_least16_t) = " << sizeof(int_least16_t)
        << "sizeof(int_least32_t) = " << sizeof(int_least32_t)
        << "sizeof(int_least64_t) = " << sizeof(int_least64_t)
        << '\n';

    // (un)signed integer type capable of holding a pointer
    // intptr_t can't be assigned a nullptr, it's just an int size of pointer
    intptr_t iptr = 0;

    // maximum-width (un)signed integer type
    intmax_t imax{};

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

#pragma endregion


int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <function>\n";
        return 1;
    }
    std::string func = argv[1];

    if (func == "boolean_type") {
        boolean_type();
    }
    else if(func == "character_type") {
        character_types();
    }
    else if(func == "integer_types") {
        integer_types();
    }
    else if(func == "unsigned_types") {
        unsigned_types();
    }
    else if (func == "numeric_promotions") {
        numeric_promotions();
    }
    else if (func == "numeric_conversions") {
        numeric_conversions();
    }
    else if (func == "fixed_size_types") {
        fixed_size_types();
    }
    else {
        std::cerr << "Unknown function: " << func << '\n';
        return 1;
    }

    return 0;
}
