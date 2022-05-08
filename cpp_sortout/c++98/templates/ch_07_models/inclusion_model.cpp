#include "include_model.h"

// We cannot define methods of a template class here in *.cpp
// because it is not yet known at compile time
// what type of template instance has to be instantiated

#if 0
template <typename T>
include_model<T>::include_model(const T& t) : _x(t) {}

template <typename T>
include_model<T>::~include_model(void) {}

template <typename T>
T include_model<T>::test() const
{
    return _x;
}
#endif
