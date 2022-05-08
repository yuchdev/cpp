#include <iostream>
#include "simple_ptr.h"
#include "counting_ptr.h"

using std::cout;
using std::endl;

// The purpose of a class is to demonstrate the capabilities of smart pointers
class hold_me
{
    int i;
public:
    hold_me() :i()
    {
        cout << "construction" << endl;
    }
    hold_me(const hold_me& rhs) :i(rhs.i)
    {
        cout << "copy construction" << endl;
    }

    ~hold_me()
    {
        cout << "destruction" << endl;
    }
    void f() { i++; }
};

// A simple smart pointer without assignment
void show_simple_ptr()
{
    simple_ptr<hold_me> ptr(new hold_me());
    ptr->f();
    (*ptr).f();
}

// A function for passing a smart pointer with a reference counter by value
// While copying, the object itself is left untouched; only the counter is incremented and decremented
counting_ptr<hold_me> show_pass_ptr_copy(counting_ptr<hold_me> ptr)
{

    // at this point counter == 2
    // f() must be called here for the passed object
    // the function is non-constant, so the object is detouched
    ptr->f();

    // Create a new object
    counting_ptr<hold_me> new_ptr;

    // Just increment the refcount
    new_ptr = ptr;

    // non-constant operation! once again detouch the object
    new_ptr->f();

    // return by value
    return new_ptr;

    // destroying all the objects in the scope
}

// Demonstration of smart pointer with a reference counter
void show_refcounting_ptr()
{
    // initialize, counter = 1
    counting_ptr<hold_me> ptr(new hold_me());

    // pass by value, counter = 2
    counting_ptr<hold_me> new_ptr = show_pass_ptr_copy(ptr);

    // after exiting the function we have two objects, 1st counter==1, 2nd other counter==2
    // (one is disconnected after a non-constant operation is called)
    new_ptr->f();

    // destroying all the objects in the scope
    // counter = 1
    // counter = 0
}


int main()
{

    show_simple_ptr();
    show_refcounting_ptr();

    return 0;
}
