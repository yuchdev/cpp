#pragma once


// Initial pattern for raising N to the power of POW
template <int N, int POW>
struct Pow
{
    enum { result = N * Pow<N, POW - 1>::result };
};

// Partial specialization to complete recursion
template <int N>
struct Pow<N, 0>
{
    enum { result = 1 };
};
