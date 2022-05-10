/** @brief
Chapter 13 of Straustrup.
Simple templates.
Definition, instantiation, parameters, equivalence, type checking
Function templates.
Algorithm parametrization templates.
Specialization. Inheritance and templates. Type casting of templates.
*/

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

// an abstract template class
// can be used as a regular class
// to create a generic interface
// Template can also inherit from a regular class.
template<typename Symbols>
class compare_str
{
public:
    virtual bool compare(const basic_string<Symbols>& str1, const basic_string<Symbols>& str2) = 0;
};

// Derived template classes with virtual methods
template<typename Symbols>
class lexigraphical : public compare_str<Symbols>
{
public:
    virtual bool compare(const basic_string<Symbols>& str1, const basic_string<Symbols>& str2);
};

template<typename Symbols>
class no_case : public compare_str<Symbols>
{
public:
    virtual bool compare(const basic_string<Symbols>& str1, const basic_string<Symbols>& str2);
};

// methods implementation
template<typename Symbols>
bool lexigraphical<Symbols>::compare(const basic_string<Symbols>& str1, const basic_string<Symbols>& str2)
{
    return str1 > str2;
}

template< typename Symbols >
bool no_case<Symbols>::compare(const basic_string<Symbols>& str1, const basic_string<Symbols>& str2)
{
    std::basic_string<Symbols> nocase1, nocase2;
    std::transform(str1.begin(), str1.end(), std::back_inserter(nocase1), ::tolower);
    std::transform(str2.begin(), str2.end(), std::back_inserter(nocase2), ::tolower);
    return nocase1 > nocase2;
}

// Template class, in which 2 arguments are passed
// type of symbol and comparison type
// The latter has a default value
template<typename Symbols, typename Comparer = lexigraphical<Symbols> >
class container
{
    basic_string<Symbols> _str;
    Comparer _c;
public:
    container(basic_string<Symbols>& str);
    void compare(basic_string<Symbols>& str);
};

template<typename Symbols, typename Comparer>
container<Symbols, Comparer>::container(basic_string<Symbols>& str) :
    _str(str)
{
}

// Use the passed class type to compare strings
template<typename Symbols, typename Comparer>
void container<Symbols, Comparer>::compare(basic_string<Symbols>& str)
{
    _c.compare(str, _str);
}

// Example of partial specialization
// Stack for storing pointers only
// Inherits the interface from my_stack<void*>
// In order to define the type of elements as a pointer
// let's explicitly specify the my_stack<T*> template argument
// Specialization is an "overload" for template classes

// Inheriting of partial specialization from the full specialization to reduce code duplication
// is a pattern of template programming.
// Without it, specialized classes are not linked by any relationships, including inheritance
template <typename T>
class my_stack
{
public:
    my_stack()
    {
        cout << "general stack<T>" << endl;
    }
    // methods here
private:
    std::vector<T> _stack1;
};


template <>
class my_stack<void*>
{
public:
    my_stack()
    {
        cout << "Full specialization <void*>" << endl;
    }
    // methods here
protected:
    std::vector<void*> _stack1;
};


template <typename T>
class my_stack<T*> : public my_stack<void*>
{
public:
    typedef my_stack<void*> base;
    my_stack() : base()
    {
        cout << "Pointers stack<T*>" << endl;
    }
    // methods here
};
