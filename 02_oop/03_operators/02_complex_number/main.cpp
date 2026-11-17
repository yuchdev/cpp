#include "complex.h"
#include "complex_matrix.h"

#include <iostream>

using std::cout;
using std::endl;

// Member compound-assignment operators (+=, -=, *=, /=) return *this by
// reference specifically so they chain; the free operator+/-/*// (declared
// as friends, defined inline in complex.h) are built on top of them by
// copying the left operand into a temporary and delegating.
void show_arithmetic()
{
    complex_t a(1.0, 2.0);
    complex_t b(3.0, -1.0);

    complex_t sum = a + b;
    complex_t product = a * b;
    complex_t quotient = a / b;

    cout << "a=(" << a.real() << "," << a.imag() << ") "
         << "b=(" << b.real() << "," << b.imag() << ")\n";
    cout << "a+b=(" << sum.real() << "," << sum.imag() << ")\n";
    cout << "a*b=(" << product.real() << "," << product.imag() << ")\n";
    cout << "a/b=(" << quotient.real() << "," << quotient.imag() << ")\n";

    try {
        complex_t zero(0.0, 0.0);
        complex_t broken = a / zero;
        (void)broken;
    }
    catch (const std::exception& e) {
        cout << "division by zero: " << e.what() << '\n';
    }
}

// complex_t defines conversion operators to both int and double, plus a
// constructor taking int -- intentionally giving the compiler several
// implicit-conversion paths to reason about at once.
void show_conversions()
{
    complex_t c(3.5, 9.0);

    double d = c;      // operator double()
    int i = c;          // operator int()
    complex_t from_int = 7; // complex_t(int)

    cout << "double(c)=" << d << " int(c)=" << i
         << " from_int=(" << from_int.real() << "," << from_int.imag() << ")\n";
}

// Prefix ++/-- return a reference to the already-modified object (cheap);
// postfix ++/-- must save the pre-modification value first and return it
// by value (one extra object, unavoidable for correct postfix semantics).
void show_increment_decrement()
{
    complex_t c(1.0, 1.0);

    complex_t pre = ++c;        // c becomes (2,2); pre aliases c
    complex_t post = c++;       // post is the OLD (2,2); c becomes (3,3)

    cout << "after ++c, c++: pre=(" << pre.real() << "," << pre.imag() << ") "
         << "post=(" << post.real() << "," << post.imag() << ") "
         << "c=(" << c.real() << "," << c.imag() << ")\n";
}

// complex_matrix provides both operator() and two chained operator[]
// calls for 2D indexing; both reach the same element through the same
// underlying _matrix[row][col] storage.
void show_matrix()
{
    complex_matrix m(2, 2);

    m(0, 0) = complex_t(1.0, 0.0);
    m(0, 1) = complex_t(0.0, 1.0);
    m[1][0] = complex_t(2.0, 2.0);
    m[1][1] = complex_t(3.0, -3.0);

    for (unsigned row = 0; row < 2; ++row) {
        for (unsigned col = 0; col < 2; ++col) {
            const complex_t& elem = m(row, col);
            cout << "(" << elem.real() << "," << elem.imag() << ") ";
        }
        cout << '\n';
    }

    // Deep copy: modifying the copy must not affect the original.
    complex_matrix copy(m);
    copy(0, 0) = complex_t(99.0, 99.0);
    cout << "original(0,0) after modifying copy: ("
         << m(0, 0).real() << "," << m(0, 0).imag() << ")\n";

    // Assignment between DIFFERENTLY-SIZED matrices: this is the case
    // that exercises whether operator= correctly updates the assigned-to
    // matrix's own dimensions, not just its data pointer.
    complex_matrix single_row(3);
    single_row = m;
    cout << "single_row now holds a " << 2 << "x" << 2 << " matrix: ("
         << single_row(1, 1).real() << "," << single_row(1, 1).imag() << ")\n";
}

int main()
{
    show_arithmetic();
    show_conversions();
    show_increment_decrement();
    show_matrix();
    return 0;
}
