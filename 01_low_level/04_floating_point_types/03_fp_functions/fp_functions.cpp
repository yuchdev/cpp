#include <cmath>
#include <cfenv>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numbers>
#include <numeric>

static void print_roundings(double value)
{
    std::cout << std::fixed << std::setprecision(1)
              << "x=" << value
              << " floor=" << std::floor(value)
              << " ceil=" << std::ceil(value)
              << " trunc=" << std::trunc(value)
              << " round=" << std::round(value)
              << " rint=" << std::rint(value)
              << " nearbyint=" << std::nearbyint(value)
              << '\n';
}

static void rounding_functions()
{
    std::cout << "== Rounding functions ==\n";

    for (double x : {-2.5, -1.5, -0.5, 0.5, 1.5, 2.5})
        print_roundings(x);

    const int old_mode = std::fegetround();

    if (std::fesetround(FE_DOWNWARD) == 0)
    {
        std::cout << "FE_DOWNWARD: rint(2.9)=" << std::rint(2.9)
                  << ", round(2.9)=" << std::round(2.9) << '\n';
    }

    std::fesetround(old_mode);

    std::cout << "round ignores the dynamic rounding mode; rint/nearbyint obey it.\n";
}

static void cxx20_math_constants()
{
    std::cout << "\n== C++20 <numbers> mathematical constants ==\n";

    const double pi = std::numbers::pi;
    const float pif = std::numbers::pi_v<float>;

    std::cout << std::setprecision(std::numeric_limits<double>::max_digits10)
              << "pi<double> = " << pi << '\n'
              << "pi<float>  = " << pif << '\n'
              << "e          = " << std::numbers::e << '\n'
              << "sqrt(2)    = " << std::numbers::sqrt2 << '\n'
              << "phi        = " << std::numbers::phi << '\n';

    std::cout << "sin(pi) = " << std::sin(pi)
              << " (close to zero, not generally exactly zero)\n";
}

static void cxx20_interpolation()
{
    std::cout << "\n== C++20 std::midpoint and std::lerp ==\n";

    const double a = std::numeric_limits<double>::max() / 2.0;
    const double b = std::numeric_limits<double>::max();

    const double naive_midpoint = (a + b) / 2.0;
    const double safe_midpoint = std::midpoint(a, b);

    std::cout << "naive midpoint finite? "
              << std::boolalpha << std::isfinite(naive_midpoint) << '\n'
              << "std::midpoint finite? " << std::isfinite(safe_midpoint) << '\n';

    std::cout << "lerp(10, 20, 0.25) = "
              << std::lerp(10.0, 20.0, 0.25) << '\n';

    // std::lerp is specified with useful endpoint/finite-result guarantees that
    // a casual a + t*(b-a) implementation may fail to preserve at extremes.
    std::cout << "lerp(max/2, max, 0.5) finite? "
              << std::isfinite(std::lerp(a, b, 0.5)) << '\n';
}

static void stable_special_forms()
{
    std::cout << "\n== Stable special forms ==\n";

    const double tiny = 1e-16;

    const double naive_log = std::log(1.0 + tiny);
    const double stable_log = std::log1p(tiny);

    const double naive_exp = std::exp(tiny) - 1.0;
    const double stable_exp = std::expm1(tiny);

    std::cout << std::setprecision(std::numeric_limits<double>::max_digits10);
    std::cout << "log(1 + 1e-16) = " << naive_log << '\n';
    std::cout << "log1p(1e-16)   = " << stable_log << '\n';
    std::cout << "exp(1e-16)-1   = " << naive_exp << '\n';
    std::cout << "expm1(1e-16)   = " << stable_exp << '\n';

    const double x = 1e308;
    const double y = 1e308;

    const double naive_hypot = std::sqrt(x * x + y * y);
    const double stable_hypot = std::hypot(x, y);

    std::cout << "sqrt(x*x+y*y) finite? "
              << std::boolalpha << std::isfinite(naive_hypot) << '\n';
    std::cout << "hypot(x,y) finite? "
              << std::isfinite(stable_hypot) << '\n';
}

static void fused_multiply_add()
{
    std::cout << "\n== std::fma: one rounding instead of two ==\n";

    // Carefully chosen values can expose the difference between an explicit
    // multiply followed by add and one fused operation.
    const double a = 0x1.0000000000001p+0;
    const double b = 0x1.fffffffffffffp+0;
    const double c = -0x1.0000000000000p+1;

    const double separate = a * b + c;
    const double fused = std::fma(a, b, c);

    std::cout << std::hexfloat
              << "a*b+c = " << separate << '\n'
              << "fma   = " << fused << '\n'
              << std::defaultfloat;

    std::cout << "FMA changes intermediate rounding; different answers do not "
                 "imply either computation violated its contract.\n";
}

static void decomposition_and_scaling()
{
    std::cout << "\n== Numerical decomposition ==\n";

    const double value = 13.5;
    int exponent = 0;

    const double fraction = std::frexp(value, &exponent);
    const double recomposed = std::ldexp(fraction, exponent);

    std::cout << value << " = " << fraction << " * 2^"
              << exponent << ", recomposed=" << recomposed << '\n';

    double integer_part = 0.0;
    const double fractional_part = std::modf(std::numbers::pi, &integer_part);

    std::cout << "pi = " << integer_part << " + "
              << fractional_part << '\n';

    std::cout << "ilogb(1024) = " << std::ilogb(1024.0) << '\n';
    std::cout << "scalbn(1.5, 10) = " << std::scalbn(1.5, 10) << '\n';
}

static const char* classification_name(double value)
{
    switch (std::fpclassify(value))
    {
    case FP_INFINITE:
        return "infinite";
    case FP_NAN:
        return "NaN";
    case FP_ZERO:
        return std::signbit(value) ? "negative zero" : "positive zero";
    case FP_SUBNORMAL:
        return "subnormal";
    case FP_NORMAL:
        return "normal";
    default:
        return "unknown";
    }
}

static void classification()
{
    std::cout << "\n== Classification and neighboring values ==\n";

    const double inf = std::numeric_limits<double>::infinity();
    const double qnan = std::numeric_limits<double>::quiet_NaN();
    const double subnormal = std::numeric_limits<double>::denorm_min();

    for (double x : {0.0, -0.0, 1.0, -1.0, subnormal, inf, -inf, qnan})
        std::cout << x << " -> " << classification_name(x) << '\n';

    std::cout << std::setprecision(std::numeric_limits<double>::max_digits10)
              << "nextafter(1,+inf) = "
              << std::nextafter(1.0, inf) << '\n'
              << "nextafter(0,+1) = "
              << std::nextafter(0.0, 1.0) << '\n';
}

int main()
{
    rounding_functions();
    cxx20_math_constants();
    cxx20_interpolation();
    stable_special_forms();
    fused_multiply_add();
    decomposition_and_scaling();
    classification();
}
