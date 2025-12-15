#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <type_traits>

// arrays_modern_helpers_cpp11_to_cpp17.cpp
// Focus: C++11+ helpers that make raw arrays safer without changing storage model:
// std::begin/end, range-for, std::array, and bridging via .data().

namespace cpp {

template <typename T, std::size_t N>
constexpr std::size_t array_size(T (&)[N]) noexcept { return N; }

static void begin_end_and_range_for()
{
    int a[4] = {1,2,3,4};

    // C++11: std::begin/std::end work for raw arrays
    int sum = 0;
    for (auto it = std::begin(a); it != std::end(a); ++it) sum += *it;
    assert(sum == 10);

    // Range-for works directly on raw arrays without decay:
    int sum2 = 0;
    for (int x : a) sum2 += x;
    assert(sum2 == 10);

    // Size can be deduced:
    static_assert(array_size(a) == 4, "");
}

static void std_array_basics()
{
    // std::array<T,N> is a thin wrapper holding T[N] as a member.
    // It does not decay, carries size, and is copyable/assignable.
    std::array<int, 3> a = {1,2,3};
    static_assert(a.size() == 3, "");
    assert(a[0] == 1);

    // .data() gives T* for interop with C APIs
    int* p = a.data();
    p[1] = 42;
    assert(a[1] == 42);

    // Note: std::array<int,0> exists and is well-formed in C++11+.
    std::array<int, 0> empty{};
    assert(empty.size() == 0);
}

} // namespace cpp

int main()
{
    cpp::begin_end_and_range_for();
    cpp::std_array_basics();
    std::cout << "arrays_modern_helpers_cpp11_to_cpp17.cpp: OK\n";
    return 0;
}
