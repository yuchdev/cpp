#include <algorithm>
#include <bit>
#include <cmath>
#include <compare>
#include <concepts>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numbers>

template <std::floating_point T>
bool nearly_equal(T a, T b,
                  T rel_tol = T{8} * std::numeric_limits<T>::epsilon(),
                  T abs_tol = T{})
{
    // Exact equality is useful here: it handles identical finite values,
    // equal infinities, and +0 == -0 without subtraction.
    if (a == b)
        return true;

    if (std::isnan(a) || std::isnan(b))
        return false;

    const T diff = std::fabs(a - b);
    const T scale = std::max(std::fabs(a), std::fabs(b));

    return diff <= std::max(abs_tol, rel_tol * scale);
}

static std::uint64_t ordered_double_key(double value)
{
    static_assert(sizeof(double) == sizeof(std::uint64_t));
    static_assert(std::numeric_limits<double>::is_iec559);

    constexpr std::uint64_t sign = std::uint64_t{1} << 63;
    const std::uint64_t bits = std::bit_cast<std::uint64_t>(value);

    // Map IEEE sign-magnitude-like encoding into monotonically increasing keys.
    // Negative encodings are reversed; nonnegative encodings are shifted above them.
    return (bits & sign) ? ~bits : (bits | sign);
}

static std::uint64_t ulp_distance(double a, double b)
{
    if (a == b)
        return 0; // intentionally treats +0 and -0 as the same numeric value

    if (std::isnan(a) || std::isnan(b))
        return std::numeric_limits<std::uint64_t>::max();

    const auto ka = ordered_double_key(a);
    const auto kb = ordered_double_key(b);
    return ka > kb ? ka - kb : kb - ka;
}

static void compare_pi_approximations()
{
    std::cout << "== Exact vs approximate comparison ==\n";

    // Machin-like formula.
    const double pi1 =
        4.0 * (4.0 * std::atan(0.2) - std::atan(1.0 / 239.0));

    // Sexagesimal approximation.
    const double pi2 =
        3.0 + 8.0 / 60.0
            + 29.0 / std::pow(60.0, 2)
            + 44.0 / std::pow(60.0, 3);

    const double standard_pi = std::numbers::pi; // C++20

    std::cout << std::setprecision(std::numeric_limits<double>::max_digits10);
    std::cout << "Machin-like pi = " << pi1 << '\n';
    std::cout << "sexagesimal pi = " << pi2 << '\n';
    std::cout << "std::numbers::pi = " << standard_pi << '\n';

    std::cout << std::boolalpha;
    std::cout << "pi1 == pi2: " << (pi1 == pi2) << '\n';
    std::cout << "nearly_equal(pi1, pi2): "
              << nearly_equal(pi1, pi2, 1e-8, 1e-15) << '\n';

    // The sexagesimal approximation is only approximate to a few decimal places.
    // Machine epsilon is not an appropriate universal domain tolerance.
}

static void relative_and_absolute_tolerance()
{
    std::cout << "\n== Relative and absolute tolerance play different roles ==\n";

    const double near_zero_a = 0.0;
    const double near_zero_b = 1e-15;

    std::cout << "relative-only near zero: "
              << nearly_equal(near_zero_a, near_zero_b,
                              1e-12, 0.0) << '\n';
    std::cout << "with absolute tolerance: "
              << nearly_equal(near_zero_a, near_zero_b,
                              1e-12, 1e-14) << '\n';

    const double large_a = 1e12;
    const double large_b =
        std::nextafter(large_a, std::numeric_limits<double>::infinity());

    std::cout << std::setprecision(std::numeric_limits<double>::max_digits10)
              << "one ULP near 1e12 = " << (large_b - large_a) << '\n';
}

static void ulp_demo()
{
    std::cout << "\n== ULP neighborhood with std::nextafter ==\n";

    const double a = 1.0;
    const double b =
        std::nextafter(a, std::numeric_limits<double>::infinity());
    const double c =
        std::nextafter(b, std::numeric_limits<double>::infinity());

    std::cout << std::setprecision(std::numeric_limits<double>::max_digits10);
    std::cout << "a = " << a << '\n';
    std::cout << "b = " << b << ", ULP distance(a,b) = "
              << ulp_distance(a, b) << '\n';
    std::cout << "c = " << c << ", ULP distance(a,c) = "
              << ulp_distance(a, c) << '\n';

    std::cout << "ULP distance is a representation metric, not a domain-specific "
                 "definition of equality.\n";
}

static void partial_ordering_demo()
{
    std::cout << "\n== C++20 floating-point <=> gives partial ordering ==\n";

    const double qnan = std::numeric_limits<double>::quiet_NaN();
    const double one = 1.0;

    const std::partial_ordering normal_order = one <=> 2.0;
    const std::partial_ordering nan_order = qnan <=> one;

    std::cout << "1.0 <=> 2.0 is less: "
              << std::boolalpha
              << (normal_order == std::partial_ordering::less) << '\n';

    std::cout << "NaN <=> 1.0 is unordered: "
              << (nan_order == std::partial_ordering::unordered) << '\n';

    std::cout << "NaN == NaN: " << (qnan == qnan) << '\n';
    std::cout << "NaN < 1.0: " << (qnan < one) << '\n';

    // A comparator for sorting data containing NaNs therefore needs an explicit
    // policy; ordinary floating-point < is not a total ordering relation.
}

static void signed_zero_demo()
{
    std::cout << "\n== Signed zero ==\n";

    const double positive_zero = +0.0;
    const double negative_zero = -0.0;

    std::cout << std::boolalpha
              << "+0 == -0: " << (positive_zero == negative_zero) << '\n'
              << "signbit(+0): " << std::signbit(positive_zero) << '\n'
              << "signbit(-0): " << std::signbit(negative_zero) << '\n';

    if (std::numeric_limits<double>::is_iec559)
    {
        std::cout << "1/+0 = " << 1.0 / positive_zero << '\n'
                  << "1/-0 = " << 1.0 / negative_zero << '\n';
    }
}

int main()
{
    compare_pi_approximations();
    relative_and_absolute_tolerance();
    ulp_demo();
    partial_ordering_demo();
    signed_zero_demo();
}
