// ReSharper disable All
#include <iostream>
#include <cstddef> // For offsetof

/**
 * For hardware to access data efficiently, the bytes holding it must have proper alignment.
 * For example, an integer might need to be at an even address, or a double at an address divisible by 8.
 */
void show_align()
{
    // The `alignof()` operator returns the alignment requirement of its argument (a type or an object).
    // The value returned is the number of bytes between successive addresses
    // where an object of this type can be allocated.
    std::cout << "--- alignof ---" << std::endl;
    std::cout << "alignof(char): " << alignof(char) << std::endl;
    std::cout << "alignof(int): " << alignof(int) << std::endl;
    std::cout << "alignof(double): " << alignof(double) << std::endl;

    struct MyStruct {
        char c;   // 1 byte
        int i;    // 4 bytes
        double d; // 8 bytes
    };

    // The alignment of a struct is the largest alignment of any of its members.
    // In MyStruct, the largest alignment is that of double (8 bytes).
    std::cout << "alignof(MyStruct): " << alignof(MyStruct) << std::endl;

    // The compiler may add padding between members to ensure proper alignment.
    std::cout << "sizeof(MyStruct): " << sizeof(MyStruct) << std::endl; // Likely 16, not 1+4+8=13

    // Let's find a place of each member using offsetof
    std::cout << "offsetof(MyStruct, c): " << offsetof(MyStruct, c) << std::endl;
    std::cout << "offsetof(MyStruct, i): " << offsetof(MyStruct, i) << std::endl; // Padding is added before 'i'
    std::cout << "offsetof(MyStruct, d): " << offsetof(MyStruct, d) << std::endl;

    // Changing the order of members can affect the size due to padding.
    struct MyStruct2 {
        double d; // 8 bytes
        int i;    // 4 bytes
        char c;   // 1 byte
    };

    // Now the alignment is still that of double (8 bytes),
    // but the size may be smaller due to reduced padding.
    std::cout << "alignof(MyStruct2): " << alignof(MyStruct) << std::endl;

    // The `alignas` specifier enforces a specific alignment for a variable or a type.
    // This is useful for optimization or for hardware interface requirements.
    std::cout << std::endl;
    std::cout << "--- alignas ---" << std::endl;

    // `alignas(T)` makes the struct have the same alignment as type T.
    struct alignas(double) AlignedLikeDouble {
        int i;
    };
    std::cout << "alignof(AlignedLikeDouble) is " << alignof(AlignedLikeDouble)
              << ", same as alignof(double) which is " << alignof(double) << std::endl;

    // `alignas` can also take a constant integer expression (must be a power of 2).
    // This requests a stronger alignment than the type would naturally have.
    struct alignas(32) StricterAlignedStruct {
        int i;
        char c;
    };
    std::cout << "alignof(StricterAlignedStruct): " << alignof(StricterAlignedStruct) << std::endl; // Will be 32
    std::cout << "sizeof(StricterAlignedStruct): " << sizeof(StricterAlignedStruct) << std::endl;   // Also a multiple of 32

    // Still, offsets are not that different from defaults, as members are laid out in order.
    std::cout << "offsetof(StricterAlignedStruct, i): " << offsetof(StricterAlignedStruct, i) << std::endl;
    std::cout << "offsetof(StricterAlignedStruct, c): " << offsetof(StricterAlignedStruct, c) << std::endl;

    // You can also apply `alignas` to individual members of a struct.
    struct ColorVector {
      // align member 'r' to a 16-byte boundary, useful for SIMD instructions.
      alignas(16) float r;
      alignas(16) float g;
      alignas(16) float b;
      alignas(16) float a;
    };
    // The alignment of the struct is now at least 16.
    std::cout << "alignof(ColorVector): " << alignof(ColorVector) << std::endl; // Will be 16
    std::cout << "sizeof(ColorVector): " << sizeof(ColorVector) << std::endl;   // Likely 64 bytes (4 floats * 16-byte alignment)
    std::cout << "offsetof(ColorVector, r): " << offsetof(ColorVector, r) << std::endl; // 0
    std::cout << "offsetof(ColorVector, g): " << offsetof(ColorVector, g) << std::endl; // 16
    std::cout << "offsetof(ColorVector, b): " << offsetof(ColorVector, b) << std::endl; // 32
    std::cout << "offsetof(ColorVector, a): " << offsetof(ColorVector, a) << std::endl; // 48
}

int main()
{
    show_align();
    return 0;
}
