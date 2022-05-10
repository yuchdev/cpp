#include <iostream>
#include "complex.h"
#include "smart_ptr.h"
using std::vector;

// The function shows the work of overloaded operators 
// and the implicit constructor
void show_overloads()
{


    // implicit constructor call
    complex_t c1 = 3;

    // explicit constructor calls
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

// The function shows overload of opperators utilizing control of dynamic memory
void show_dynamic()
{

    complex_matrix m1(2);
    complex_matrix m2(2, 2);

    m1(0, 0) = complex_t(1., 2.);
    m1(0, 1) = complex_t(3., 5.2);
    complex_t* pc = m2[1];

}

// Examples with operator {type} and smart pointer

int main()
{
    show_smart_ptr();
    show_vector_by_val();
    show_dynamic();
    show_overloads();
    show_enum_overload();
    return 0;
}
