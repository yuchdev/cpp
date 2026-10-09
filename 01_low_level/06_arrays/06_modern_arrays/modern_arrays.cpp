#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <ranges>
#include <span>
#include <type_traits>

// modern_arrays.cpp
// Focus: preserving extent with std::array, viewing contiguous storage with
// std::span, and bridging raw arrays into ranges-oriented C++20 code.

namespace cpp {

template <typename T, std::size_t N>
constexpr std::size_t array_size(T (&)[N]) noexcept
{
    return N;
}

static void begin_end_size_and_ranges()
{
    int a[4] = {1, 2, 3, 4};

    int sum = 0;
    for (auto it = std::begin(a); it != std::end(a); ++it)
        sum += *it;
    assert(sum == 10);

    int sum2 = 0;
    for (int x : a)
        sum2 += x;
    assert(sum2 == 10);

    static_assert(array_size(a) == 4);
    static_assert(std::size(a) == 4);
    static_assert(std::ranges::size(a) == 4);

    static_assert(std::is_same_v<
        decltype(std::ranges::data(a)),
        int*>);
}

static void std_array_basics()
{
    std::array<int, 3> a = {1, 2, 3};

    static_assert(a.size() == 3);
    assert(a[0] == 1);

    int* p = a.data();
    p[1] = 42;
    assert(a[1] == 42);

    std::array<int, 0> empty{};
    static_assert(empty.size() == 0);

    // For N == 0, data() is not required to be nullptr. The useful contract is
    // empty() == true and begin() == end(), not a particular pointer value.
    assert(empty.begin() == empty.end());
}

static void to_array_cpp20()
{
    // CTAD here would produce std::array<const char*, 1>, because the literal
    // decays while deducing the std::array element type.
    std::array via_ctad{"abc"};
    static_assert(std::is_same_v<
        decltype(via_ctad),
        std::array<const char*, 1>>);

    // C++20 std::to_array preserves the literal's four characters, including
    // the terminating null character, in an owning std::array<char, 4>.
    constexpr auto text = std::to_array("abc");
    static_assert(std::is_same_v<
        std::remove_cv_t<decltype(text)>,
        std::array<char, 4>>);
    static_assert(text[3] == '\0');

    constexpr auto numbers = std::to_array<int>({1, 2, 3, 4});
    static_assert(numbers.size() == 4);
    static_assert(numbers[2] == 3);
}

static int sum_span(std::span<const int> values)
{
    int sum = 0;
    for (int value : values)
        sum += value;
    return sum;
}

static void span_cpp20()
{
    int raw[4] = {1, 2, 3, 4};
    std::array<int, 4> wrapped = {5, 6, 7, 8};

    std::span<int, 4> fixed{raw};
    static_assert(decltype(fixed)::extent == 4);
    // The extent is part of the span type. The standard does not require a
    // particular object size/layout for the span implementation.

    std::span<int> dynamic{wrapped};
    assert(dynamic.size() == 4);

    fixed[0] = 10;
    assert(raw[0] == 10);

    assert(sum_span(raw) == 19);
    assert(sum_span(wrapped) == 26);

    auto middle = dynamic.subspan(1, 2);
    assert(middle.size() == 2);
    assert(middle[0] == 6);
    assert(middle[1] == 7);

    // span is a view: destroying or invalidating the underlying storage makes
    // the span dangle. It carries bounds, not ownership.
}

static void bytes_view_cpp20()
{
    std::array<std::uint32_t, 2> words{0x01020304u, 0x05060708u};

    const auto bytes = std::as_bytes(std::span{words});
    using ByteSpan = std::remove_cv_t<decltype(bytes)>;

    static_assert(ByteSpan::extent == sizeof(words));
    assert(bytes.size_bytes() == sizeof(words));

    // std::as_bytes exposes object representation without inventing an
    // unrelated pointer type.
}

} // namespace cpp

int main()
{
    cpp::begin_end_size_and_ranges();
    cpp::std_array_basics();
    cpp::to_array_cpp20();
    cpp::span_cpp20();
    cpp::bytes_view_cpp20();

    std::cout << "modern_arrays.cpp: OK\n";
    return 0;
}
