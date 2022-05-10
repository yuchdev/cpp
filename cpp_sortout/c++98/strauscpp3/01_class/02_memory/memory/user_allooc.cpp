#include "user_allooc.h"
//#include "construct.h"

#include <cstdio>
#include <iostream>

using namespace std;


user_alloc::user_alloc(void) :_test(0)
{

    // representation in memory
    // vptr			test int	
    // 20 ea 87 00	00 00 00 00	
    cout << "user_alloc()" << endl;
}

user_alloc::~user_alloc(void)
{
    cout << "~user_alloc()" << endl;
}

void user_alloc::test()
{
    cout << "test = " << _test << endl;
}

void* user_alloc::universal_allocate(size_t s, new_handler handler/* = 0*/)
{

    void* p = 0;

    // When implementing the `operator new()`, there are a few simple rules to follow

    // 1. Checking for 0.
    // Standard trick is to substitute 0 for 1 or leave
    if (0 == s) {
        s = 1;
    }

    new_handler oldHandler = 0;
    if (handler)
        oldHandler = std::set_new_handler(handler);

    // 2. Handling a lack of memory situation
    while (true) {
        p = malloc(s);
        if (p) {
            break;
        }
        else {
            // a trick to receive the current global new_handler
            // set it to nullptr and quickly return it back
            new_handler globalHandler = set_new_handler(nullptr);
            set_new_handler(globalHandler);
            if (0 == globalHandler) {
                // do not call the handler, just throw an exception
                throw bad_alloc();
            }
            else {
                // return global handler as new_handle()
                set_new_handler(globalHandler);

                // call it
                globalHandler();
            }

        }
    }
    if (handler)
        std::set_new_handler(oldHandler);
    return p;
}

void user_alloc::universal_free(void* p)
{
    // operator delete() olso must guarantee a check for nullptr
    if (0 == p)
        return;
    free(p);
}

void* user_alloc::operator new(size_t s)
{
    return universal_allocate(s);
}

void user_alloc::operator delete(void* p)
{
    universal_free(p);
}


void* user_alloc::operator new[] (size_t s) {
    return universal_allocate(s);
}

void user_alloc::operator delete[] (void* p) {
    universal_free(p);
}


// Overloaded form of the placement operator new
void* user_alloc::operator new(size_t s, void* place)
{
    printf("placement new\n");
    return place;
}

// Overloaded form of the placement `operator delete()`
// (Could be called directly only in its functional form)
void user_alloc::operator delete(void* p, void* place)
{
    // do nothing
    printf("placement delete\n");
}

// 4. Overloaded form of `operator new`, replacing new_handler
void* user_alloc::operator new(size_t s, new_handler p)
{
    return universal_allocate(s, p);
}

void user_alloc::operator delete(void* p, new_handler h)
{
    universal_free(p);
}
