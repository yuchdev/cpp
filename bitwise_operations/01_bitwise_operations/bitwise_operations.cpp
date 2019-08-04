#include <iostream>

#include <utilities.h>

//////////////////////////////////////////////////////////////////////////
// Bitwise operations

// Bitwise AND:
// 1 if both bits are 1; 0 otherwise
// 1 & 1 == 1
// 1 & 0 == 0
// 0 & 1 == 0
// 0 & 0 == 0
// Bitwise AND is usually applied for checking if some particular bit is set
void bitwise_and()
{
    // 50 base 10 = 110010 base 2
    long b = 50;

    // check if 2nd bit is set
    if (b & 0x02) {
        std::cout << "Fourth bit is set\n";
    }
    else {
        std::cout << "Fourth bit is clear\n";
    }
        

    std::cout << "b(10) = " << b << '\t' << "b(2) = " << bitwise(b) << '\n';
}

// Bitwise OR:
// 1 if at least one bit is 1; 0 otherwise
// 1 | 1 == 1
// 1 | 0 == 1
// 0 | 1 == 1
// 0 | 0 == 0
// This operator could be used for setting some particular bit to 1
void bitwise_or()
{
    long b = 0x01;
    std::cout << "b(10) = " << b << '\t' << "b(2) = " << bitwise(b) << '\n';
    
    // Set 3rd bit to 1
    long c = b | 0x04;
    std::cout << "c(10) = " << c << '\t' << "c(2) = " << bitwise(c) << '\n';
}

// Bitwise Exclusive-Or, or XOR:
// 1 if both bits are different, and 0 it they are the same
// 1 ^ 1 == 0
// 1 ^ 0 == 1
// 0 ^ 1 == 1
// 0 ^ 0 == 0
// You can switch any bit using XOR; 
// if it was 1 and you pass 1, it becomes 0
// if it was 0 and you pass 1, it becomes 1
void bitwise_xor()
{
    long b = 0xff;
    std::cout << "b(10) = " << b << '\t' << "b(2) = " << bitwise(b) << '\n';
    
    // Switch 2nd bit
    b = b ^ 0x02;
    std::cout << "b(10) = " << b << '\t' << "b(2) = " << bitwise(b) << '\n';

    // Switch 3nd bit
    b = b ^ 0x04;
    std::cout << "b(10) = " << b << '\t' << "b(2) = " << bitwise(b) << '\n';
}

// Bitwise NOT
// Switch all bits to opposite
void bitwise_negate()
{
    // Let's say, we want to set all bits to 1, except 1st and 2nd (0x03)
    long b = ~0x03;
    std::cout << "b(10) = " << b << '\t' << "b(2) = " << bitwise(b) << '\n';
}


int main()
{
    bitwise_and();
    bitwise_or();
    bitwise_xor();
    bitwise_negate();
    return 0;
}