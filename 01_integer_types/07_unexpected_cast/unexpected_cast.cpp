#include <iostream>

#include <bitwise.h>

// warning C4018: '<': signed/unsigned mismatch as a counter-example
#ifdef _MSC_VER
#pragma warning( disable : 4018)
#endif

// Unexpected type cast

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
void show_shift(){
    
    // Let's have, for example, b10(1075) == b2(10000110011)
    // And move it right as many times how many bits in the integer type

    // In our case it's going to be 64
    long long l = 0;

    // 1st shift it's going to be b2(100001100110)
    // 2nd b2(1000011001100)
    // 3rd b2(10000110011000)
    // Let's see how it works...

    std::cout << "Wanna see some type magic?\n";
    for (size_t i = 0; i < sizeof(long long)* 8; ++i){
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
