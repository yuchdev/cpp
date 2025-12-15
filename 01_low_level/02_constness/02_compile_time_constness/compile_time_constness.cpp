// ReSharper disable All
#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <type_traits>

namespace cpp {

// ---- 1) constexpr basics -----------------------------------------------------
// constexpr variable => can be used in constant evaluation contexts.
// const variable => read-only but may or may not be constant-evaluable.
constexpr int square(int x) { return x * x; }

// C++14 and further allows loops/branches in constexpr functions.
// (In C++11, this factorial with loop would not be allowed)
constexpr int factorial(int n)
{
    int result = 1;
    for (int i = 1; i <= n; ++i) result *= i;
    return result;
}

static void compiletime_constness()
{
    constexpr int side = 5;
    constexpr int area = square(side);
    static_assert(area == 25);

    constexpr int f = factorial(6);
    static_assert(f == 720);

    // Use in a context requiring constant expression:
    std::array<int, square(3)> a{}; // size 9
    static_assert(a.size() == 9);
}

// ---- 2) "constexpr" does NOT force compile-time evaluation -------------------
// A constexpr function can be evaluated at runtime if not required at compile time.
constexpr int maybe_runtime(int x) { return x + 1; }

static int read_runtime()
{
    return 41;
}

static void constexpr_can_run_at_runtime()
{
    int x = read_runtime();
    int y = maybe_runtime(x); // runtime call is fine
    assert(y == 42);
}

// ---- 3) consteval: MUST be evaluated at compile time -------------------------
// C++20+ feature.
#if __cpp_consteval >= 201811L
consteval int must_be_compile_time(int x)
{
    return x * 2;
}
#endif

static void consteval_demo()
{
#if __cpp_consteval >= 201811L
    constexpr int v = must_be_compile_time(21);
    static_assert(v == 42);

    // int x = read_runtime();
    // int bad = must_be_compile_time(x); // ERROR: not a constant expression
#else
    // If your compiler doesn't support consteval, skip.
#endif
}

// ---- 4) constinit: forces static initialization (not dynamic initialization) --
// constinit does NOT mean const. It means initialization happens before runtime.
// This prevents the "static initialization order fiasco" for that variable.
#if __cpp_constinit >= 201907L
constinit int g_counter = 0; // must be initialized with constant initialization
#endif

static void constinit_demo()
{
#if __cpp_constinit >= 201907L
    ++g_counter; // ok: not const
    assert(g_counter >= 1);
#endif
}

// ---- 5) constexpr objects, literal types, and constructors -------------------
// Literal type: can be used in constant expressions if it has constexpr constructors etc.
struct point
{
    double x_, y_, z_;
    constexpr point(double x, double y, double z) : x_{x}, y_{y}, z_{z} {}
    constexpr double norm2() const { return x_*x_ + y_*y_ + z_*z_; }
};

static void literal_types()
{
    constexpr point p{1.,2.,3.};
    static_assert(p.norm2() == 14.0);

    constexpr point parr[] = {{1.,2.,3.},{1.,2.,3.},{1.,2.,3.}};
    static_assert(parr[0].norm2() == 14.0);
}

// ---- 6) is_constant_evaluated: different behavior at compile-time vs runtime -
#if __cpp_lib_is_constant_evaluated >= 201811L
#include <utility>
constexpr int tricky(int x)
{
    // You can branch based on evaluation mode.
    if (std::is_constant_evaluated())
        return x + 1000;
    else
        return x + 1;
}
#endif

static void constant_evaluation_mode_demo()
{
#if __cpp_lib_is_constant_evaluated >= 201811L
    constexpr int a = tricky(1);
    static_assert(a == 1001);

    int r = tricky(1);
    assert(r == 2);
#endif
}

// ---- 7) constant evaluation contexts: where you MUST be constexpr ------------
// Examples:
// - array bounds
// - non-type template parameters
// - case labels
// - static_assert conditions
template<int N>
struct tag { static constexpr int value = N; };

static void constant_contexts()
{
    constexpr int n = 7;

    tag<n> t{};
    static_assert(t.value == 7);

    // switch case labels require integral constant expressions:
    switch (n)
    {
        case 7: break;
        default: assert(false);
    }
}

// ---- 8) constexpr and pointers / string literals -----------------------------
// String literals have static storage duration, so returning them is safe.
// But returning pointer to local storage is NOT safe (constexpr doesn't change that).
constexpr const char* error_message(int)
{
    return "range error";
}

static void string_literal_ok()
{
    constexpr const char* m = error_message(0);
    static_assert(m[0] == 'r');
}

// ---- 9) Pitfall: "const" is not "constant expression" ------------------------
// const int initialized with runtime value cannot be used where constant expression required.
static void const_is_not_constexpr()
{
    const int runtime = read_runtime();
    (void)runtime;

    // std::array<int, runtime> a{}; // ERROR: runtime not constant expression
}

} // namespace cpp

int main()
{
    cpp::compiletime_constness();
    cpp::constexpr_can_run_at_runtime();
    cpp::consteval_demo();
    cpp::constinit_demo();
    cpp::literal_types();
    cpp::constant_evaluation_mode_demo();
    cpp::constant_contexts();
    cpp::string_literal_ok();
    cpp::const_is_not_constexpr();

    std::cout << "compile_time_constness.cpp: OK\n";
    return 0;
}
