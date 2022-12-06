#pragma once
#include "if_then_else.h"

// Initial tenplate for calculating the integer square root
// Root of an integer is searched by the binary search and direct comparison
// Numbers without an integer root return -1
// For the example N=16 the order of calculations is as follows
// Sqrt<16, 1, 16>
// Sqrt<16, 1, 8>
// Sqrt<16, 1, 4>
// Sqrt<16, 3, 4>
// Sqrt<16, 4, 4> => sqrt(16) = 4

template <int N, int L0 = 1, int HI = N>
struct Sqrt
{
    enum { mid = (L0 + HI + 1) / 2 };

    typedef typename
        IfThenElse<
        (N < (mid* mid)),
        Sqrt<N, L0, mid - 1>,
        Sqrt<N, mid, HI>
        >::Result CalculatedT;

    enum { result = CalculatedT::result };
};

template <int I>
struct TypeWrapper
{
    enum { result = I };
};


// Partial specialization to complete the recursion
// If upper and lower intervals meet each other, we have the result
template <int N, int M>
struct Sqrt<N, M, M>
{

    typedef typename
        IfThenElse <
        (N == (M * M)),
        typename TypeWrapper<M>,
        typename TypeWrapper<-1>
        >::Result CalculatedT;


    enum { result = CalculatedT::result };
};
