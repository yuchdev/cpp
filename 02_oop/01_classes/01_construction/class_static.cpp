#include "construct.h"

#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

SimpleClass sc_global;

void show_construct_destruct()
{
    std::cout << std::endl << "Create automatic objects" << std::endl;

    // 1. Named object on stack.
    {
        SimpleClass c;
        SimpleClass d;
        {
            SimpleClass c1(c);
        }
        NonCopyClass nc;
    }
    // Created in order c-d-c1-nc, destroyed in order c1-nc-d-c.

    // 2. Dynamic object in heap (new/delete) -- SimpleClass has a
    // user-declared default constructor, so () vs no-() makes no
    // difference for it; see show_aggregate_init_forms() for the case
    // where it does.
    std::cout << std::endl << "Create dynamic objects" << std::endl;
    SimpleClass* pc1 = new SimpleClass();
    SimpleClass* pc2 = new SimpleClass;
    delete pc1; pc1 = nullptr;
    delete pc2; pc2 = nullptr;

    // 3. Non-static class member.
    std::cout << std::endl << "Create non-static member" << std::endl;
    {
        SimpleClass c1;
    }

    // 4. Object-element of a C array.
    std::cout << std::endl << "Create objects array " << std::endl;
    SimpleClass arr_c[5];
    SimpleClass arr_1[] = { SimpleClass(2), SimpleClass(sc_global) };
    SimpleClass arr_2[] = { 1, 2 }; // implicit int -> SimpleClass conversion

    // 5. Local static object.
    std::cout << std::endl << "Create local static objects " << std::endl;
    create_static(0);
    create_static(1);
    create_static(2);

    int st = return_static();
    (void)st;

    // 6. Global/namespace-scope object: sc_global, above, already ran its
    // constructor before main() started.

    // 7. Temporary object.
    std::cout << std::endl << "Create temporary object " << std::endl;
    SimpleClass(5);

    // 8. Object created by a user-defined allocator: see 02_memory/.

    // 9. Union member: not applicable -- a union cannot have members with
    // constructors or destructors at all.
}

// `new T()` value-initializes; `new T` default-initializes. For a class
// with a user-provided default constructor (SimpleClass, SimpleStruct)
// these end up doing the same thing -- the constructor runs either way.
// The difference is only observable for a type with NO user-declared
// constructor at all: an aggregate.
void show_aggregate_init_forms()
{
    AggregateOfInts* zeroed = new AggregateOfInts(); // value-init: every member zeroed
    assert(zeroed->a == 0 && zeroed->b == 0);
    delete zeroed;

    AggregateOfInts* indeterminate = new AggregateOfInts; // default-init: members left indeterminate
    // Deliberately not read or asserted on: an indeterminate int's value
    // is unspecified, and the point stands without having to read it.
    delete indeterminate;
}

// A local static's first-use initialization is guaranteed thread-safe
// since C++11 -- no caller-side locking needed, and no risk of running
// the initializer twice even if multiple threads race to be first. Before
// C++11 this was implementation-defined; some compilers got it wrong.
void show_local_static_is_thread_safe()
{
    constexpr int n_threads = 8;
    std::vector<int*> addresses(n_threads, nullptr);
    std::vector<std::thread> threads;

    for (int i = 0; i < n_threads; ++i) {
        threads.emplace_back([&addresses, i] {
            addresses[i] = &return_static(); // first-use race, by construction
        });
    }
    for (auto& t : threads)
        t.join();

    for (int i = 1; i < n_threads; ++i)
        assert(addresses[i] == addresses[0]); // exactly one instance, however many racers
}

int main()
{
    show_construct_destruct();
    show_forward_declared_by_value();
    show_aggregate_init_forms();
    show_local_static_is_thread_safe();
    return 0;
}
