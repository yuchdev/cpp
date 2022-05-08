#pragma once
#include "simple_array.h"
#include "expressions.h"

// The Rep parameter in this template can be either an array or 
// "delayed calculations" template, 
// which is validated only when operator[] is called
template <typename T, typename Rep = SArray<T> >
class Array
{
private:
    // Data encapsulation (array representation)
    Rep expr_rep;
public:

    // Array of a said size
    explicit Array(size_t s) : expr_rep(s) {}

    // Array from the existing representation
    explicit Array(Rep const& r) : expr_rep(r) {}

    // Assign the same type
    Array& operator=(Array const& rhs)
    {

        print_debug("=");
        assert(size() == rhs.size());
        for (size_t i = 0; i < size(); ++i) {
            expr_rep[i] = rhs[i];
        }
        return *this;
    }

    // Assign the other type
    template <typename T2, typename Rep2>
    Array& operator=(Array<T2, Rep2> const& rhs)
    {

        print_debug("=");
        assert(size() == rhs.size());
        for (size_t i = 0; i < size(); ++i) {
            // each time [] is accessed, the expression would be calculated once
            expr_rep[i] = rhs[i];
        }
        return *this;
    }

    // thesize of representation
    size_t size() const { return expr_rep.size(); }

    // accessors
    T operator[](size_t i) const { return expr_rep[i]; }

    T& operator[](size_t i) { return expr_rep[i]; }

    Rep& rep() { return expr_rep; }

    Rep const& rep() const { return expr_rep; }
};

// Let's implement overloaded vector arithmetic functions
// now, due to the expression templates, some of calculations will be folded

// Vector addition (return by value)
template <typename T, typename R1, typename R2>
Array<T, A_Add<T, R1, R2> >		// return value
operator+(Array<T, R1> const& a, Array<T, R2> const& b)
{
    // Again, here we do not calculate anything, but rather create an expression template,
    // calculating the requested value it by calling in operator[]
    return Array<T, A_Add<T, R1, R2> >(A_Add<T, R1, R2>(a.rep(), b.rep()));
}

// Vector multiplication (return by value)
template <typename T, typename R1, typename R2>
Array<T, A_Mult<T, R1, R2> >		// return value
operator*(Array<T, R1> const& a, Array<T, R2> const& b)
{

    return Array<T, A_Mult<T, R1, R2> >(A_Mult<T, R1, R2>(a.rep(), b.rep()));
}

// Vector multiplied by a scalar (return by value)
template <typename T, typename R2>
Array<T, A_Mult<T, A_Scalar<T>, R2> >
operator*(T const& a, Array<T, R2> const& b)
{

    return Array<T, A_Mult<T, A_Scalar<T>, R2> >(A_Mult<T, A_Scalar<T>, R2>(A_Scalar<T>(a), b.rep()));
}
