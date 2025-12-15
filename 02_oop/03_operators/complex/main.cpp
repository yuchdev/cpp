#include <iostream>
#include "complex.h"
#include "smart_ptr.h"

using std::vector;

// The function demonstrates the overloaded operators and the effect of implicit constructor
void show_overloads()
{
    // implicit constructor call
    complex_t c1 = 3;

    // explicit constructor call
    complex_t c2(1., 2.5);
    complex_t c3(1.5, 3.5);

    c1 = c2 + c3;
    c1 = c2 - c3;
    c1 = c2 * c3;
    c1 = c2 / c3;

    c1 += c2;
    c2 -= c3;
    c1 *= c3;
    c3 /= c1;
}

// The function demonstrates overloading of opperators in a class utilizing dynamic memory
void show_dynamic()
{
    complex_matrix m1(2);
    complex_matrix m2(2, 2);

    m1(0, 0) = complex_t(1., 2.);
    m1(0, 1) = complex_t(3., 5.2);
    complex_t* pc = m2[1];
}


int main()
{
    show_overloads();
    show_dynamic();
    return 0;
}
