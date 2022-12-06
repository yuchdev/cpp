#pragma once

// Template class for unfolding short cycles
// Here is a class for calculating the scalar product of arrays
// Similar techniques are used in the Blitz++ numerical library

// Basic template
template <int DIM, typename T>
struct DotProduct
{
    static T result(T* a, T* b)
    {
        return (*a) * (*b) + DotProduct<DIM - 1, T>::result(a + 1, b + 1);
    }
};


// Partial specialization of recursion termination
template <typename T>
struct DotProduct<1, T>
{
    static T result(T* a, T* b)
    {
        return (*a) * (*b);
    }
};

// Scalar product
template <int DIM, typename T>
inline T dot_product(T* a, T* b)
{
    return DotProduct<DIM, T>::result(a, b);
}
