#define _USE_MATH_DEFINES
#include <iostream>
#include <iomanip>
#include <cmath>

// OsX workaround
#include <cfloat>
#include <cstdint>

#include <utilities.h>


// 2."Extract" sign bit, significand and exponent
// * Before significand we always assume 1.
// * Effective exponent is exponent_value-127
void extract_fp_components(float val)
{
    static_assert(sizeof(long) == sizeof(float), "sizeof(long) should be equal sizeof(float)");

    union
    {
        float floating_number;
        long bitwise_representation;
    } float_bits;

    float_bits.floating_number = val;

    // extract sign bit
    long sign_bit = (float_bits.bitwise_representation >> 31) ? -1 : 1;

    // extract exponent
    long exponent = (float_bits.bitwise_representation >> 23) & 0xFF;

    // extract significand
    long significand = float_bits.bitwise_representation & 0x7FFFFF;

    std::cout << "Significand binary representation = " << bitwise(significand) << '\n';

    int m =
        exponent ?
        significand | 0x800000 :
        significand << 1;

    double m1 = double(m) / pow(2., 23.);

    // exponent shift
    exponent -= 127;

    std::cout << "s = " << sign_bit << "; e =  " << exponent << "; m(2) = " << m << "; m(10) = " << m1 << '\n';
}

// 1.0 = 0 01111111 00000000000000000000000
// E = 01111111 = 127 - 127 = 0
// 1.0 = (-1)^s * 1.M * 2^E = (1-)^0 * 1.000 * 2^0 = 1.0

//       s E        M
// 1.5 = 0 01111111 10000000000000000000000
// E = 01111111 = 127 - 127 = 0
// 1.1(2) = 2^0 + 2^(-1) = 1 + 1/2 = 1.5
// 1.5 = (-1)^s * 1.M * 2^E = (1-)^0 * 1.1(2) * 2^0 = 1.5


// Let's create generic, template-based floating-point components extractor
template <typename T>
struct floating_point_traits {};

template <>
struct floating_point_traits<float> {
    static constexpr size_t significand = 23;
    static constexpr size_t exponent = 31;
};

template <>
struct floating_point_traits<double> {
    static constexpr size_t significand = 52;
    static constexpr size_t exponent = 63;
};

template <typename T>
void extract_fp_components(T val)
{

    static constexpr size_t exponent = floating_point_traits<T>::exponent;
    static constexpr size_t significand = floating_point_traits<T>::significand;

    union
    {
        T fl;
        long dw;
    } f;
    f.fl = val;
    int s = (f.dw >> exponent) ? -1 : 1;
    int e = (f.dw >> significand) & 0xFF;
    int m =
        e ?
        (f.dw & 0x7FFFFF) | 0x800000 :
        (f.dw & 0x7FFFFF) << 1;

    e -= 127;
    std::cout << "sign = " << s
        << " mantissa = " << m
        << " exponent = " << e << '\n';
}

void fp_components_extract()
{
    extract_fp_components(1.0);
    extract_fp_components(1.5);
    extract_fp_components(-1.0);
}

int main()
{
    return 0;
}