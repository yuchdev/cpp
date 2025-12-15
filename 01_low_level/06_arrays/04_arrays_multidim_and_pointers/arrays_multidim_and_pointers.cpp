#include <cassert>
#include <cstddef>
#include <iostream>
#include <type_traits>

// arrays_multidim_and_pointer_to_array.cpp
// Focus: multi-dimensional arrays, contiguous layout, pointer-to-array,
// and why int** != int[R][C].

namespace cpp {

static void layout_is_contiguous()
{
    int m[2][3] = {
        {1,2,3},
        {4,5,6}
    };

    // Row-major contiguous storage:
    int* flat = &m[0][0];
    assert(flat[0] == 1);
    assert(flat[3] == 4);

    // The type of m is int[2][3], and m decays to pointer to its first row: int (*)[3]
    int (*rowp)[3] = m;
    static_assert(std::is_same<decltype(rowp), int (*)[3]>::value, "");

    // rowp + 1 moves by a whole row (3 ints)
    assert(&rowp[1][0] == &m[1][0]);

    // Pointer-to-array to the whole 2x3 matrix:
    int (*matp)[2][3] = &m;
    static_assert(std::is_same<decltype(matp), int (*)[2][3]>::value, "");

    // matp + 1 jumps past the whole matrix object
    assert(reinterpret_cast<char*>(matp + 1) == reinterpret_cast<char*>(matp) + sizeof(m));
}

// Passing 2D array to a function: second dimension must be known in the type.
static int sum_2d(const int (&m)[2][3])
{
    int s = 0;
    for (int r = 0; r < 2; ++r)
        for (int c = 0; c < 3; ++c)
            s += m[r][c];
    return s;
}

// Alternative: template on sizes
template <std::size_t R, std::size_t C>
static int sum_2d_t(const int (&m)[R][C])
{
    int s = 0;
    for (std::size_t r = 0; r < R; ++r)
        for (std::size_t c = 0; c < C; ++c)
            s += m[r][c];
    return s;
}

static void passing_2d()
{
    int m[2][3] = {{1,2,3},{4,5,6}};
    assert(sum_2d(m) == 21);
    assert(sum_2d_t(m) == 21);

    // Note: int** is a pointer to pointer, not a contiguous 2D array type.
    // You cannot safely pass int[2][3] as int**.
}

} // namespace cpp

int main()
{
    cpp::layout_is_contiguous();
    cpp::passing_2d();
    std::cout << "arrays_multidim_and_pointer_to_array.cpp: OK\n";
    return 0;
}
