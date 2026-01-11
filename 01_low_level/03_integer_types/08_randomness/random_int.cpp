// ReSharper disable All
#include <algorithm>
#include <array>
#include <chrono>
#include <climits>
#include <cstdint>
#include <cstdlib> // std::rand, std::srand
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <type_traits>
#include <vector>

/**
 * @file random_int.cpp
 * @brief Integer randomness in C++ from C++98 to C++20.
 *
 * This file is a self-contained lesson + demo program.
 *
 * The timeline (very roughly):
 * - C++98/03: `std::rand()` / `std::srand()` from `<cstdlib>`.
 * - C++11: `<random>` added with engines, distributions, and `std::random_device`.
 * - C++17: `std::clamp` is handy for DIY range mapping (still discouraged vs distributions).
 * - C++20: no radically new RNG API, but there are important library improvements and
 *          a stronger culture of using `<random>` correctly.
 *
 * @warning
 * `std::rand()` is not suitable for serious simulation, games, security, statistics,
 * or Monte-Carlo work. It's provided mostly for backward compatibility.
 */

namespace cpp
{

using clock = std::chrono::steady_clock;
using milliseconds = std::chrono::milliseconds;
using nanoseconds = std::chrono::nanoseconds;

// ---------------------------------------------------------------------
// Forward declarations (keep lesson sections readable while satisfying C++
// rules about declarations-before-use).
// ---------------------------------------------------------------------

struct Timer;

// WARNING: not a real synchronization primitive. It's only to create an observable side-effect
// so the optimizer doesn't remove benchmarking loops.
extern volatile std::uint64_t sink;

template <class F>
static milliseconds bench_ms(std::size_t n, F&& f);

/**
 * @brief Return a time-based seed suitable for `std::srand()`.
 *
 * In C++98 code you often see `std::srand(std::time(nullptr))`. This is:
 * - low entropy (1 second granularity)
 * - repeatable across processes started within the same second
 * - not portable in quality
 * - locked data bus on some platforms (time() syscall)
 */
static unsigned seed_for_srand()
{
    const auto now = clock::now().time_since_epoch();
    const auto ticks = std::chrono::duration_cast<nanoseconds>(now).count();
    return static_cast<unsigned>(ticks);
}

/**
 * @brief Map `std::rand()` output into [lo, hi] using modulo.
 *
 * This is the classic C++98 idiom:
 * @code
 * int x = lo + std::rand() % (hi - lo + 1);
 * @endcode
 *
 * Problems:
 * 1) Modulo bias: if (RAND_MAX+1) is not divisible by the range size, some values
 *    become more probable.
 * 2) Usually very low RAND_MAX (often 32767), so you only get ~15 bits of randomness.
 * 3) Global state + poor generator quality.
 */
static int rand_int_modulo(int lo, int hi)
{
    if (lo > hi)
        std::swap(lo, hi);

    // Beware: (hi - lo + 1) can overflow for extreme ranges; keep the demo simple.
    const auto range = static_cast<unsigned>(hi - lo + 1);
    return lo + static_cast<int>(static_cast<unsigned>(std::rand()) % range); // NOLINT(*-msc50-cpp)
}

/**
 * @brief Generate an unbiased integer in [0, bound) using rejection sampling.
 *
 * This is the right algorithmic idea when you're stuck with a generator that produces
 * values in [0, RAND_MAX]. In practice, you should prefer `<random>`.
 */
static unsigned rand_bounded_reject(unsigned bound)
{
    if (bound == 0u)
        return 0u;

    // Compute the largest multiple of bound within [0, RAND_MAX].
    // Values >= limit would create modulo bias and must be rejected.
    const unsigned limit = (static_cast<unsigned>(RAND_MAX) / bound) * bound;

    unsigned r{};
    do
    {
        r = static_cast<unsigned>(std::rand()); // NOLINT(*-msc50-cpp)
    } while (r >= limit);

    return r % bound;
}

/**
 * @brief C++98-friendly bounded int using rejection sampling.
 */
static int rand_int_reject(int lo, int hi)
{
    if (lo > hi)
        std::swap(lo, hi);
    const auto range = static_cast<unsigned>(hi - lo + 1);
    return lo + static_cast<int>(rand_bounded_reject(range));
}

/**
 * @brief Create an engine seeded with good-enough entropy for most non-crypto uses.
 *
 * The standard C++11 approach is:
 * - Use `std::random_device` to obtain entropy (if available).
 * - Feed a `std::seed_seq`.
 * - Seed a high-quality engine like `std::mt19937`.
 */
static std::mt19937 make_mt_engine()
{
    std::random_device rd;

    // Seed with several 32-bit values instead of a single one.
    // This reduces correlation if rd() is weak or deterministic on a platform.
    std::seed_seq seq{
        rd(), rd(), rd(), rd(),
        rd(), rd(), rd(), rd()};

    return std::mt19937{seq};
}

/**
 * @brief Explain why `rand()` is slow/inefficient compared to modern tools.
 *
 * Key points (the short but practical engineering view):
 * - **Global shared state**: `rand()` updates a global RNG state. This blocks many
 *   optimizations and hurts multi-threading (often requires hidden synchronization
 *   or forces you to add your own locking).
 * - **Tiny output**: `RAND_MAX` is commonly 32767. To make a 32-bit/64-bit result
 *   you must call `rand()` multiple times and combine bits, increasing overhead.
 * - **Bad statistical quality**: many `rand()` implementations are simple LCGs.
 *   When you then do `% range`, you amplify low-bit patterns (LCGs have especially
 *   poor low bits).
 * - **Distribution mapping done wrong**: `%` is biased unless the range divides
 *   RAND_MAX+1. Correct mapping requires rejection sampling (extra branches/loops).
 * - **ABI / legacy constraints**: implementations keep it stable and simple,
 *   and it can't easily evolve into a better, larger-state generator.
 *
 * Modern `<random>` engines like `std::mt19937`:
 * - Are explicit objects (you can have one per thread)
 * - Generate full-width results (32 bits per call for mt19937)
 * - Compose with correct distributions (`std::uniform_int_distribution` is unbiased)
 * - Often inline well and are easier for the optimizer to reason about
 *
 * See the performance comparison below.
 */
static void print_why_rand_is_bad()
{
    std::cout
        << "Why rand() tends to be slow/inefficient vs <random>:\n"
        << "  * Global shared state (hard to optimize, painful for threads)\n"
        << "  * Often tiny RAND_MAX (commonly 32767 => ~15 random bits)\n"
        << "  * Many implementations are low-quality LCGs (esp. low bits)\n"
        << "  * Modulo mapping is biased; unbiased mapping needs rejection loop\n"
        << "  * Legacy ABI: can't realistically upgrade semantics/quality\n";

    // ---------------------------------------------------------------------
    // A tiny, local performance comparison.
    //
    // NOTE:
    // This is a micro-benchmark; it is NOT a universal truth because it depends on:
    // - libc/libstdc++/libc++ implementation details
    // - optimization flags
    // - CPU frequency scaling / turbo / power state
    //
    // Still, the trends are usually consistent:
    // - rand() has global state and often produces few bits per call
    // - “fixing” rand() range/bias costs extra work
    // - <random> gives you explicit state + better mapping
    // ---------------------------------------------------------------------

    constexpr int lo = 0;
    constexpr int hi = 9;
    constexpr std::size_t N = 2'000'000;

    // The rejection limit for unbiased mapping to [0,9] when using rand().
    // For typical RAND_MAX=32767: limit = 32760, so ~7/32767 calls get rejected.
    const unsigned limit = (static_cast<unsigned>(RAND_MAX) / 10u) * 10u;
    std::cout << "    (example: unbiased rand() for [0,9] rejects r >= " << limit << ")\n";

    // Make sure rand() is seeded (otherwise it may be deterministic across runs).
    std::srand(cpp::seed_for_srand());

    const auto ms_rand_mod = cpp::bench_ms(N, [&]()
        {
            // Fast but biased mapping.
            return cpp::rand_int_modulo(lo, hi); });

    const auto ms_rand_rej = cpp::bench_ms(N, [&]()
        {
            // Unbiased mapping (still uses rand()).
            return cpp::rand_int_reject(lo, hi); });

    auto eng = cpp::make_mt_engine();
    std::uniform_int_distribution<int> dist(lo, hi);

    const auto ms_mt = cpp::bench_ms(N, [&]()
        { return dist(eng); });

    std::cout << "  Perf (" << N << " ints in [0,9], lower is better):\n";
    std::cout << "    rand() % range            : " << ms_rand_mod.count() << " ms (biased)\n";
    std::cout << "    rand() + rejection mapping : " << ms_rand_rej.count() << " ms (unbiased mapping)\n";
    std::cout << "    mt19937 + uniform_int_dist : " << ms_mt.count() << " ms (recommended)\n";

    // A tiny note about what you're really paying for.
    std::cout << "  Takeaway: rand() often looks 'simple', but correctness (unbiased ranges)\n"
                 "            adds work, and its global state makes scaling harder.\n";
}

/**
 * @brief A very small timing helper.
 */
struct Timer
{
    clock::time_point start = clock::now();

