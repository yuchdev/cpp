// ReSharper disable All
#include <utilities/defines.h>
#include <utilities/bitwise.h>
SUPPRESS_PRAGMA_WARNINGS()

#include <iostream>

// Fundamental C++ types
// https://en.cppreference.com/w/cpp/language/types

// If less than C++20
#if (__cplusplus <= 201703L)
using char8_t = unsigned char;
#endif


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

    // Let's use variable of type `unsigned long long` and pointer to unsigned char
    unsigned long long little_endian = 0x8899aabbccddeeff;
    unsigned char* p_byte = reinterpret_cast<unsigned char*>(&little_endian); // NOLINT(modernize-use-auto)
    for (size_t i = 0; i < sizeof(unsigned long long); ++i, ++p_byte) {
        std::cout << "Byte " << i << " = 0x" << std::hex << static_cast<unsigned int>(*p_byte) << '\n';
    }

    // Look once again on bitwise representation
    std::cout << "bitwise(little_endian) = " << bitwise(little_endian) << '\n';

    // Purpose of the little-endian popularity is that LE system can read from memory 
    // at different lengths without using different addresses
    // For example, a 64-bit memory location with content [4A 00 00 00 00 00 00 00] 
    // can be read at the same address as either 8-bit (value = 4A), 16-bit (004A), or 32-bit (0000004A)
    union
    {
        uint8_t u8;
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;
    } u = { .u64 = 0x4A };

    std::cout << "8-bit value = 0x" << unsigned{u.u8} << '\n';
    std::cout << "16-bit value = 0x" << u.u16 << '\n';
    std::cout << "32-bit value = 0x" << u.u32 << '\n';
    std::cout << "64-bit value = 0x" << u.u64 << '\n';

    // Some big-endian architectures include the IBM z/Architecture, AVR32, SPARC, and OpenRISC
    // Most of ARM processors support switching endianness, being effectively double-endian

    // Regarding the size Standard guarantees
    // 1 == sizeof(char) <= sizeof(short) <= sizeof(int) <= sizeof(long) <= sizeof(long long)

/*
    Data models

    The choices made by each implementation about the sizes of the fundamental types
    are collectively known as data model. Four data models found wide acceptance:

    1.  LP32 or 2/4/4 (int is 16-bit, long and pointer are 32-bit)
        16-bit systems, Win16 API
    2.  ILP32 or 4/4/4 (int, long, and pointer are 32-bit):
        32-bit systems
        Win32 API
        Unix and Unix-like systems (32-bit Linux, macOS)
    3.  LLP64 or 4/4/8 (int and long are 32-bit, pointer is 64-bit)
        64-bit systems
        Win64 API
    4.  LP64 or 4/8/8 (int is 32-bit, long and pointer are 64-bit)
        64-bit systems
        Unix and Unix-like systems (64-bit Linux, macOS)
    5.  Other models are very rare. For example, ILP64 (8/8/8: int, long, and pointer are 64-bit)
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
        << "sizeof(short) = " << sizeof(short) << '\n'
        << "sizeof(int) = " << sizeof(int) << '\n'
        << "sizeof(long) = " << sizeof(long) << '\n'
        << "sizeof(void*) = " << sizeof(void*) << '\n'
        << '\n';
}


void brace_init()
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

    // {}-initialization, does not allow narrowing

    // The trap is that if the initializer is a {}-list, we may not want its type deduced

    // Important note: z1 is an `initializer_list<int>` in C++11 and `int` in C++17 and further
    auto z1 { 99 };

    // z2 is an int
    auto z2 = 99;

    //
    auto z3{99}; // C++17 and later: z3 is an int

    std::cout << std::dec << "z1 = " << z1 << "; z2 = " << z2 << "; z3 = " << z3 << '\n';

    // The classical example is a vector of integers :
    std::vector<int> v1 { 99 }; // v1 is a vector of 1 element with the value 99
    std::vector<int> v2(99); // v2 is a vector of 99 elements each with the default value 0

    // We can decorate a deduced type with specifiers and modifiers, such as const and &
    for (const auto& x : v1) {
        std::cout << x << std::endl;
    }
}

int main()
{
    integer_types();
    brace_init();
    return 0;
}
