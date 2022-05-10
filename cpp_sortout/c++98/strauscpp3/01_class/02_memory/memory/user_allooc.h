#pragma once
#include <new>
#include <cstdio>

using std::new_handler;
using std::bad_alloc;

// In this header we demonstrate creation of the object using a custom allocator

// Class with overloaded new/delete operators
class user_alloc
{
public:
    user_alloc();

    // Without virtual destructors, `operator delete()` may not work properly
    // operator delete is unique in being a non-member or static member function that is dynamically dispatched
    // A type with a virtual destructor performs the call to its own delete from the most derived destructor
    virtual ~user_alloc();

    // 1. Plain new/delete overload
    static void* operator new(size_t s);
    static void operator delete(void* p);

    // 2. Array new/delete overload
    static void* operator new[](size_t s);
    static void operator delete[](void* p);


    // 3. Placement new/delete overload
    static void* operator new(size_t s, void* place);
    static void operator delete(void* p, void* place);

    // 4. New replacing new_handler
    static void* operator new(size_t s, new_handler p);
    static void operator delete(void* p, new_handler h);

    void test();
protected:

    // For the sake of simplicity, we use a single protected function
    // for `operator new()` and `operator new[]()`.
    // This works, because in the overloaded operator new (or new[]) we just allocate memory 
    // and don't do anything specific with constructed objects.
    // In practice, there can be difference; for example, in `operator new` we may want to check if
    // `s == sizeof(Type)`, in `operator new[]` we don't (as the array size is unknown)
    static void* universal_allocate(size_t s, new_handler handler = 0);

    // Same for memory release
    static void universal_free(void* p);

private:
    int _test;
};