    template <class Duration = milliseconds>
    [[nodiscard]] auto elapsed() const
    {
        return std::chrono::duration_cast<Duration>(clock::now() - start);
    }
};

/**
 * @brief Consume numbers to prevent the optimizer from removing loops.
 */
volatile std::uint64_t sink = 0;

/**
 * @brief Benchmark N generations of a generator callable returning an int.
 */
template <class F>
static milliseconds bench_ms(std::size_t n, F&& f)
{
    Timer t;
    std::uint64_t acc = 0;
    for (std::size_t i = 0; i < n; ++i)
    {
        acc += static_cast<std::uint64_t>(f());
    }
    sink = acc;
    return t.elapsed<>();
}

/**
 * @brief Print a small histogram so we can eyeball bias.
 */
template <class Gen>
static void histogram_small(Gen&& gen, int lo, int hi, std::size_t samples)
{
    if (lo > hi)
        std::swap(lo, hi);
    const int buckets = hi - lo + 1;
    std::vector<std::size_t> counts(static_cast<std::size_t>(buckets), 0);

    for (std::size_t i = 0; i < samples; ++i)
    {
        const int v = gen();
        if (v < lo || v > hi)
            continue;
        ++counts[static_cast<std::size_t>(v - lo)];
    }

    const auto minmax = std::minmax_element(counts.begin(), counts.end());
    const auto minv = *minmax.first;
    const auto maxv = *minmax.second;

    std::cout << "Histogram (" << samples << " samples, range [" << lo << "," << hi << "]):\n";
    for (int x = lo; x <= hi; ++x)
    {
        const auto c = counts[static_cast<std::size_t>(x - lo)];
        std::cout << "  " << std::setw(2) << x << ": " << std::setw(8) << c;
        if (maxv > 0)
        {
            const int bar = static_cast<int>(50.0 * (static_cast<double>(c) / static_cast<double>(maxv)));
            std::cout << " |" << std::string(static_cast<std::size_t>(bar), '#');
        }
        std::cout << "\n";
    }
    std::cout << "  min=" << minv << ", max=" << maxv << "\n\n";
}

} // namespace lesson

