#include <cassert>
#include <iostream>
#include <limits>
#include <new>

#include "user_alloc.h"
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

// Inherit from the mixture-class. Note what is NOT here: a deliberately
// huge member array to force an allocation failure "by sheer size". On a
// modern 64-bit system with virtual-memory overcommit, a multi-gigabyte
// allocation can succeed immediately (no physical page is touched until
// something writes to it), so that classic textbook trick is no longer
// reliable -- see show_new_handler() below for how this example actually
// forces the failure deterministically instead.
class LargeObject : public NewHandlerSupport<LargeObject>
{
public:
    LargeObject() {}
    ~LargeObject() {}
};

// Inherit from the mixture-class again
class SmallObject : public NewHandlerSupport<SmallObject>
{
public:
    SmallObject() {}
    ~SmallObject() {}
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

    // new_handler replacement new overload. sizeof(user_alloc) is tiny,
    // so a plain `new (handler) user_alloc()` would never actually fail
    // and the handler would never run -- calling operator new directly
    // with an explicit, impossible size is the only deterministic way to
    // prove the handler-replacement path actually works, as opposed to
    // "happened not to be tested".
    {
        std::cout << "Test placement new overload" << '\n';
        try {
            void* p = user_alloc::operator new(
                std::numeric_limits<size_t>::max() / 2, test_placement_new_handler);
            (void)p;
            assert(false); // unreachable: the handler always throws
        }
        catch (const bad_alloc&) {
            std::cout << "handler-replacement new correctly threw bad_alloc\n";
        }
    }
}

void show_new_handler()
{
    // Showcase of the CRTP mixture-class replacing new_handler per class.
    // Force LargeObject's mixin-provided operator new to request a size
    // no real system can satisfy, instead of relying on sizeof(LargeObject)
    // being "large enough to fail" by itself -- see the comment on
    // LargeObject's declaration for why that no longer works reliably.
    try {
        LargeObject::set_new_handler(NewHandlerSupport<LargeObject>::no_more_memory);
        void* p = LargeObject::operator new(std::numeric_limits<size_t>::max() / 2);
        (void)p;
        assert(false); // unreachable: no_more_memory() always throws
    }
    catch (const std::bad_alloc& e) {
        std::cerr << "Lack of memory: " << e.what() << '\n';
    }

    // SmallObject's allocation genuinely, normally succeeds -- its own
    // per-class handler is installed but is never actually invoked.
    SmallObject::set_new_handler(NewHandlerSupport<SmallObject>::no_more_memory);
    SmallObject* x = new SmallObject();
    delete x;
}

int main()
{
    show_user_alloc();
    show_new_handler();
    return 0;
}

