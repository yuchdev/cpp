#pragma once
#include <iostream>
using std::cout;
using std::endl;

// Chapter 16.4 Nicolai Josuttis
// Parameterized Virtuality

// Implement the virtuality of method as a param
class non_virtual {};

class my_virtual
{
public:
    virtual void foo() {}
};

template <typename VBase>
class Base : private VBase
{
public:

    // The virtuality of foo() method depends on which class is passed as a parameter
    // as a base - with or without virtuality.
    // Inheritance itself can be private - the implementation is not important
    void foo() { cout << "Base::foo" << endl; }
};

template <typename IsVirtual>
class Derived : public Base<IsVirtual>
{
public:
    // The virtuality of foo() method depends on which class is passed as a parameter
    // as a base - with or without virtuality.
    void foo() { cout << "Derived::foo" << endl; }
};
