#include <cassert>
#include <cstddef>
#include <iostream>
#include <type_traits>

// arrays_templates_and_traits.cpp
// Focus: preserving array type and extent in templates, including C++20 traits.

namespace cpp {

template <typename T, std::size_t N>
constexpr std::size_t array_size(T (&)[N]) noexcept
{
    return N;
}

template <typename T, std::size_t N>
constexpr T& first(T (&array)[N]) noexcept
{
    static_assert(N > 0);
    return array[0];
}

static void deduction_and_const()
{
    int a[4] = {1, 2, 3, 4};
    const int ca[2] = {10, 20};

    static_assert(array_size(a) == 4);
    static_assert(array_size(ca) == 2);

    // Reference deduction preserves the complete array type, including extent
    // and element constness.
    auto& r1 = a;
    auto& r2 = ca;

    static_assert(std::is_same_v<decltype(r1), int (&)[4]>);
    static_assert(std::is_same_v<decltype(r2), const int (&)[2]>);

    first(a) = 42;
    assert(a[0] == 42);
    static_assert(std::is_same_v<decltype(first(ca)), const int&>);

    // decltype on an unparenthesized id-expression preserves the declared
    // array type rather than applying array-to-pointer conversion.
    decltype(a) copy = {1, 2, 3, 4};
    static_assert(std::is_same_v<decltype(copy), int[4]>);
    assert(copy[3] == 4);
}

static void array_type_traits()
{
    using OneDimensional = int[3];
    using Matrix = int[2][5];
    using UnknownBound = int[];

    static_assert(std::is_array_v<OneDimensional>);
    static_assert(std::is_array_v<Matrix>);

    // extent: size of each known dimension.
    static_assert(std::extent_v<OneDimensional> == 3);
    static_assert(std::extent_v<Matrix, 0> == 2);
    static_assert(std::extent_v<Matrix, 1> == 5);

    // rank: number of array dimensions.
    static_assert(std::rank_v<OneDimensional> == 1);
    static_assert(std::rank_v<Matrix> == 2);

    // remove_extent peels one dimension; remove_all_extents reaches the scalar.
    using MatrixRow = std::remove_extent_t<Matrix>;
    using MatrixElement = std::remove_all_extents_t<Matrix>;

    static_assert(std::is_same_v<MatrixRow, int[5]>);
    static_assert(std::is_same_v<MatrixElement, int>);

    // C++20 distinguishes arrays whose bound is part of the type from arrays
    // of unknown bound. Unknown-bound arrays appear in declarations such as
    // extern int values[] and in some low-level interfaces.
    static_assert(std::is_bounded_array_v<OneDimensional>);
    static_assert(std::is_bounded_array_v<Matrix>);
    static_assert(!std::is_bounded_array_v<UnknownBound>);

    static_assert(std::is_unbounded_array_v<UnknownBound>);
    static_assert(!std::is_unbounded_array_v<OneDimensional>);
}

} // namespace cpp

int main()
{
    cpp::deduction_and_const();
    cpp::array_type_traits();

    std::cout << "arrays_templates_and_traits.cpp: OK\n";
    return 0;
}
