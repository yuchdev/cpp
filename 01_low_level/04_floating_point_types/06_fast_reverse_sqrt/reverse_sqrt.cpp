#include <bit>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

static float exact_rsqrt(float x)
{
    return 1.0f / std::sqrt(x);
}

// Historical Quake-style initial approximation + one Newton step.
// C++20 std::bit_cast makes the representation transfer well-defined;
// it does NOT make the IEEE-binary32 assumptions portable.
static float fast_rsqrt(float number)
{
    static_assert(sizeof(float) == sizeof(std::uint32_t));
    static_assert(std::numeric_limits<float>::is_iec559);
    static_assert(std::numeric_limits<float>::radix == 2);
    static_assert(std::numeric_limits<float>::digits == 24);

    if (std::isnan(number) || number < 0.0f)
        return std::numeric_limits<float>::quiet_NaN();

    if (number == 0.0f)
        return std::numeric_limits<float>::infinity();

    if (std::isinf(number))
        return 0.0f;

    // The classic magic constant was tuned for positive binary32 values.
    std::uint32_t bits = std::bit_cast<std::uint32_t>(number);
    bits = 0x5f3759dfu - (bits >> 1);

    float y = std::bit_cast<float>(bits);
    const float half_x = 0.5f * number;

    y = y * (1.5f - half_x * y * y);
    return y;
}

static float fast_rsqrt_fma(float number)
{
    if (!std::isfinite(number) || number <= 0.0f)
        return fast_rsqrt(number);

    std::uint32_t bits = std::bit_cast<std::uint32_t>(number);
    bits = 0x5f3759dfu - (bits >> 1);

    float y = std::bit_cast<float>(bits);
    const float half_x = 0.5f * number;

    // Same Newton step, but the inner 1.5 - half_x*y*y expression can use
    // one fused multiply-add rounding where hardware/library support permits.
    y *= std::fma(-half_x, y * y, 1.5f);
    return y;
}

static void accuracy_demo()
{
    std::cout << "== Fast inverse square root accuracy ==\n";

    std::cout << std::setprecision(std::numeric_limits<float>::max_digits10);

    for (float x : {0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 10.0f, 1000.0f})
    {
        const float exact = exact_rsqrt(x);
        const float approx = fast_rsqrt(x);
        const float approx_fma = fast_rsqrt_fma(x);

        const float rel_error =
            std::fabs(approx - exact) / exact;
        const float rel_error_fma =
            std::fabs(approx_fma - exact) / exact;

        std::cout << "x=" << x
                  << " exact=" << exact
                  << " classic=" << approx
                  << " rel.err=" << rel_error
                  << " fma=" << approx_fma
                  << " rel.err=" << rel_error_fma
                  << '\n';
    }
}

template <typename Function>
static double benchmark(const std::vector<float>& values,
                        Function function,
                        const char* name)
{
    const auto start = std::chrono::steady_clock::now();

    double checksum = 0.0;
    for (float value : values)
        checksum += function(value);

    const auto stop = std::chrono::steady_clock::now();
    const auto elapsed =
        std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << name << ": " << elapsed.count()
              << " us, checksum=" << checksum << '\n';

    return checksum;
}

static void benchmark_demo()
{
    std::cout << "\n== Benchmark on this build/target ==\n";

    std::vector<float> values;
    values.reserve(1'000'000);

    for (std::size_t i = 1; i <= 1'000'000; ++i)
        values.push_back(0.01f + static_cast<float>(i % 10000) * 0.01f);

    const double exact_checksum =
        benchmark(values, exact_rsqrt, "1/sqrt(x)");
    const double fast_checksum =
        benchmark(values, fast_rsqrt, "classic fast rsqrt");

    std::cout << "checksum difference = "
              << std::fabs(exact_checksum - fast_checksum) << '\n';

    std::cout << "A historical instruction-count argument is not a modern "
                 "performance result: benchmark the actual compiler, ISA, "
                 "vectorization, and required error bound.\n";
}

static void edge_cases()
{
    std::cout << "\n== Explicit edge-case policy ==\n";

    for (float x : {
             0.0f,
             -0.0f,
             -1.0f,
             std::numeric_limits<float>::infinity(),
             std::numeric_limits<float>::quiet_NaN()})
    {
        std::cout << "x=" << x << " -> " << fast_rsqrt(x) << '\n';
    }
}

int main()
{
    accuracy_demo();
    edge_cases();
    benchmark_demo();
}
