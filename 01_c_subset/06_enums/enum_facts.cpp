#include <iostream>

// Important agreement about types size in C++:
// 1 = sizeof(char) <= sizeof(short) <= sizeof(int) <= sizeof(long)
// 1 <= sizeof(bool) <= sizeof(long)
// sizeof(char) <= sizeof(wchar_t) <= sizeof(long)
// sizeof(float) <= sizeof(double) <= sizeof(long double)
// sizeof(N) <= sizeof(signed N) <= sizeof(unsigned N)

// Suppress counter-example of enum overflow
#ifdef _MSC_VER
#pragma warning( disable : 4309)
#endif

// What is the size of enum

enum enum1 { dark, light };		// 0:1
enum enum2 { a = 3, b = 9 };		// 0:15
enum enum3 { c = 3, d = 900 };	// 0:1024
enum enum4 { e = 3, f = 5000000000 };	// out of range 0:2^32, behaves like integer, gives warning

void enum_size()
{
    enum1 e1 = enum1(0);
    enum1 e2 = enum1(1);

    // UB
    enum1 e3 = enum1(20);

    // enum size is platform-dependent
    size_t sz = sizeof(enum1);
    sz = sizeof(enum2);
    sz = sizeof(enum3);

    // out if range
    enum4 e4 = enum4::f;
    sz = sizeof(e4);
    long long l = e4;
}

void show_enum_classes()
{
    // The underlying type must be one of the signed or unsigned integer types (6.2.4)
    // the default is int. We could be explicit about that:

    enum class warning1 : long { green, yellow, red };

    enum class warning2 : unsigned char { green, yellow, red };

    long l = static_cast<long>(warning1::green);
    unsigned char c = static_cast<unsigned char>(warning2::green);

    //
    enum plain : long { plain1, plain2, plain3 };
    plain p = plain1;

    // Todo plain and class name resolution

    // An enum is a user-defined type, so we can define the | and & operators

    // A plain enum can be unnamed, e.g.:
    enum { arrow_up = 1, arrow_down, arrow_sideways };
}

// Print enum as int
enum class warning { green, yellow, red };

void print_warning(warning w)
{
    std::cout << static_cast<int>(w) << '\n';
}

int main()
{
    enum_size();
    enum_class();
    return 0;
}
