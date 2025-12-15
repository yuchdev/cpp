#pragma once
#include "complex.h"

/** @brief
The class complex_matrix implements a dynamic matrix of elements of complex_t type,
and is implemented exclusively for the purpose of demonstration
of memory management and operators overload
*/
class complex_matrix
{

public:

    /** @brief This constructor creates a row matrix
    * Constructors with the only parameter should be declared `explicit`,
    * unless implicit conversion is a part of design
        */
    explicit complex_matrix(const size_t count_x);

    /** @brief This constructor creates matrix of size X*Y */
    complex_matrix(const size_t count_x, const size_t count_y);

    /** @brief The copy constructor for this class must implement
    the so called "deep copy", as opposed to bitwise copying aka "shallow copying"
        */
    complex_matrix(const complex_matrix& c);

    /** @brief
    The assignment operator must also be "deep"
        */
    complex_matrix& operator=(const complex_matrix& c);

    // There is a more elegant way of assigning
    // using the constructor and swap operations

    // Destructors do not generate exceptions, or exceptions must not leave scope of destructor
    ~complex_matrix();

    // Overload 2 forms of `operator()` and `operator[]` for constant and non-constant version 
        // (non-constant version must return reference in odeer to be l-value)
    // The value returned by reference should also be constant
    complex_t* operator[](unsigned index);
    const complex_t* operator[](unsigned index) const;

    // In implementation of multidimensional matrices it is better to use operator() for indexing
    // instead of `operator[]`, as `operator[]` allows passing only one index
    complex_t& operator()(unsigned index_x, unsigned index_y);
    const complex_t& operator()(unsigned index_x, unsigned index_y) const;

    // Friends can be declared both a private or public part of the class, it doesn't matter.
    // Single class function or the entire class can be declared friend.
        // Although, in this case it is better to utilize class interface, composition or inheritance
    friend double complex_t::real() const;
    friend double complex_t::imag() const;

    friend class complex_t;

    // The friend class must be declared in the enclosing scope
    // The friend function must either be declared in the enclosing scope,
    // OR have an argument to this class (like in our binary operators)

protected:
    /** @brief Service function to clear memory without destroying the object */
    void clear();

private:
    size_t _count_x;
    size_t _count_y;
    complex_t** _matrix;
};
