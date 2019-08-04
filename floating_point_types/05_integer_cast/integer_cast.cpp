#define _USE_MATH_DEFINES
#include <iostream>
#include <iomanip>
#include <bitset>
#include <cmath>

// OsX workaround
#include <cfloat>
#include <cstdint>

// for atan2
#include <valarray>

/// Returns bitset with bitwise representation of 'value'
template <typename T>
std::bitset<sizeof(T) * 8> bitwise(T value)
{
    constexpr size_t bits_size = sizeof(T) * 8;
    return std::bitset<bits_size>(value);
}


// 1.Floating-point format
// https://en.wikipedia.org/wiki/Floating-point_arithmetic

// The term floating point (FP) refers to the fact that a number's decimal point can "float",
// in the other words, it can be placed anywhere relative to the significant digits of the number.
// For example:
// 0.1
// 0.001
// 0.00...0001
// 10000000000
// 5*10^-10; 7*10^9

// In computing, floating-point arithmetic (FP) is arithmetic 
// using formulaic representation of real numbers as an approximation 
// so as to support a trade-off between range and precision.
// Representation:
// FP_Number = Significand * Base^Exponent
// 1.2345 = 12345 * 10^-4
// All these numbers are "packed" in 32, 64 or 128 bit value
void floating_point_representation()
{
    // Single precision
    float float_numbers[] = {1.0, 1.5, 0.75};

    for (auto float_number : float_numbers) {

        long* float_hack = reinterpret_cast<long*>(&float_number);
        static_assert(sizeof(float_number) == sizeof(*float_hack), "Float and long should have equal size");
        std::cout << "Binary representation of " << float_number << " =\n\t " << *float_hack
            << " =\n\t " << bitwise(*float_hack) << '\n';
    }

    // Double precision
    double double_numbers[] = { 1.0, 1.5, 0.75 };

    for (auto double_number : double_numbers) {
        long long* double_hack = reinterpret_cast<long long*>(&double_number);
        static_assert(sizeof(double_number) == sizeof(*double_hack), "Double and long long should have equal size");

        std::cout << "Binary representation of " << double_number << " =\n\t " << *double_hack
            << " =\n\t " << bitwise(*double_hack) << '\n';
    }
}

int main()
{
    return 0;
}