#include <bit>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <optional>

static void builtin_conversion_semantics()
{
    std::cout << "== Built-in floating -> integer conversion ==\n";

    for (double x : {9.99, -9.99, 1.5, -1.5})
    {
        std::cout << x << " -> static_cast<int> = "
                  << static_cast<int>(x) << '\n';
    }

    std::cout << "Built-in conversion truncates toward zero; it does not obey "
                 "the dynamic FP rounding mode.\n";
}

static std::optional<int> checked_to_int(double value)
{
    if (!std::isfinite(value))
        return std::nullopt;

    // For this concrete double -> int example, both int endpoints are exactly
    // representable as double on mainstream implementations.
    constexpr double lo =
        static_cast<double>(std::numeric_limits<int>::min());
    constexpr double hi =
        static_cast<double>(std::numeric_limits<int>::max());

    // Conversion first discards the fractional part. A value such as
    // INT_MAX + 0.9 still truncates to INT_MAX and is representable.
    const double truncated = std::trunc(value);

    if (truncated < lo || truncated > hi)
        return std::nullopt;

    return static_cast<int>(value);
}

static void checked_conversion_demo()
{
    std::cout << "\n== Range-check before conversion ==\n";

    const double values[] = {
        42.75,
        -42.75,
        static_cast<double>(std::numeric_limits<int>::max()),
        static_cast<double>(std::numeric_limits<int>::max()) + 1024.0,
        std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::quiet_NaN()
    };

    for (double value : values)
    {
        const auto result = checked_to_int(value);

        std::cout << std::setprecision(std::numeric_limits<double>::max_digits10)
                  << value << " -> ";

        if (result)
            std::cout << *result;
        else
            std::cout << "rejected";

        std::cout << '\n';
    }

    std::cout << "Out-of-range floating -> integer conversion, including NaN "
                 "and infinity, is undefined behavior for the built-in cast.\n";
}

static void representation_is_not_conversion()
{
    std::cout << "\n== std::bit_cast is representation transfer, not conversion ==\n";

    constexpr float value = 9.75f;
    constexpr std::uint32_t bits =
        std::bit_cast<std::uint32_t>(value);

    std::cout << "numeric conversion static_cast<int>(9.75f) = "
              << static_cast<int>(value) << '\n';
    std::cout << "representation bit_cast<uint32_t>(9.75f) = 0x"
              << std::hex << bits << std::dec << '\n';

    static_assert(std::bit_cast<float>(bits) == value);
}

// Historical "add a large float and subtract its integer representation" trick.
// This is intentionally presented as an obsolete rounding trick, not as a
// replacement for static_cast<int>. It relies on IEEE binary32 layout and
// round-to-nearest behavior and has a restricted useful range.
static int historical_magic_round_to_int(float value)
{
    static_assert(sizeof(float) == sizeof(std::uint32_t));
    static_assert(std::numeric_limits<float>::is_iec559);
    static_assert(std::numeric_limits<float>::radix == 2);
    static_assert(std::numeric_limits<float>::digits == 24);

    constexpr std::uint32_t magic_bits =
        (std::uint32_t{150} << 23) | (std::uint32_t{1} << 22);
    constexpr float magic = std::bit_cast<float>(magic_bits);

    const float shifted = value + magic;
    const std::int32_t shifted_bits =
        std::bit_cast<std::int32_t>(shifted);
    const std::int32_t magic_as_signed =
        static_cast<std::int32_t>(magic_bits);

    return shifted_bits - magic_as_signed;
}

static void historical_trick_demo()
{
    std::cout << "\n== Historical magic-number rounding trick ==\n";

    for (float x : {1.4f, 1.5f, 1.6f, -1.4f, -1.5f, -1.6f, 9.99f})
    {
        std::cout << x
                  << " -> magic trick " << historical_magic_round_to_int(x)
                  << ", static_cast<int> " << static_cast<int>(x)
                  << ", lround " << std::lround(x)
                  << '\n';
    }

    std::cout << "Notice that the historical trick rounds; static_cast<int> "
                 "truncates. Modern CPUs have direct conversion instructions, "
                 "so this representation trick is mainly educational.\n";
}

static void large_integer_to_double()
{
    std::cout << "\n== Integer -> floating precision loss ==\n";

    static_assert(std::numeric_limits<double>::radix == 2);
    static_assert(std::numeric_limits<double>::digits < 64);

    constexpr std::uint64_t exact_boundary =
        std::uint64_t{1} << std::numeric_limits<double>::digits; // 2^53 on binary64

    const std::uint64_t n1 = exact_boundary;
    const std::uint64_t n2 = exact_boundary + 1;

    const double d1 = static_cast<double>(n1);
    const double d2 = static_cast<double>(n2);

    std::cout << "n1 = " << n1 << ", n2 = " << n2 << '\n';
    std::cout << std::setprecision(std::numeric_limits<double>::max_digits10)
              << "double(n1) = " << d1
              << ", double(n2) = " << d2 << '\n';
    std::cout << std::boolalpha
              << "double(n1) == double(n2): " << (d1 == d2) << '\n';
}

int main()
{
    builtin_conversion_semantics();
    checked_conversion_demo();
    representation_is_not_conversion();
    historical_trick_demo();
    large_integer_to_double();
}
