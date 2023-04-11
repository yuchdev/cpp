#include <iostream>
using std::cout;
using std::endl;


// Non-type template parameters
// Array template with boundary check
// initialized by null and throws exceptions
template <typename T, int MAX>
class fixed_array
{
public:

    // It is handy to improve the readability introducing the following aliaces
    typedef T			value_type;
    typedef T& reference;
    typedef const T& const_reference;

    fixed_array();
    T& operator[](int index);
    const T& operator[](int index) const;

    // Templates can be friends and templates can have friends
    // The given friendly operator itself is not a template.
    // it just depends on its parameter. 
    // It must depend on it, otherwise it will break the ODR.
    // Define the friend functions in the template itself in order to include it into the template interface
    friend std::ostream& operator<< (std::ostream& os, const fixed_array<T, MAX>& f)
    {
        for (int i = 0; i < MAX; i++) {
            os << f._arr[i];
        }
        return os;
    }

private:
    T _arr[MAX];
};



template <typename T, int MAX>
fixed_array<T, MAX>::fixed_array() : _arr() {}

template <typename T, int MAX>
T& fixed_array<T, MAX>::operator [](int index)
{
    if ((index < 0) || (index >= MAX))
        throw std::out_of_range("Array ranges error");
    return _arr[index];
}

template <typename T, int MAX>
const T& fixed_array<T, MAX>::operator [](int index) const
{
    if ((index < 0) || (index >= MAX))
        throw std::out_of_range("Array ranges error");
    return _arr[index];
}

// Similar parameters can be used in functions as well
template <typename T, int VAL>
T add_value(const T& t)
{
    return t + VAL;
}

// A template argument can be used to define other types
template <typename T, T def_val> class cont;

// Nested templates
// The following options are possible:
// - field-template in a class (to be instantiated) or template
// - Template method, including the constructor, in a class or a template
// - nested class in a class or template
// - nested template in a class or template

// A template cannot be defined inside a function!
// It is also not possible to combine the declaration of a nested template class
// with the definition, it is unclear how to instantiate it


// 1.Class
class no_templ
{
public:
    // template field
    fixed_array<int, 100> _array;

    no_templ() : _intern(5) {}

    // Template constructor in a class
    // Template constructors and copy operators
    // never replace default constructors and operators
    template <typename T>
    no_templ(const T& t) : _intern(5) {}

    // a template nested in a class
    template <typename T2>
    struct internal_no_templ
    {
        internal_no_templ(const T2& t) : _param(t) {}
        T2 _param;
    } /*_internal - you can't define a template in a class;
      it is unclear what to instantiate */;

    internal_no_templ<int> _intern;
};

// 2.Template
template<typename T>
class templ
{
public:
    // template field
    fixed_array<T, 100> _array;

    // Template constructor in a template
    templ(const T& t) : _intern(t) {}

    // Template class nested in the template
    template <typename T2>
    struct internal_no_templ
    {
        internal_no_templ(const T2& t) : _param(t) {}
        T2 _param;
    };

    internal_no_templ<T> _intern;
};

// Inheritance
// * Template from a class
// * Class from template
// * Template from a template

// 1.Template from the class
// Static counter in a base class
template<typename T>
struct common_counter
{
    common_counter() { ++count; }
    ~common_counter() { --count; }
    static unsigned long count;
};

template<typename T>
unsigned long common_counter<T>::count = 0;


class Xclass : public common_counter<Xclass>
{
public:
    Xclass() {}
};

class Yclass : public common_counter<Yclass>
{
public:
    Yclass() {}
};

// 2. Class from template; see CRTP.h example

// 3.Template from template - see example in tmpl.h

// Template inheritance and type casting
// From the point of C++, two classes instantiated from the same template,
// are not connected by any inheritance relationship. For example:

class Shape {};

class Circle : public Shape {};

template <typename T>
class Set {};

void get_shape(Set<Shape*>& s)
{

}

void get_circle(Set<Circle*>& c)
{

    //casting error
    //get_shape(c); - uncomment for the error
}
