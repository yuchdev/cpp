#include <iostream>
#include <cstdint>
#include <vector>

using namespace std;

/*

New features:
* Uniform and general initialization using {}-lists (2.2.2, 6.3.5)
* Type deduction from initializer: auto (2.2.2, 6.3.6.1)
* Prevention of narrowing (2.2.2, 6.3.5)
* Type and template aliases (3.4.5, 6.5, 23.6)
* Unicode characters (6.2.3.2, 7.3.2.2)
* long long integer type (6.2.4)
* Alignment controls: alignas and alignof(6.2.9)
* The ability to use the type of expression as a type in a declaration: decltype
  Important note: decltype has new behavior in C++14
* C99 features (long long)
* Integer type names, such as int16_t, uint32_t, and int_fast64_t (6.2.8)

Examples:
1. Behavior examples
2. Boolean type casting
3. Signed-unsigned char, escape characters, literals
4. Integer types, suffixes, C compatible types
5. Floating-point suffixes
6. Sizes and alignment
7. Declaration and decltype, -> suffix (see ch.12)
8. Scopes (text only)
7. Initialization, auto and {}
8. Type aliases (see Ch.05 for examples)
*/

void show_bool()
{
    bool b1 = true;
    bool b2 { false };

    // narrowing
    int i1 = b1;
    int i2 { b1 };

    // narrowing in expression (cast to 1)
    int i3 = 5 + b1;

    // narrowing pointers
    int* pint = nullptr;
    if (pint) {/*never be there*/ }
}

void show_chars()
{
    // Character types

    // the choice of signed or unsigned for a char is implementation-defined
    // (char might be signed or unsigned)
    char c1 = 'A';
    signed char c2 = -10;
    unsigned char c3 = 0;

    wchar_t w1 = L'A';

    // The notation int{c} gives the integer value for a character c
    int i1 { c1 };
    cout << i1 << endl;

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

    cout << v1 << endl;
    cout << v2 << endl;
    cout << v3 << endl;
    cout << v4 << endl;

    // Literals of such larger character sets are presented as sequences 
    // of four or eight hexadecimal digits preceded by a U or a u
    char16_t c5 = u'A';
    char32_t c6 = U'A';

    char cl0[] = u8"DEADBEAF";
    cout << cl0 << endl;

    char16_t cl1[] = u"FADEBEEF";
    //cout << cl1 << endl; wrong output

    char32_t cl2[] = U"DEADBEAF";
    //cout << cl2 << endl; wrond output
}


void show_int()
{
    // If you need more detailed control over integer sizes, you can use aliases from <cstdint>

    // type "exactly"
    int8_t i8 {};
    int16_t i16 {};
    int32_t i32 {};
    int64_t i64 {};

    // type "at least"
    int_least8_t il8 {};
    int_least16_t il16 {};
    int_least32_t il32 {};
    int_least64_t il64 {};

    // type "fast"
    uint_fast8_t fi8 {};
    uint_fast16_t fi16 {};
    uint_fast32_t fi32 {};
    uint_fast64_t fi64 {};
}

void show_int_float()
{
    // The reason for providing more than one integer type, more than one unsigned type, 
    // and more than one floating-point type is to allow the programmer 
    // to take advantage of hardware characteristics

    // This is what is guaranteed about sizes of fundamental types :
    // 1 = sizeof(char) <= sizeof(short) <= sizeof(int) <= sizeof(long) <= sizeof(long long)
    // 1 <= sizeof(bool) <= sizeof(long)
    // sizeof(char) <= sizeof(wchar_t) <= sizeof(long)
    // sizeof(float) <= sizeof(double) <= sizeof(long double)
    // sizeof(N) == sizeof(signed N) == sizeof(unsigned N)

    // Some implementation-defined aspects of fundamental types 
    //can be found by a simple use of sizeof, and more can be found in <limits>

    int i1 = 077;
    int i2 = 0xaa;
    long l1 = 100l;
    unsigned long l2 = 100ul;
    unsigned long long l3 = 100ull;

    float f1 = 1.f;
    //long float f2 = 1.f;
    double d1 = 1.0;
    long double d2 = 1.0;
}

void show_align()
{
    // In addition, on some machine architectures, 
    // the bytes used to hold it must have proper alignment 
    // for the hardware to access it efficiently (or in extreme cases to access it at all)

    // The alignof() operator returns the alignment of its argument expression.For example :

    // TODO: alignof

    // Sometimes, we have to use alignment in a declaration, where an expression, 
    // such as alignof(x + y) is not allowed.
    // Instead, we can use the type specifier alignas: alignas(T)means 'align just like a T'

    //TODO: alignas
}

// C++14 auto return type deduction (impossible in C++11)
template <typename T>
auto add_things(T a, T b)
{
    const auto ret = a + b;
    return ret;
}

void show_init()
{
    // Google's auto guidelines
    // Use type deduction only if it makes the code clearer to readers 
    // who aren't familiar with the project, or if it makes the code safer. 
    // Do not use it merely to avoid the inconvenience of writing an explicit type.
    // https://google.github.io/styleguide/cppguide.html#auto

    // An initializer can use one of four syntactic styles:
    int a1 { 1 };
    int a2 = { 1 };
    int a3 = 1;
    int a4(1);

    // In the cases above, the = is redundant

    // list initialization, does not allow narrowing

    // The trap is that if the initializer is a{}-list, we may not want its type deduced

    // Important note: z1 is an `initializer_list<int>` in C++11 and `int` in C++14 and further
    auto z1 { 99 };

    // z2 is an int
    auto z2 = 99;

    // The classical example is a vector of integers :
    vector<int> v1 { 99 }; // v1 is a vector of 1 element with the value 99
    vector<int> v2(99); // v2 is a vector of 99 elements each with the default value 0

    // We can decorate a deduced type with specifiers and modifiers, such as const and &
    for (const auto& x : v1) {
        cout << x << endl;
    }
}

/*
template<class T, class U>
auto operator+(const Matrix<T>& a, const Matrix<U>& b) -> Matrix<decltype(T{}+U{})>;

I use the suffix return type syntax to be able to express the return type in terms of the arguments

Important note: decltype has new behavior in C++14
*/


int main()
{
    show_bool();
    show_chars();
    show_int();
    show_align();
    show_init();
    return 0;
}