/**
 * @brief Program entry.
 *
 * The program demonstrates:
 * - C++98 style `rand()` generation (modulo + rejection)
 * - C++11+ `<random>` with `std::mt19937` and `std::uniform_int_distribution`
 * - A small micro-benchmark and a histogram to visualize bias
 */
int main(int /*argc*/, char* /*argv*/[])
{
    std::cout << "Integer randomness demo (C++98 -> C++20)\n";
    std::cout << "CHAR_BIT=" << CHAR_BIT
              << ", sizeof(int)=" << sizeof(int)
              << ", RAND_MAX=" << RAND_MAX << "\n\n";

    cpp::print_why_rand_is_bad();
    std::cout << "\n";

    // -------------------------------------------------------------------------
    // C++98/03: std::rand
    // -------------------------------------------------------------------------

    // Seeding: many codebases used `std::time(nullptr)`. We'll use a higher-resolution
    // time seed without depending on <ctime>.
    std::srand(cpp::seed_for_srand());

    constexpr int lo = 0;
    constexpr int hi = 9;

    std::cout << "C++98: rand() % range (biased)\n";
    cpp::histogram_small([&](){ return cpp::rand_int_modulo(lo, hi); }, lo, hi, 200'000);

    std::cout << "C++98: rand() with rejection sampling (unbiased mapping, still poor generator)\n";
    cpp::histogram_small([&](){ return cpp::rand_int_reject(lo, hi); }, lo, hi, 200'000);

    // -------------------------------------------------------------------------
    // C++11+: <random>
    // -------------------------------------------------------------------------

    // Engine: mt19937 is a good general-purpose PRNG (not cryptographic).
    auto eng = cpp::make_mt_engine();

    // Distribution: handles unbiased integer range mapping correctly.
    std::uniform_int_distribution<int> dist(lo, hi);

    std::cout << "C++11+: mt19937 + uniform_int_distribution (recommended default)\n";
    cpp::histogram_small([&](){ return dist(eng); }, lo, hi, 200'000);

    // -------------------------------------------------------------------------
    // Micro-benchmark (very rough; just to illustrate overhead patterns)
    // -------------------------------------------------------------------------

    constexpr std::size_t N = 10'000'000;

    // Note: This isn't a perfect benchmark. It will vary by platform/libc++/libstdc++.
    // It's here to highlight typical costs: rand() global state, small output width,
    // modulo/rejection mapping, and distribution overhead vs inlined engine calls.

    // Ensure rand() is seeded.
    std::srand(cpp::seed_for_srand());

    const auto ms_rand_mod = cpp::bench_ms(N, [&]()
        { return cpp::rand_int_modulo(0, std::numeric_limits<int>::max() / 2); });
    const auto ms_rand_rej = cpp::bench_ms(N, [&]()
        { return cpp::rand_int_reject(0, std::numeric_limits<int>::max() / 2); });

    auto eng2 = cpp::make_mt_engine();
    std::uniform_int_distribution<int> dist2(0, std::numeric_limits<int>::max() / 2);
    const auto ms_mt = cpp::bench_ms(N, [&]()
        { return dist2(eng2); });

    std::cout << "Timing (" << N << " generations, lower is better):\n";
    std::cout << "  rand() % range        : " << ms_rand_mod.count() << " ms\n";
    std::cout << "  rand() rejection      : " << ms_rand_rej.count() << " ms\n";
    std::cout << "  mt19937 + distribution: " << ms_mt.count() << " ms\n\n";

    // -------------------------------------------------------------------------
    // Practical guidance summary
    // -------------------------------------------------------------------------
    std::cout
        << "Practical guidance:\n"
        << "  * Need repeatable pseudo-random ints? Use <random> engines + distributions.\n"
        << "    - Seed with a fixed constant for deterministic tests.\n"
        << "    - Seed with random_device/seed_seq for non-deterministic runs.\n"
        << "  * Need cross-thread scalability? Give each thread its own engine instance.\n"
        << "  * Need cryptographic randomness? C++ standard library does not guarantee it.\n"
        << "    Use platform APIs / vetted crypto libraries.\n";

    return 0;
}
