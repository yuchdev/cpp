#include <iostream>

#include <utilities/bitwise.h>

// warning C4018: '<': signed/unsigned mismatch as a counter-example
#ifdef _MSC_VER
#pragma warning( disable : 4018)
#endif

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
    for (auto no_sign : unsigned_longs) {
        std::cout
            << "no_sign = " << no_sign << " bitwise(no_sign) = " << bitwise(no_sign)
            << '\n';
    }

    // Even though 'unsigned' allows to use new range of numbers within the same type,
    // using unsigned during actual mathematical computations can introduce programming errors
    // that are really hard to spot, and it's usually better to use signed int just to avoid them

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
    unsigned long long unsigned_ll {};
    long int unsigned long uint_ll {};
    static_assert(sizeof(unsigned_ll) == sizeof(uint_ll), "unsigned_ll and uint_ll are not the same size");
}

// Unexpected type cast
void endless_loop()
{
    // This loop is endless
    for (unsigned i = 5; i >= 0; i--) {
        std::cout << i << '\n';
    }
}

// Be careful with unsigned integers, I mean, really
void comparing_unsigned()
{
    unsigned int i1 = 1;
    int j1 = -1;

    // i1 == 1, j1 == -1, arithmetically it's bigger, right?
    // In Soviet C++ unsigned type cast you!
    if (j1 < i1) {
        std::cout << "Unreachable;";
    }

    // Unreachable section will never be reached, 
    // because unsigned(-1) being casted to maxint == 2147483647 == 0xffffffff
}

// This cast was really unexpected
// TODO: some explanation
void show_shift()
{

    // Let's have, for example, b10(1075) == b2(10000110011)
    // And move it right as many times how many bits in the integer type

    // In our case it's going to be 64
    long long l = 0;

    // 1st shift it's going to be b2(100001100110)
    // 2nd b2(1000011001100)
    // 3rd b2(10000110011000)
    // Let's see how it works...

    std::cout << "Wanna see some type magic?\n";
    for (size_t i = 0; i < sizeof(long long) * 8; ++i) {
        l = 1075 << i;
        std::cout << "Step " << i << " magic =  " << bitwise(l) << '\n';
    }

    l = 0;
    for (size_t i = 0; i < sizeof(long long) * 8; ++i) {
        l = 1075LL << i;
        std::cout << "Step " << i << " magic =  " << bitwise(l) << '\n';
    }
}

int main()
{
    // TODO: insert any function call
    return 0;
}
