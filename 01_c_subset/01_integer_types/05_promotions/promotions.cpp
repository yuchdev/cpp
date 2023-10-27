#include <utilities/defines.h>
SUPPRESS_PRAGMA_WARNINGS()

#include <utilities/bitwise.h>
#include <utilities/defines.h>
SUPPRESS_UNSIGNED_COUNTEREXAMPLE_WARNINGS()

#include <iostream>
#include <cstdint>

// Fundamental C++ types
// https://en.cppreference.com/w/cpp/language/types
namespace cpp
{
    enum MyEnum : long long
    {
        First = 1,
        Second = 2,
        Total = 3
    };
} // namespace cpp

void numeric_promotions()
{
    // https://en.cppreference.com/w/cpp/language/implicit_conversion
    // Numeric types could be implicitly converted to larger type
    // Such conversion called promotion
    // Promotion never changes the value of converted number

    // Integer Promotion in C++ is pretty much a legacy of ANSI C

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

int main(int argc, char* argv[])
{
    numeric_promotions();
    return 0;
}
