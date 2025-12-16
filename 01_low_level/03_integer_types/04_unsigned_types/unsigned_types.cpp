// ReSharper disable All
#include "utilities/bitwise.h"

#include <iostream>
#include <thread>

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

    // Historically, unsigned is being used for size, offset, bitmask and other values of non-negative nature
    // Note: as with all type specifiers, any order is permitted:
    // 'unsigned long long' int and 'long int unsigned long' name the same type
    unsigned long long unsigned_ll {};
    long int unsigned long uint_ll {};
    static_assert(sizeof(unsigned_ll) == sizeof(uint_ll), "unsigned_ll and uint_ll are not the same size");
}

// Don't play with unsigned
void endless_loop()
{
    // Theory:
    // - 'i' is 'unsigned', whose range is [0, 2^N - 1]. Unsigned arithmetic uses modulo 2^N.
    // - The condition 'i >= 0' is a tautology for unsigned; it's always true.
    // - When 'i' is 0, 'i--' wraps to the maximum value (well-defined), so the loop is endless.
    // - Signed underflow is UB, but unsigned underflow is defined wraparound.

    // This loop is endless (demonstration of unsigned wraparound)
    for (unsigned i = 5; i >= 0; i--) {
        std::cout << i << '\n';
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    // Safer patterns if you want to count 5..0 exactly once:
    // for (int i = 5; i >= 0; --i) { ... }
    // for (unsigned i = 5; ; --i) { ...; if (i == 0) break; }
    // for (unsigned i = 5; i-- > 0; ) { ... } // prints 5..1; handle 0 separately
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
    // Using unsigned introduces the potential for these sorts of bugs, and there's not really any upside
}

// This cast was really unexpected
void show_shift()
{
    // Theory:
    // - The usual arithmetic conversions apply to shift: the left operand's (promoted) type
    //   determines both the width and the semantics of the shift.
    // - '1075' is an 'int' literal, so (1075 << i) is computed in 'int' (typically 32-bit),
    //   then the result is assigned to 'long long'. If i >= width(int) or if the result
    //   overflows 'int', the behavior is undefined (UB) for signed types.
    // - '1075LL' is a 'long long' literal, so (1075LL << i) is computed in 64-bit.
    //   Still, for signed types, shifting into/through the sign bit is UB.
    // - If you want well-defined wrap-around semantics, use an unsigned type
    //   (e.g., 'unsigned long long'); for unsigned, left shift is defined modulo 2^N
    //   as long as the shift count is < width.

    std::cout << "Wanna see some type magic?\n";

    long long l = 0;

    // 1) Shift computed in 'int' (signed, typically 32-bit), then assigned to 'long long'.
    //    This demonstrates that the operation width is taken from the left operand's type.
    //    Note: For i >= width(int) the behavior is UB; shown here for illustration only.
    std::cout << "[int  << i] (computed in 32-bit signed)\n";
    for (size_t i = 0; i < sizeof(long long) * 8; ++i) {
        l = 1075 << i; // shift happens in 'int', not in 'long long'
        std::cout << "Step " << i << " = " << bitwise(l) << '\n';
    }

    // 2) Shift computed in 'long long' (signed, typically 64-bit).
    //    Still UB once the result cannot be represented (e.g., when the sign bit is crossed).
    std::cout << "[long long << i] (computed in 64-bit signed)\n";
    l = 0;
    for (size_t i = 0; i < sizeof(long long) * 8; ++i) {
        l = 1075LL << i; // shift happens in 'long long'
        std::cout << "Step " << i << " = " << bitwise(l) << '\n';
    }

    // 3) Well-defined variant: use an unsigned type for modulo-2^N behavior.
    //    This avoids UB as long as i < width(unsigned long long).
    std::cout << "[unsigned long long << i] (computed in 64-bit unsigned, modulo 2^N)\n";
    unsigned long long ul = 0;
    for (size_t i = 0; i < sizeof(unsigned long long) * 8; ++i) {
        ul = 1075ULL << i;
        std::cout << "Step " << i << " = " << bitwise(ul) << '\n';
    }
}

int main()
{
#if 0
    endless_loop();
#endif

    unsigned_types();
    comparing_unsigned();
    show_shift();
    return 0;
}
