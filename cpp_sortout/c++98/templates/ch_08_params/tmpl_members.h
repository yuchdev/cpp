#pragma once
#include <iostream>
using std::cout;
using std::endl;
// --- Demonstration of template class members --- 
// 1.Member functions
// 2.Nested classes
// 3.Static members

template <typename T>
class tmpl_members
{
public:
    tmpl_members(const T& v);

    // Plain method
    void open();

    // Template method
    template <typename T2>
    void copy(const T2& t);

    // Static member
    static double s_dbl;

    // Nested class
    class self
    {
    public:
        self(int i);
        int _i;
    } _self;

    // Nested template
    template <typename T3>
    class self_tmpl
    {
    public:
        self_tmpl(const T3& t);
        T _t;
    };

    self_tmpl<T> _self_tmpl;
private:
    T _val;
};

// Defining a constructor (calls constructors for nested classes)
template <typename T>
tmpl_members<T>::tmpl_members(const T& v) : _self(5), _self_tmpl(v), _val(v) {}

// Define plain method
template <typename T>
void tmpl_members<T>::open()
{

}

// Define template method
template <typename T>
template <typename T2>
void tmpl_members<T>::copy(const T2& t)
{

}

// Define static member
template <typename T>
double tmpl_members<T>::s_dbl = 0.;

// Define constructor for nested class
template <typename T>
tmpl_members<T>::self::self(int i) : _i(i) {}

// Define the nested template method
template <typename T>
template <typename T3>
tmpl_members<T>::self_tmpl<T3>::self_tmpl(const T3& t) : _t(t) {}

// --- Demonstration of virtual methods in templates --- 
template <typename T>
class base
{
public:
    // the plain method can be virtual in the pattern
    virtual void open(const T& t)
    {
        cout << "base::open " << typeid(t).name() << endl;
    }
    // ...but template can not
};

template <typename T>
class derived : public base<T>
{
public:
    // method will be called virtually even for base<double>
    virtual void open(const T& t)
    {
        cout << "derived::open " << typeid(t).name() << endl;
    }
};

// ------------------- Template linkage -------------------

// Unlike classes, templates are not allowed to have the same name as other objects
// Templates cannot have extern "C" calling conventions, but they can have 
// non-standard calling conventions extern "X"

// Templates have external linkage, except those declared as static
template <typename T>
void external();	// external

template <typename T>
static void internal();	// internal
