#include <iostream>
#include <new>

#include "user_allooc.h"
#include "new_handler_mixture.h"

using std::new_handler;
using std::bad_alloc;

// In this file we will try to create a user-defined allocator

/*
It is possible to replace the standard new_handler, which is called on the situation of lack of memory
The standard `set_new_handler()` is called until memory is released or the program is finished

The user-defined new_handler can:
* make the required amount of memory available
* set another new_handler
* set a `nullptr` handler (i.e. `bad_alloc` exception) or throw `bad_alloc` manually
* terminate the program by `abort()` or `exit()`
*/

// Inherit from the mixture-class
class LargeObject : public NewHandlerSupport<LargeObject>
{
public:
    LargeObject() {}
    ~LargeObject() {}
private:
    // very large array, will throw out of memory
    char large_array[2147483646];
};

// Inherit from the mixture-class again
class SmallObject : public NewHandlerSupport<SmallObject>
{
public:
    SmallObject() {}
    ~SmallObject() {}
private:
    // everything ok should be here
    char large_array[42];
};

void test_placement_new_handler()
{
    std::cout << "No more memory!\n";
    throw bad_alloc();
}

void show_user_alloc()
{

    // different versions of new() and delete()

    // simple new overload
    {
        std::cout << "Test simple new overload" << '\n';
        user_alloc* x = new user_alloc();
        x->test();
        delete x;
    }

    // new for array overload
    {
        std::cout << "Test array new overload" << '\n';
        user_alloc* x = new user_alloc[10];
        x[1].test();
        delete[] x;
    }

    // placement new overload
    {
        std::cout << "Test placement new overload" << '\n';
        user_alloc* x = new user_alloc();
        x->test();
        x->~user_alloc();

        user_alloc* y = new(x)user_alloc();
        // could be directly called in functional form only!
        // will be implicitly called in case of exception
        user_alloc::operator delete(y, x);
        delete x;
    }

    // new_handler replacement new overload
    {
        std::cout << "Test placement new overload" << '\n';
        user_alloc* x = new (test_placement_new_handler)user_alloc();
        x->test();
        delete x;
    }
}

void show_new_handler()
{

    // Showcase of memory pool, mixture-class replacing new_handler
    // Let's try a very large object
    try {
        LargeObject::set_new_handler(NewHandlerSupport<LargeObject>::no_more_memory);
        LargeObject* x = new LargeObject();
        delete x;
    }
    catch (const std::bad_alloc& e) {
        std::cerr << "Lack of memory: " << e.what() << '\n';
    }

    // Let's try regular object
    try {
        SmallObject::set_new_handler(NewHandlerSupport<SmallObject>::no_more_memory);
        SmallObject* x = new SmallObject();
        delete x;
    }
    catch (const std::bad_alloc& e) {
        std::cerr << "Lack of memory: " << e.what() << '\n';
    }
}


int main()
{
    show_user_alloc();
    return 0;
}

