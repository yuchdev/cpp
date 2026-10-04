#include <bit>
#include <bitset>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numbers>
#include <type_traits>

#if __cplusplus > 202002L && __has_include(<stdfloat>)
#include <stdfloat>
#define CPP_DEMO_HAS_STDFLOAT 1
#endif

template <typename Floating, typename UInt>
void print_bits(Floating value)
{
    static_assert(std::is_floating_point_v<Floating>);
    static_assert(std::is_unsigned_v<UInt>);
    static_assert(sizeof(Floating) == sizeof(UInt));

    const UInt bits = std::bit_cast<UInt>(value);

    std::cout << std::setprecision(std::numeric_limits<Floating>::max_digits10)
              << value << " -> 0x" << std::hex << bits << std::dec
              << " -> " << std::bitset<sizeof(UInt) * 8>(bits) << '\n';
}

template <typename T>
void print_limits(const char* name)
{
    using limits = std::numeric_limits<T>;

    std::cout << name
              << ": sizeof=" << sizeof(T)
              << ", radix=" << limits::radix
              << ", digits=" << limits::digits
              << ", digits10=" << limits::digits10
              << ", max_digits10=" << limits::max_digits10
              << ", min=" << limits::min()
              << ", lowest=" << limits::lowest()
              << ", max=" << limits::max()
              << ", IEC559=" << std::boolalpha << limits::is_iec559
              << '\n';
}

static void representation_and_limits()
{
    std::cout << "== Standard floating-point types ==\n";

    // C++ guarantees only the nondecreasing size relation; exact formats are
    // implementation-defined.
    static_assert(sizeof(float) <= sizeof(double));
    static_assert(sizeof(double) <= sizeof(long double));

    print_limits<float>("float");
    print_limits<double>("double");
    print_limits<long double>("long double");

    // The bit layout below is meaningful only after establishing the expected
    // format. These assertions intentionally document the assumption.
    if constexpr (sizeof(float) == sizeof(std::uint32_t))
    {
        std::cout << "\nfloat representations (C++20 std::bit_cast):\n";
        print_bits<float, std::uint32_t>(1.0f);
        print_bits<float, std::uint32_t>(1.5f);
        print_bits<float, std::uint32_t>(0.75f);
        print_bits<float, std::uint32_t>(-0.0f);
    }

    if constexpr (sizeof(double) == sizeof(std::uint64_t))
    {
        std::cout << "\ndouble representations (C++20 std::bit_cast):\n";
        print_bits<double, std::uint64_t>(1.0);
        print_bits<double, std::uint64_t>(1.5);
        print_bits<double, std::uint64_t>(0.75);
        print_bits<double, std::uint64_t>(-0.0);
    }
}

static void spacing_and_integer_precision()
{
    std::cout << "\n== Non-uniform spacing ==\n";

    const double one = 1.0;
    const double next_one =
        std::nextafter(one, std::numeric_limits<double>::infinity());

    const double large = 1'000'000'000'000.0;
    const double next_large =
        std::nextafter(large, std::numeric_limits<double>::infinity());

    std::cout << std::setprecision(std::numeric_limits<double>::max_digits10);
    std::cout << "nextafter(1.0,+inf)-1.0 = "
              << next_one - one << '\n';
    std::cout << "nextafter(1e12,+inf)-1e12 = "
              << next_large - large << '\n';

    // binary32 has 24 bits of precision on the overwhelmingly common IEEE
    // implementation. At 2^24, the next integer can no longer be represented.
    if constexpr (std::numeric_limits<float>::radix == 2 &&
                  std::numeric_limits<float>::digits == 24)
    {
        constexpr float exact_boundary = 16'777'216.0f; // 2^24
        constexpr float rounded_next = 16'777'217.0f;
        static_assert(exact_boundary == rounded_next);

        std::cout << "float(16777216) == float(16777217): "
                  << std::boolalpha
                  << (exact_boundary == rounded_next) << '\n';
    }
}

