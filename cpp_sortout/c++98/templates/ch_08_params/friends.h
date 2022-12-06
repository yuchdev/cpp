#pragma once
// Templates and friends
// 1. Friend function declaration could be the only defenition of the objest
// (does not available anywhere except the class itself)
// 2.Declaration of the friend function could be also a defenition

// - Friend class declaration can't be defenitions
// - It's possible to befriend a specific template instance
// - Friend template should be visible in the defenition point, unlike plain class
// Examples of friend functions:

// Overloading of template and plain function
void comrade(void*) {}

template <typename T>
void comrade(T) {}

template <typename T>
class comrade_t;

class comrades
{
public:
    // Declaring the friend is also a function defenitions
    friend void comrade(int) {}

    // plain friend function
    friend void comrade(void*);

    // template function (argument deduction)
    friend void comrade(int);

    // template function (arguments defined explicitly)
    friend void comrade<double>(double);

    // Declaring the friend template function is also a template defenitions
    template <typename T>
    friend T comrade(const T t)
    {
        return t;
    }

    // Declare friend class
    template <typename T>
    friend class comrade_t;
};

// It's possible to declate friend template and specialization of template
template <typename T>
class Node;

template <typename T>
class Node2;


template <typename T>
class Tree
{
    friend class Node<T>;
    friend class Node2<int>;
};

// It's important to understand, you can't define a friend function in a template class
// otherwise it would violate ODR
template<typename T>
class createor
{
public:
    createor() {}

    // Uncomment for error:
    // error C2084: function 'void createor<T>::appear(void)' already has a body
    // friend void appear(){ int i = 1; }

    // The correct way to do it is make a friend function depend on a template type
    friend void appear(createor<T>*) { int i = 1; }
};

// Here, if we leave defenition of a friend function inside the template class, 
// it will be defined twice, for every type
void test_friends_odr()
{
    createor<int> a;
    createor<double> b;
}
