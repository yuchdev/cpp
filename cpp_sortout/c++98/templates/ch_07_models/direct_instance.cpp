#include "direct_instance.h"

// In void show_direct_instance() function
// instances of direct_instance<int> and <double> are required
// Mention them explicitly before the class definition
template class direct_instance<int>;
template class direct_instance<double>;

// You can instantiate an entire class or a single function
// If the class has already been instantiated, the function cannot be instantiated

template <typename T>
direct_instance<T>::direct_instance(const T& t) : _x(t) {}

template <typename T>
direct_instance<T>::~direct_instance(void) {}

template <typename T>
T direct_instance<T>::test() const
{
    return _x;
}
