#include "templ.hpp"

template <class T>
T declared_only(T a, T b)
{
    return a + b;
}

template int declared_only<int>(int, int);
template double declared_only<double>(double, double);
