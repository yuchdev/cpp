#pragma once
#include <iostream>

using std::cout;
using std::endl;

// This is where the reference counter class is stored 
// as well as strategy classes to define the details to adjust the smart pointer behavior
size_t* alloc_counter()
{
    return new size_t;
}

void free_counter(size_t* c)
{
    delete c;
}

// The reference counter class allocates memory for the counter outside the class (so that the class can be copied)
// Different counters can use different strategies, e.g. thread-safe counters with locks
class simple_ref_count
{
public:

    // Trivial constructor
    simple_ref_count() :_counter() {}

    // the default copy constructor is fine

    // initialize the counter (an object is created somewhere)
    template <typename T>
    void init(T*)
    {
        _counter = alloc_counter();
        (*_counter) = 1;
        cout << _counter << ", counter = 1" << endl;
    }

    // releasing the counter (somewhere the last copy of the object has been deleted)
    template <typename T>
    void dispose(T*)
    {
        free_counter(_counter);
        cout << _counter << ", dispose counter" << endl;
    }

    // increment (an object is copied somewhere)
    template <typename T>
    void increment(T*)
    {
        (*_counter)++;
        cout << _counter << ", counter = " << (*_counter) << endl;
    }

    // decrement (object is deleted somewhere)
    template <typename T>
    void decrement(T*)
    {
        (*_counter)--;
        cout << _counter << ", counter = " << (*_counter) << endl;
    }

    // check if 0
    template <typename T>
    bool is_zero(T*)
    {
        return ((*_counter) == 0);
    }


private:
    size_t* _counter;
};

// The standard removal strategy
struct standard_object_policy
{
    template <typename T>
    void dispose(T* ptr)
    {
        delete ptr;
    }
};


// The standard deleting an array of objects strategy
struct standard_array_policy
{
    template <typename T>
    void dispose(T* ptr)
    {
        delete[] ptr;
    }
};
