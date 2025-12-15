#include <iostream>
#include "construct.h"

SimpleClass sc_global;

void show_construct_destruct()
{
    std::cout << std::endl << "Create automatic objects" << std::endl;
 
    // 1.Named object on stack
    // If the object contains constants and references, it can't be constructed by default
    {
        SimpleClass c;
        SimpleClass d;
        {
            SimpleClass c1(c);
        }
        NonCopyClass nc;
    }
    // Created in order c-d-c1-nc, destroyed in order c1-nc-d-c

    // 2. Dynamic object in heap (new/delete).
    std::cout << std::endl << "Create dynamic objects" << std::endl;

    // If the class has only automatic default c-tor
    // this calling form с() will zero the memory...
    SimpleClass* pc1 = new SimpleClass();
    // ... but this won't
    SimpleClass* pc2 = new SimpleClass;

    // For the raw pointer, nullptr is 'unused' sign
    delete pc1; pc1 = 0;
    delete pc2; pc2 = 0;

    // 3. Non-static class member
    std::cout << std::endl << "Create non-static member" << std::endl;
    // Default c-tor for internal
    {
        SimpleClass c1;
    }


    // 4. Object-element of C-array
    std::cout << std::endl << "Create objects array " << std::endl;

    // ! C-array elements are constructed by default
    SimpleClass arr_c[5];
    // One can use c-tor with parameters in C-arrays
    SimpleClass arr_1[] = { SimpleClass(2), SimpleClass(sc_global) };
    // and even implicit conversion
    SimpleClass arr_2[] = { 1, 2 };

    // 5. Local static object
    std::cout << std::endl << "Create local static objects " << std::endl;
    create_static(0);
    create_static(1);
    create_static(2);

    // Static object is always constructed before the first use
    // Such an approach is called lazy initialization,
    // and utilized in the singleton pattern
    int st = return_static();

    // 6. Global static object
    // Constructors of global objects are called before main() in the order of their declaration
    // The order in different translation units is unspecified

    // 7. Temporary object
    // For the temporary object, the lifetime is the same as the lifetime of the expression
    std::cout << std::endl << "Create temporary object " << std::endl;
    SimpleClass(5);
    // Also, implicit copying happen during passing and returning objects by value,
    // and during throwing and catching exceptions

    // 8. Object created by a user-defined allocator
    {
        // see 01_c_subset/06_memory_management/01_memory_management.cpp
    }

    // 9. Member of union.
    // Union is a special class, which allows to store different types in the same memory
    // It can't have constructors, destructors, and virtual functions
}


int main()
{
    return 0;
}
