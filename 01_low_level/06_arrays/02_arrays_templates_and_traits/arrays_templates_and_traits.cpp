#include <cassert>
#include <cstddef>
#include <iostream>
#include <type_traits>

// arrays_templates_and_traits.cpp
// Focus: deduction of N, const propagation, decltype rules, std::extent/rank/remove_extent.

namespace cpp {

template <typename T, std::size_t N>
constexpr std::size_t array_size(T (&)[N]) noexcept { return N; }

static void deduction_and_const()
{
    int a[4] = {1,2,3,4};
    const int ca[2] = {10,20};

    static_assert(array_size(a) == 4, "");
    static_assert(array_size(ca) == 2, "");

    // auto by value drops top-level const (but arrays by value are not allowed);
    // here we show constness with references:
    auto& r1 = a;   // r1 is int (&)[4]
    auto& r2 = ca;  // r2 is const int (&)[2]
    static_assert(std::is_same<decltype(r1), int (&)[4]>::value, "");
    static_assert(std::is_same<decltype(r2), const int (&)[2]>::value, "");

    // decltype preserves array type when used on an unparenthesized id-expression:
    decltype(a) copy = {1,2,3,4}; // still int[4]
    static_assert(std::is_same<decltype(copy), int[4]>::value, "");
    assert(copy[3] == 4);
}

// C++11+: type traits for arrays
static void array_type_traits()
{
    int a[3] = {};
    int b[2][5] = {};

    // extent: size of the first dimension for arrays
    static_assert(std::extent<decltype(a)>::value == 3, "");
    static_assert(std::extent<decltype(b), 0>::value == 2, "");
    static_assert(std::extent<decltype(b), 1>::value == 5, "");

    // rank: number of dimensions
    static_assert(std::rank<decltype(a)>::value == 1, "");
    static_assert(std::rank<decltype(b)>::value == 2, "");

    // remove_extent: peel one dimension
    using A0 = std::remove_extent<decltype(a)>::type; // int
    using B0 = std::remove_extent<decltype(b)>::type; // int[5]
    static_assert(std::is_same<A0, int>::value, "");
    static_assert(std::is_same<B0, int[5]>::value, "");
}

} // namespace cpp

int main()
{
    cpp::deduction_and_const();
    cpp::array_type_traits();
    std::cout << "arrays_templates_and_traits.cpp: OK\n";
    return 0;
}
