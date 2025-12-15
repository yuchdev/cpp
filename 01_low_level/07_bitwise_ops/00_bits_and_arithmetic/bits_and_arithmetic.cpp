#include <iostream>

#include <utilities/bitwise.h>


// Execution order inside the line is not defined (v[i] = i++; - UB).
// TODO: Operators are something to overload. The rest are statements

// Bitwise operation review
void bitwise_ops()
{
    // Bitwise AND is usually applied for checking if some particular bit is set
    {
        long b = 50;
        std::cout << "b(10) = " << b << '\t' << "b(2) = " << bitwise(b) << '\n';
        if (b & 0x02) {
            std::cout << "Fourth bit is set\n";
        }
    }

    // OR could be useful to set particular bit
    // Ex: set 3rd bit to 1
    {
        long b = 0x01;
        b = b | 0x04;
        std::cout << "b(10) = " << b << '\t' << "b(2) = " << bitwise(b) << '\n';
    }

    // You can switch any bit using XOR; 
    // if it was 1 and you pass 1, it becomes 0
    // if it was 0 and you pass 1, it becomes 1

    // Ex: switch 2nd bit
    {
        long b = 0xff;
        std::cout << "b(10) = " << b << '\t' << "b(2) = " << bitwise(b) << '\n';
        b = b ^ 0x02;
        std::cout << "b(10) = " << b << '\t' << "b(2) = " << bitwise(b) << '\n';
    }

    // Bitwise NOT
    // Let's say, we want to set all bits to 1, except 1st and 2nd (0x03)
    {
        long b = ~0x03;
        std::cout << "b(10) = " << b << '\t' << "b(2) = " << bitwise(b) << '\n';
    }
}

// TODO:
// !(!a && !b)
//a || b
//This is correct.  NOT (NOT A  AND  NOT B) equals A OR B.  Conversely, NOT (NOT A  OR  NOT B) equals A AND B.


// Using bitwise operations for arithmetic for mathematical operations

// Positive integer representation more or less clear
// Sign bit is 0, rest 31 bits are binary representation
// Is the negative integer sign bit is 1, and the rest is BINARY REVERSE representation

// Therefore we can "emulate" unary arithmetical minus
int my_negate(long i)
{
    return ~(i - 1);
}

void negative_integer()
{
    long a = 1;
    std::cout << "a(2) = " << bitwise(a) << '\n';

    long b = -1;
    std::cout << "b(2) = " << bitwise(b) << '\n';

    std::cout << my_negate(1) << '\n';
    std::cout << my_negate(-1) << '\n';

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

void multiply_and_divide()
{
    std::cout << multiply_2(10) << '\n';
    std::cout << divide_2(50) << '\n';
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

void raised_to2_explanation()
{
    // Let's try with some numbers
    long numbers[] = { 2, 64, 1024, 25, 120 };
    for (auto val : numbers) {
        if (raised_to2(val))
            std::cout << val << " is a power of 2\n";
        else
            std::cout << val << " is not a power of 2\n";

    }

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

// !(!a && !b)
//a || b
//This is correct.  NOT (NOT A  AND  NOT B) equals A OR B.  Conversely, NOT (NOT A  OR  NOT B) equals A AND B.

void show_bitwise()
{
    int* p = nullptr;
    while (p && !(*p)) ++p;
    // Here, p is not dereferenced if it is the nullptr

    // For example, one could extract the middle 16 bits of a 32 - bit int like this
    static_assert(sizeof(int) == 4, "unexpected int size");
    static_assert(sizeof(short) == 2, "unexpected short size");
}

int main()
{
    bitwise_ops();
    negative_integer();
    shift_operations();
    multiply_and_divide();
    powers_of_2();
    raised_to2_explanation();
    return 0;
}