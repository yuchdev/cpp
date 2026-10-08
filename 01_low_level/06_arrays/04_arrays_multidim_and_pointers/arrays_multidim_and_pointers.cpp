#include <cassert>
#include <cstddef>
#include <iostream>
#include <type_traits>

#if defined(__has_include)
#  if __has_include(<mdspan>)
#    include <mdspan>
#  endif
#endif

namespace cpp {

static void layout_is_contiguous()
{
    int m[2][3] = {
        {1, 2, 3},
        {4, 5, 6}
    };

    int* flat = &m[0][0];
    assert(flat[0] == 1);
    assert(flat[3] == 4);

    int (*rowp)[3] = m;
    static_assert(std::is_same_v<decltype(rowp), int (*)[3]>);
    assert(&rowp[1][0] == &m[1][0]);

    int (*matp)[2][3] = &m;
    static_assert(std::is_same_v<decltype(matp), int (*)[2][3]>);

    assert(reinterpret_cast<char*>(matp + 1) ==
           reinterpret_cast<char*>(matp) + sizeof(m));
}

static int sum_2d(const int (&m)[2][3])
{
    int sum = 0;
    for (const auto& row : m)
        for (int value : row)
            sum += value;
    return sum;
}

template <std::size_t R, std::size_t C>
static int sum_2d_t(const int (&m)[R][C])
{
    int sum = 0;
    for (std::size_t r = 0; r < R; ++r)
        for (std::size_t c = 0; c < C; ++c)
            sum += m[r][c];
    return sum;
}

static void passing_2d()
{
    int m[2][3] = {{1, 2, 3}, {4, 5, 6}};

    assert(sum_2d(m) == 21);
    assert(sum_2d_t(m) == 21);

    // int** is a pointer to pointer. It neither carries the row stride nor
    // describes one contiguous int[2][3] object.
}

#if defined(__cpp_lib_mdspan) && __cpp_lib_mdspan >= 202207L
static void mdspan_cpp23()
{
    int storage[2][3] = {
        {1, 2, 3},
        {4, 5, 6}
    };

    // mdspan separates storage from multidimensional indexing. The view is
    // non-owning; the underlying six ints still live in the raw array.
    std::mdspan view{&storage[0][0], 2, 3};

    static_assert(decltype(view)::rank() == 2);

    assert(view.extent(0) == 2);
    assert(view.extent(1) == 3);
    assert(view[0, 2] == 3);
    assert(view[1, 0] == 4);

    view[1, 2] = 42;
    assert(storage[1][2] == 42);
}
#else
static void mdspan_cpp23()
{
    // C++23 demonstration activates when the standard library supplies <mdspan>.
}
#endif

} // namespace cpp

int main()
{
    cpp::layout_is_contiguous();
    cpp::passing_2d();
    cpp::mdspan_cpp23();

    std::cout << "arrays_multidim_and_pointers.cpp: OK\n";
    return 0;
}
