#include <iostream>
#include <bitset>
using std::cout;
using std::endl;

// Chapter 5
// typename keyword. X.template construct
// this-> usage
// Template class members. Template params.
// Default initialization. Literal usage.

// ------------------- typename keyword -------------------
// Keyword 'typename' is being used not only for declaring template params - types
// but also for pointing that construct declated with typename is a type

// The funtion prints elements of type T container
template <typename T>
void print_container(const T& coll)
{

    // Let's point that iterators are also types
    // applying typename keyword to iterator is a standard technique
    typename T::const_iterator pos;
    typename T::const_iterator end(coll.end());

    // Iterate
    pos = coll.begin();
    while (pos != end) {
        std::cout << *pos << endl;
        ++pos;
    }
}


// ------------------- template keyword -------------------
template<int N>
void print_bitset(const std::bitset<N>& b)
{

    // We need to clarify that b param is dependent on template
    cout << b.template to_string<char, char_traits<char>, allocator<char> >() << endl;

    // ...but some compilers allow omitting the keyword
    cout << b.to_string<char, char_traits<char>, allocator<char> >() << endl;
}

void test_bitset()
{
    bitset<10> b;
    print_bitset(b);
}

// ------------------- this usage -------------------
template <typename T>
class base
{
public:
    void exit()
    {
        cout << "base::exit" << endl;
    }
};

template <typename T>
class derived : public base<T>
{
public:
    void foo()
    {

        // Don't just call exit() methos

        // The compiter would never consider exit() from the base calss

        /*
        this->exit() makes compilers think that "this class must have an exit() method"
                Since this must-be is not defined until the moment of instantiation (probably base would be specialized
        in a specific manner), the rest of the semantic analysis is postponed for later.

        Similar hints for the compiler are Derived::exit() and Base::exit()

        Otherwise, the compiler tries to find the exit() name in the current context. And the current context is:
        — function body Derived<T>::g()
        — class declaration Derived<T> excluding unknown zone Base<T>
        — namespace :: where Derived is declared
        And the compiler finds fuch a name. It's a fucntion extern "C" void exit(int exitcode) from <cstdlib>
        */

        // Correct form
        this->exit();

        // Alternative form 1
        //Base<T>::exit();

        // Alternative form 2
        //using Base<T>::exit;    
        //exit();

        // Make it a rule to call template method this->member() explicitly
        // It could be necessary also for ensuring polymorphic behavior

    }
};


// ------------------- Template class members -------------------
// Template method or class could be itself a class member (both template or plain class)

template <typename T>
class copyable_stack
{
public:
    copyable_stack();
    void push(const T& p);
    void pop();
    const T& top() const;
    bool empty() const;

    // Template method of template class
    template<typename TCOPY>
    copyable_stack<T>& operator=(const copyable_stack<TCOPY>& s);

private:
    std::vector<T> _copyable_stack;
};

template <typename T>
copyable_stack<T>::copyable_stack() {}

template <typename T>
void copyable_stack<T>::push(const T& p)
{
    _copyable_stack.push_back(p);
}

template <typename T>
void copyable_stack<T>::pop()
{
    _copyable_stack.pop_back();
}

template <typename T>
const T& copyable_stack<T>::top() const
{
    return _copyable_stack.back();
}

template <typename T>
bool copyable_stack<T>::empty() const
{
    return _copyable_stack.empty();
}


// The instance of the template stack could not be copied
// to instance of another type
// E.g.:
// stack<int> a; 
// stack<double> b; 
// a=b

// We can define a template copyable_stack::operator=()
// where we copy stack elements one by one
// Internal and external templates have different declarations
template <typename T>
template <typename TCOPY>
copyable_stack<T>& copyable_stack<T>::operator=(const copyable_stack<TCOPY>& s)
{

    copyable_stack<TCOPY>  tmp(s);
    _copyable_stack.clear();

    // Copying one-by-one with type conversion, to be checked in compile-time
    while (!tmp.empty()) {
        _copyable_stack.push_back(static_cast<T>(tmp.top()));
        tmp.pop();
    }
    return *this;
}

// ------------------- Template params of template -------------------
// Template could be a template param
// E.g. stack with custom underlying container, which is defined as a template param
template <
    typename T,
    // As CONT defines class name rather than type, we use the keyword class
    // Typenames of unused params could be omittes (... typename = std::allocator)
    template <typename ELEM, typename = std::allocator<ELEM> > class CONT = std::deque >
class tmpl_stack
{
public:
    tmpl_stack();
    void push(const T& p);
    void pop();
    const T& top() const;
    bool empty() const;

    // Template method
    template<typename TCOPY, template <typename, typename> class CONT_COPY>
    tmpl_stack<T, CONT>& operator=(const tmpl_stack<TCOPY, CONT_COPY>& s);

private:
    CONT<T> _copyable_stack;
};

// template <typename ELEM, typename = std::allocator<ELEM> > - typical template declaration of a STL container
// now template argument could be passed as std::deque, not std::deque<T>

// Defenitions of template methods with template params
// (Names of unused params could be omitted)
#define TEMPLATE_METHOD template <typename T, template <typename, typename> class CONT>

TEMPLATE_METHOD
tmpl_stack<T, CONT>::tmpl_stack() {}

TEMPLATE_METHOD
void tmpl_stack<T, CONT>::push(const T& p)
{
    _copyable_stack.push_back(p);
}

TEMPLATE_METHOD
void tmpl_stack<T, CONT>::pop()
{
    _copyable_stack.pop_back();
}

TEMPLATE_METHOD
const T& tmpl_stack<T, CONT>::top() const
{
    return _copyable_stack.back();
}

TEMPLATE_METHOD
bool tmpl_stack<T, CONT>::empty() const
{
    return _copyable_stack.empty();
}


// The most complicated implementation of a template method from a template class
// conraining a template param
template <typename T, template <typename, typename> class CONT>
template <typename TCOPY, template <typename, typename> class CONT_COPY>
tmpl_stack<T, CONT>& tmpl_stack<T, CONT>::operator=(const tmpl_stack<TCOPY, CONT_COPY>& s)
{

    tmpl_stack<TCOPY, CONT_COPY>  tmp(s);
    _copyable_stack.clear();

    // Copying one-by-one with type conversion, to be checked in compile-time
    while (!tmp.empty()) {
        _copyable_stack.push_back(static_cast<T>(tmp.top()));
        tmp.pop();
    }
    return *this;
}

// ------------------- Default initialization -------------------
// Call T() ensures default initialization of a template param
// and zero initizlization for fundamental types
template <typename T>
class init_zero
{
public:
    // Default initialization of a template param
    init_zero() : _x() {}
private:
    T _x;
};

// ------------------- String literals as template arguments -------------------
// Converting an array into pointer happens only in case of passing the array by value

// 1. Version with pass-by-reference
template <typename T>
inline const T& str_max_ref(const T& a, const T& b)
{
    return (a > b) ? a : b;
}

// 2. Version with pass-by-value
template <typename T>
inline const T str_max_val(const T a, const T b)
{
    return (a > b) ? a : b;
}

// Let's see how it works
template <typename T>
inline void show_ref(const T& a)
{
    // Type of param
    cout << "str_max_ref() param type is: " << typeid(a).name() << endl;
}

template <typename T>
inline void show_val(const T a)
{
    // Type of param
    cout << "str_max_val() param type is: " << typeid(a).name() << endl;
}
