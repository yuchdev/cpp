#pragma once
#include <cstring>

// Differences of function template from the class template
// * overloading with plain functions is possible
// * arguments deduction is possible
// * partial specialization is impossible

// Overloading, specialization
// Plain overloaded function (needed for the example, 
// must be defined before the template for correct overloading)
inline const int max(const int a, const int b)
{
    return (a > b) ? a : b;
}

// Functions are given preference during overloading with templates
// If the template argument is a type, use typename T is a common form
// T is a template argument; a, b are function parameters
template <typename T>
inline const T& max(const T& a, const T& b)
{
    return (a > b) ? a : b;
}

// Specialization for C-strings (lexicographical predicate)
template <>
inline const char* const& max(const char* const& a, const char* const& b)
{
    return (strcmp(a, b) < 0) ? a : b;
}


// Template functions can be inline.

// One specialization is considered to be more specialized than another,
// if each list of arguments of the second specialization
// corresponds to the first specialization
template <typename T> class Vector;
template <typename T> class Vector<T*>;
template <> class Vector<void*>;


// Pointers to functions and methods
void first_func(int i)
{
    cout << "First func: " << i << endl;
}

void second_func(int i)
{
    cout << "Second func: " << i << endl;
}

// pointer to a void void_func_p(int) function
typedef void (*void_func_p)(int);

// method pointers, static and plain
struct holder
{
    void method(int i)
    {
        cout << "Method: " << i << endl;
    };
    static void stat_method(int i)
    {
        cout << "Static method: " << i << endl;
    }
};

typedef void (holder::* method_p)(int);

void show_func_pointers()
{

    // Pointer to a plain function
    void_func_p f = &first_func;
    f(5);
    f = &second_func;

    // Calling a function by its pointer
    f(6);
    (*f)(7);

    cout << "sizeof f: " << sizeof(f) << endl;

    // method pointer
    method_p m = &holder::method;
    holder h;

    cout << "sizeof m: " << sizeof(m) << endl;

    // Indirect call of a method using its pointer
        // `.*` or `->*` operations
    (h.*m)(8);

    // Possible only if holder object exists
    // call of a method pointer for a dynamic object
    holder* ph = new holder();
    (ph->*m)(9);
    delete ph;

    // A pointer to a method cannot be assigned to a function pointer
    //m = f; (uncomment for error)
    //f = &holder::method;

    // But on the static method you can
    // Pointer to static method is essentially a function pointer
    void_func_p ms = &holder::stat_method;
    ms(10);

    // Also, you cannot assign a pointer to a static method to a member pointer
    //method_p m1 = ms; (uncomment for error)
}
