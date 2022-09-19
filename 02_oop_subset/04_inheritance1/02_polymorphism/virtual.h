#pragma once
#include <iostream>
using std::cout;
using namespace std;

// Virtual destructors. 
// If destructor of base class is non-virtual, the result of polymorphic destruction is undefined
// Destructors can be non-virtual (or do not exist) for POD types

// Declare purely virtual destructors to make the class abstract
// The descendant does not inherit the destructor, hence the abstractness too

// If a derived class overloads a method, the entry in the virtual function table changes.
// If the derived class defined virtual methods that were not declared in the parent class,
// they are added to the end of the table.

// The way the VPTR and VTBL are stored largely depends on the compiler
// Usually, virtual function tables are static.
// That is, they are not constructed when each class instance is created.
// The entire hierarchy of polymorphic classes is available at the compilation stage, 
// so the compiler can build these tables in advance.
// At runtime, the class constructor implicitly creates the pointer to its table.

// Example with classes without virtual functions
// only early (static) binding is performed
struct Base
{
protected:
    int func(int i) { return 2 * i; }

public:

    // the protected function that defined in the base and derived classes
    // is called from this function
    int call_protected_function(int i) { return func(i) + 1; }
};

struct Derived : public Base
{
protected:
    // In the case of early, static binding
    // this function will never be called
    virtual int func(int i) { return i * i; }
};

struct Base_virt
{
protected:
    // Let's declare the function as virtual. The class will become polymorphic.
    // Function and virtuality will be inherited,
    // All derived classes will be polymorphic.
    virtual int func(int i) { return 2 * i; }

public:
    int call_protected_function(int i) { return func(i) + 1; }
    int call_protected_function_def(int i) { return func(i) + 1; }
};

struct Derived_virt : public Base_virt
{
protected:
    // In the case of late, dynamic binding
    // this function will be called for an instance of the Derived_virt class
    virtual int func(int i) { return i * i; }
};

/*
* Constructors and static functions cannot be virtual.
* The prototypes of inherited virtual functions must be the same
  (except for the value returned by reference or pointer).
* If the prototype differs in the parameter list, the function loses its virtuality.
* If a prototype differs in constancy or return value,
  there is a compilation error.
* Virtual function can be called as non-virtual, explicitly by the class name.
* Costs of virtuality are pointer size in every class instance,
  VTBL initialization on every type, and indirect function call.
*/

struct A_virt
{


    A_virt();
    virtual ~A_virt();

    /**	Execute function f on class creation and destruction
    *   The function f is virtual, but in constructor and destructor only static binding works
    */
    virtual ostream& f() const;
};

/** The same for B_virt */
struct B_virt : public A_virt
{

    B_virt();
    virtual ~B_virt();
    /** Execute function f on class creation and destruction */
    virtual ostream& f() const;
};

// An example of virtualization of an external function
// Pass in a reference to a base class as a function parameter
// As a result, virtualization works in the external function
inline ostream& operator<<(ostream& t, A_virt& c)
{
    return c.f();
}

// The virtual destructor is widely used for polymorphic cleanup
// The destructor can be purely virtual, but its pure virtuality
// differs from other methods

// Purely virtual functions have no body.
// In the table of virtual functions they are represented by a nullptr
// This function can't be called, or rather it can be, but it will crash the program

//In case of multiple inheritance class may contain more than 1 vtbl

struct v_base
{

    v_base();

    // Create a pure virtual destructor
    virtual ~v_base() = 0;

    // Pure virtual destructor can have a definition!
    // Pure virtual destructor is not inherited, hence,
    // does not make a derived class abstract (just like operator=)

    // Moreover, methods opf abstract classes can also be implemented
    // and called in derived classes
    // This may be needed in order to create some generic implementation
    // This is called "default implementation inheritance"
    void pure();

    // All cases of pure-virtual function implementation:
    // 1. Pure virtual destructor
    // 2. Implementation by default
    // 3. Common partial implementation (protected)
    // 4. Throw an exception if pure virtual call is possible
};

struct v_derived : public v_base
{
    v_derived();
    // Let's skip a destructor in a derived class
    // It's supposed to be abstract.
    // But destructors are not inherited, so abstractness is not inherited either.
    //~v_derived();
};


// A couple more important points:
// When a default parameter is specified in a virual function, it is inherited
// The type of the return value can be different, if it is a pointer or a reference,
// that can be casted to each other, so-called co-variant type
struct Base1
{
    // just a virtual function to indicate an overload
    virtual void f(int)
    {
        std::cout << "B::f(int)" << std::endl;
    }

    // just a virtual function to indicate an overload
    virtual void f(double)
    {
        std::cout << "B::f(double)" << std::endl;
    }

    // a virtual function to demonstrate inheritance of default values
    virtual void g(int i = 10)
    {
        std::cout << "B::g(int) " << i << std::endl;
    }

    // a virtual function to demonstrate return values of different type
    virtual A_virt* pf(int i = 1)
    {
        std::cout << "A_virt B::pf(int) " << i << std::endl;
        return new A_virt();
    }

    // Changing accessibility of virtual functions!
protected:
    virtual void open_me() const
    {
        std::cout << "Base1::open_me()" << std::endl;
    }
public:
    virtual void hide_me() const
    {
        std::cout << "Base1::hide_me()" << std::endl;
    }
};

struct Derived1 : Base1
{
    // TODO: repeat for static and dynamic binding
    //int f(int) {
    //	std::cout << "int Derived1::f()" << std::endl;
    //	return 1;
    //}

    // a virtual function to show inheritance of default values
    // (i = 20) overrides (i = 10) from the base
    void g(int i = 20)
    {
        std::cout << "D::g(int) " << i << std::endl;
    }

    // a virtual function to show different return types
    // the return pointer B_virt could be casted to A_virt
    virtual B_virt* pf(int i = 2)
    {
        std::cout << "B_virt* B::pf(int) " << i << std::endl;
        return new B_virt();
    }

public:
    virtual void open_me() const
    {
        std::cout << "Derived1::open_me()" << std::endl;
    }

protected:
    // this function could be called by the pointer to the base class!
    virtual void hide_me() const
    {
        std::cout << "Derived1::hide_me()" << std::endl;
    }
};
