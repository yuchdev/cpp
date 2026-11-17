#include "triangle.h"

namespace
{
    int max3(int a, int b, int c)
    {
        int m = a > b ? a : b;
        return m > c ? m : c;
    }
}

triangle::non_triangle_exception::non_triangle_exception(const char* msg, int a, int b, int c)
    : triangle_exception(msg)
    , _deficit(a + b + c - 2 * max3(a, b, c))
{
}

triangle::triangle(int a, int b, int c) : _a(a), _b(b), _c(c)
{
    if (a <= 0 || b <= 0 || c <= 0)
        throw wrong_param("side length must be positive", a <= 0 ? a : (b <= 0 ? b : c));

    if (a + b <= c || a + c <= b || b + c <= a)
        throw non_triangle_exception("sides do not satisfy the triangle inequality", a, b, c);
}

void triangle::throw_test() const
{
    throw unspecified_defect("throw_test: structural problem with no specific diagnosis");
}

void triangle::test_nothrow() const noexcept
{
    // Deliberately does nothing that can throw -- the "normal path"
    // counterpart to the test_throw_* methods below.
}

void triangle::test_throw_some1() const
{
    throw wrong_param("side became invalid after construction", -1);
}

void triangle::test_throw_some2() const
{
    throw non_triangle_exception("sides no longer satisfy the triangle inequality", _a, _b, _c);
}
