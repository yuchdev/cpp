#include <utilities/defines.h>
SUPPRESS_PRAGMA_WARNINGS()

// ifdef clang
#ifdef __clang__

#endif

#include <iostream>

// OsX workaround
#ifdef __APPLE__
#define _USE_MATH_DEFINES
#include <cmath>
#include <cfloat>
#include <cstdint>
#endif

#include <utilities/bitwise.h>

// C++20 updates
#if __cplusplus >= 202002L
#include <format>
#endif

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
    // Single-precision format is a computer number format, usually occupying 32 bits in computer memory
    // Sign bit: 1 bit
    // Exponent width: 8 bits
    // Significand precision: 24 bits (23 explicitly stored)
    float float_numbers[] = {1.0, 1.5, 0.75};

#if __cplusplus >= 202002L

    std::format("Sizeof float {}", sizeof(float));
    std::format("Sizeof long {}", sizeof(long));
    std::format("Sizeof long* {}", sizeof(long*));

#endif

    for (auto float_number : float_numbers) {

        long* float_hack = reinterpret_cast<long*>(&float_number);
        static_assert(sizeof(float_number) == sizeof(*float_hack), "Float and long should have equal size");
        std::cout << "Binary representation of " << float_number << " =\n\t " << *float_hack
            << " =\n\t " << bitwise(*float_hack) << '\n';
    }

    // Double precision is a computer number format, usually occupying 64 bits in computer memory
    // Sign bit: 1 bit
    // Exponent: 11 bits
    // Significand precision: 53 bits (52 explicitly stored)
    double double_numbers[] = { 1.0, 1.5, 0.75 };

    for (auto double_number : double_numbers) {
        long long* double_hack = reinterpret_cast<long long*>(&double_number);
        static_assert(sizeof(double_number) == sizeof(*double_hack), "Double and long long should have equal size");

#if __cplusplus >= 202002L
        std::format("Binary representation of {} =\n\t {} =\n\t {}\n",
                    double_number, *double_hack, bitwise(*double_hack));
#endif

    }
    // Unlike integers, distribution of floating-point numbers is not uniform
    // It is denser for smaller and dense for larger numbers
    // Using floating-point numbers is a constant trade-off between range and precision
    
    // Epsilon is a floating-point positive number, as such (1 +/- Epsilon) != 1
    // DBL_EPSILON C++ Standard library == 10 ^ -16

    // Standard library  also have several special values
    // +/- INF and NaN (Not a number)
    // Two kinds of NaN: a quiet NaN (qNaN) and a signaling NaN (sNaN)

    // Numerical computing solutions like Maxima or Mathematica 
    // often "knows" very precise representation of numbers like Pi or e
}

int main()
{
    floating_point_representation();
    return 0;
}

