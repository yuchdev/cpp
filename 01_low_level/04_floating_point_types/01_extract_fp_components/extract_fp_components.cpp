#include <bit>
#include <bitset>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <type_traits>

template <typename T>
struct ieee754_traits;

template <>
struct ieee754_traits<float>
{
    using uint_type = std::uint32_t;

    static constexpr int fraction_bits = 23;
    static constexpr int exponent_bits = 8;
    static constexpr int exponent_bias = 127;

    static constexpr uint_type sign_mask = 0x80000000u;
    static constexpr uint_type exponent_mask = 0x7f800000u;
    static constexpr uint_type fraction_mask = 0x007fffffu;
    static constexpr uint_type exponent_all_ones = 0xffu;
};

template <>
struct ieee754_traits<double>
{
    using uint_type = std::uint64_t;

    static constexpr int fraction_bits = 52;
    static constexpr int exponent_bits = 11;
    static constexpr int exponent_bias = 1023;

    static constexpr uint_type sign_mask = 0x8000000000000000ull;
    static constexpr uint_type exponent_mask = 0x7ff0000000000000ull;
    static constexpr uint_type fraction_mask = 0x000fffffffffffffull;
    static constexpr uint_type exponent_all_ones = 0x7ffu;
};

template <std::floating_point T>
void extract_ieee_components(T value)
{
    using traits = ieee754_traits<T>;
    using UInt = typename traits::uint_type;

    static_assert(sizeof(T) == sizeof(UInt),
                  "This demonstration expects the matching IEEE storage width");
    static_assert(std::numeric_limits<T>::radix == 2,
                  "This decoder is specifically for binary floating point");

    const UInt bits = std::bit_cast<UInt>(value);
    const bool negative = (bits & traits::sign_mask) != 0;
    const UInt raw_exponent =
        (bits & traits::exponent_mask) >> traits::fraction_bits;
    const UInt fraction = bits & traits::fraction_mask;

    std::cout << "\nvalue = "
              << std::setprecision(std::numeric_limits<T>::max_digits10)
              << value << '\n';
    std::cout << "bits  = " << std::bitset<sizeof(UInt) * 8>(bits) << '\n';
    std::cout << "sign  = " << (negative ? '-' : '+') << '\n';
    std::cout << "raw exponent = " << raw_exponent << '\n';
    std::cout << "stored fraction = " << fraction << '\n';

    if (raw_exponent == traits::exponent_all_ones)
    {
        if (fraction == 0)
            std::cout << "classification: infinity\n";
        else
            std::cout << "classification: NaN (payload bits live in the fraction field)\n";
        return;
    }

    if (raw_exponent == 0)
    {
        if (fraction == 0)
        {
            std::cout << "classification: "
                      << (negative ? "negative zero" : "positive zero")
                      << '\n';
            return;
        }

        const int exponent = 1 - traits::exponent_bias;
        const long double significand =
            static_cast<long double>(fraction) /
            std::ldexp(1.0L, traits::fraction_bits);

        std::cout << "classification: subnormal\n";
        std::cout << "effective significand = 0.fraction = "
                  << significand << '\n';
        std::cout << "effective exponent = " << exponent << '\n';
        return;
    }

    const int exponent =
        static_cast<int>(raw_exponent) - traits::exponent_bias;
    const long double significand =
        1.0L +
        static_cast<long double>(fraction) /
            std::ldexp(1.0L, traits::fraction_bits);

    std::cout << "classification: normal\n";
    std::cout << "effective significand = 1.fraction = "
              << significand << '\n';
    std::cout << "effective exponent = " << exponent << '\n';
}

template <std::floating_point T>
void numerical_decomposition(T value)
{
    std::cout << "\nstd::frexp numerical decomposition of "
              << std::setprecision(std::numeric_limits<T>::max_digits10)
              << value << "\n";

    int exponent = 0;
    const T fraction = std::frexp(value, &exponent);

    std::cout << "fraction = " << fraction
              << ", exponent = " << exponent
              << ", recomposed = " << std::ldexp(fraction, exponent)
              << '\n';

    // frexp/ldexp describe the numeric value and do not require knowledge of
    // IEEE field widths, exponent bias, or object representation.
}

static void special_values()
{
    std::cout << "\n== Special encodings ==\n";

    extract_ieee_components(+0.0f);
    extract_ieee_components(-0.0f);
    extract_ieee_components(std::numeric_limits<float>::denorm_min());
    extract_ieee_components(std::numeric_limits<float>::infinity());
    extract_ieee_components(std::numeric_limits<float>::quiet_NaN());
}

int main()
{
    static_assert(std::numeric_limits<float>::is_iec559,
                  "This bit-field demonstration expects IEC 559 / IEEE-style float");
    static_assert(std::numeric_limits<double>::is_iec559,
                  "This bit-field demonstration expects IEC 559 / IEEE-style double");

    extract_ieee_components(1.0f);
    extract_ieee_components(1.5f);
    extract_ieee_components(-1.0f);

    extract_ieee_components(1.0);
    extract_ieee_components(1.5);

    numerical_decomposition(8.0);
    numerical_decomposition(0.1);

    special_values();
}
