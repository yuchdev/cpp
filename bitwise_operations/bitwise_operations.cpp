#include <iostream>
#include <algorithm>
#include <bitset>

// Bitwise operations

/// Returns bitset with bitwise representation of 'value'
template <typename T>
std::bitset<sizeof(T) * 8> bitwise(T value)
{
    constexpr size_t bits_size = sizeof(T) * 8;
    return std::bitset<bits_size>(value);
}

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

//////////////////////////////////////////////////////////////////////////
// Using bitwise arithmetic for mathematical operations

// Positive integer representation more or less clear
// Sign bit is 0, rest 31 bits are binary representation
// Is the negative integer sign bit is 1, and the rest is BINARY REVERSE representation
void negative_integer()
{
    long a = 1;
    std::cout << "a(2) = " << bitwise(a) << '\n';

    long b = -1;
    std::cout << "b(2) = " << bitwise(b) << '\n';
}

// Therefore we can "emulate" unary arithmetical minus
int my_negate(long i)
{
    return ~(i - 1);
}

void shift_operations()
{
    int a = 1;
    std::cout << "a(10) = " << a << '\t' << "a(2) = " << bitwise(a) << '\n';

    a = a << 1;
    std::cout << "a(10) = " << a << '\t' << "a(2) = " << bitwise(a) << '\n';

    a = a << 1;
    std::cout << "a(10) = " << a << '\t' << "a(2) = " << bitwise(a) << '\n';

    // Every left shift operation is multiplication by 2

    int b = 1024;
    std::cout << "b(10) = " << b << '\t' << "b(2) = " << bitwise(b) << '\n';

    b = b >> 1;
    std::cout << "b(10) = " << b << '\t' << "b(2) = " << bitwise(b) << '\n';

    b = b >> 1;
    std::cout << "b(10) = " << b << '\t' << "b(2) = " << bitwise(b) << '\n';

    // Every right shift operation is division by 2
}


// Emulating divide by 2 and multiple by 2
int divide_2(int i)
{
    return i >> 1;
}

int multiply_2(int i)
{
    return i << 1;
}

// In the previous example you may notice all integers are powers of 2, 
// and their bitwise representation has exactly one bit
// Let's check it
void powers_of_2()
{
    long a = 1;
    for (size_t i = 0; i < 16; ++i) {
        a *= 2;
        std::cout << "a(10) = " << a << '\t' << "a(2) = " << bitwise(a) << '\n';
    }
}

bool raised_to2(int x)
{
    if (!((x - 1) & x))
        return true;
    return false;
}

void raised_to2_examples_and_explanation()
{
    // Let's try with some numbers
    long numbers[] = { 2, 64, 1024, 25, 120 };
    std::for_each(std::begin(numbers), std::end(numbers), [](long val) {
        if (raised_to2(val))
            std::cout << val << " is a power of 2\n";
        else
            std::cout << val << " is not a power of 2\n";
    });

    // Explanation?
    long x1 = 64;
    std::cout << "x = \t\t" << bitwise(x1) << '\n';
    std::cout << "(x - 1) = \t" << bitwise(x1 - 1) << '\n';
    std::cout << "(x - 1) & x = \t" << bitwise((x1 - 1) & x1) << '\n';

    // x and (x - 1) always occupy non-intersecting bits, if the integer is a power of 2

    long x2 = 82;
    std::cout << "x = \t\t" << bitwise(x2) << '\n';
    std::cout << "(x - 1) = \t" << bitwise(x2 - 1) << '\n';
    std::cout << "(x - 1) & x = \t" << bitwise((x2 - 1) & x2) << '\n';
}


int main()
{
    raised_to2_examples_and_explanation();
    return 0;
}