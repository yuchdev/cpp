#define _USE_MATH_DEFINES
#include <iostream>
#include <iomanip>
#include <cmath>

// OsX workaround
#include <cfloat>
#include <cstdint>

#include <bitwise.h>

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
template <typename FloatingPoint, typename Bitwise>
struct floating_point_traits {};

template <>
struct floating_point_traits<float, long> {
    static constexpr size_t significand = 23;
    static constexpr size_t exponent = 31;
    static constexpr long minus_one = 0x7FFFFF;
    static constexpr long sign_mask = 0x800000;
};

template <>
struct floating_point_traits<double, long long> {
    static constexpr size_t significand = 52;
    static constexpr size_t exponent = 63;
    static constexpr long long minus_one = 0x7FFFFFFFFFFF;
    static constexpr long long sign_mask = 0x800000000000;
};

template <typename FloatingPoint, typename Bitwise>
void extract_fp_components(FloatingPoint val)
{

    static constexpr size_t exponent = floating_point_traits<FloatingPoint, Bitwise>::exponent;
    static constexpr size_t significand = floating_point_traits<FloatingPoint, Bitwise>::significand;
    static constexpr size_t minus_one = floating_point_traits<FloatingPoint, Bitwise>::minus_one;
    static constexpr size_t sign_mask = floating_point_traits<FloatingPoint, Bitwise>::sign_mask;

    union
    {
        FloatingPoint floating_point_repr;
        Bitwise integer_repr;
    } f;
    f.floating_point_repr = val;

    // TODO: warning C4293: '>>': shift count negative or too big, undefined behavior
    int s = (f.dw >> exponent) ? -1 : 1;
    // TODO: warning C4293: '>>': shift count negative or too big, undefined behavior
    int e = (f.dw >> significand) & 0xFF;
    int m =
        e ?
        (f.integer_repr & minus_one) | sign_mask :
        (f.integer_repr & minus_one) << 1;

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
    // TODO: insert any function call
    return 0;
}