static void endian_demo()
{
    std::cout << "\n== C++20 std::endian ==\n";

    if constexpr (std::endian::native == std::endian::little)
        std::cout << "native scalar byte order: little endian\n";
    else if constexpr (std::endian::native == std::endian::big)
        std::cout << "native scalar byte order: big endian\n";
    else
        std::cout << "native scalar byte order: mixed/other\n";

    std::cout << "Endianness describes byte order in memory; IEEE field positions "
                 "are a separate representation question.\n";
}


#if defined(__cpp_lib_byteswap) && __cpp_lib_byteswap >= 202110L
static void cxx23_byteswap_demo()
{
    std::cout << "\n== C++23 std::byteswap on a floating representation ==\n";

    static_assert(sizeof(float) == sizeof(std::uint32_t));

    constexpr float value = 1.0f;
    constexpr std::uint32_t bits =
        std::bit_cast<std::uint32_t>(value);
    constexpr std::uint32_t swapped =
        std::byteswap(bits);

    std::cout << "float 1.0 bits = 0x" << std::hex << bits
              << ", byte-swapped = 0x" << swapped
              << std::dec << '\n';

    std::cout << "byteswap operates on the integer representation; it does not "
                 "numerically convert the floating-point value.\n";
}
#endif

#if defined(CPP_DEMO_HAS_STDFLOAT)
static void cxx23_fixed_width_floating_types()
{
    std::cout << "\n== C++23 <stdfloat> optional extended types ==\n";

#ifdef __STDCPP_FLOAT16_T__
    {
        std::float16_t x = 0.1f16;
        std::cout << "float16_t: sizeof=" << sizeof(x)
                  << ", digits=" << std::numeric_limits<std::float16_t>::digits
                  << ", value(as double)=" << static_cast<double>(x) << '\n';
    }
#endif

#ifdef __STDCPP_FLOAT32_T__
    {
        static_assert(!std::is_same_v<std::float32_t, float>);
        std::float32_t x = 0.1f32;
        const auto mixed = x + 1.0f;

        // C++23 conversion rank/subrank rules make the fixed-width extended
        // type win over a standard type of equal rank.
        static_assert(std::is_same_v<decltype(mixed), const std::float32_t>);

        const auto pi32 = std::numbers::pi_v<std::float32_t>;

        std::cout << "float32_t: sizeof=" << sizeof(x)
                  << ", digits=" << std::numeric_limits<std::float32_t>::digits
                  << ", value(as double)=" << static_cast<double>(x)
                  << ", pi(as double)=" << static_cast<double>(pi32) << '\n';
    }
#endif

#ifdef __STDCPP_FLOAT64_T__
    {
        static_assert(!std::is_same_v<std::float64_t, double>);
        std::float64_t x = 0.1f64;
        std::cout << "float64_t: sizeof=" << sizeof(x)
                  << ", digits=" << std::numeric_limits<std::float64_t>::digits
                  << ", value(as long double)="
                  << static_cast<long double>(x) << '\n';
    }
#endif

#ifdef __STDCPP_BFLOAT16_T__
    {
        std::bfloat16_t x = 0.1bf16;
        std::cout << "bfloat16_t: sizeof=" << sizeof(x)
                  << ", digits=" << std::numeric_limits<std::bfloat16_t>::digits
                  << ", value(as double)=" << static_cast<double>(x) << '\n';
    }
#endif

#if !defined(__STDCPP_FLOAT16_T__) && !defined(__STDCPP_FLOAT32_T__) && \
    !defined(__STDCPP_FLOAT64_T__) && !defined(__STDCPP_FLOAT128_T__) && \
    !defined(__STDCPP_BFLOAT16_T__)
    std::cout << "<stdfloat> exists, but this implementation exposes none "
                 "of the optional fixed-width floating types.\n";
#endif
}
#endif

int main()
{
    representation_and_limits();
    spacing_and_integer_precision();
    endian_demo();

#if defined(__cpp_lib_byteswap) && __cpp_lib_byteswap >= 202110L
    cxx23_byteswap_demo();
#endif

#if defined(CPP_DEMO_HAS_STDFLOAT)
    cxx23_fixed_width_floating_types();
#else
    std::cout << "\nC++23 <stdfloat> demo not enabled by this build/toolchain.\n";
#endif
}
