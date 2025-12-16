// ReSharper disable All
#include <iostream>
#include <cstddef> // for std::byte

// If less than C++20
#if (__cplusplus <= 201703L)
using char8_t = unsigned char;
#endif

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
        << "; signed char c2 = " << c2
        << "; unsigned char c3 = " << c3
        << '\n';

    // C++17 introduced `std::byte` to represent raw memory without arithmetic.
    std::byte b1 = std::byte { 0b00001111 };

    // The functional cast notation int{c} gives the integer value for a character c
    std::cout << c1 << " == " << int { c1 } << '\n';

    // When embedding a numeric constant in a string using the octal notation,
    // it is wise always to use three digits for the number

    // 6 chars: 'a' '\xa' 'h' '\12' '9' '\0'
    char v1[] = "a\xah\129";

    // 5 chars: 'a' '\xa' 'h' '\127' '\0'
    char v2[] = "a\xah\127";

    // 4 chars: 'a' '\xad' '\127' '\0'
    char v3[] = "a\xad\127";

    // 5 chars: 'a' '\xad' '\012' '7' '\0'
    char v4[] = "a\xad\0127";

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
    // 16-bit Unicode symbols hold most of alphabetical and pseudographical symbols
    // 32-bit Unicode symbol may hold emoji and other extended symbols
    // See also https://en.wikipedia.org/wiki/UTF-32

    // type for UTF-8 character representation; Same size as unsigned char
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


int main(int argc, char* argv[])
{
    character_types();
    return 0;
}
