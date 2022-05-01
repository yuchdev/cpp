#include <iostream>
#include "construct.h"

construct g_construct;


// 
// Constant pointers and pointers to constant
void show_const_pointers(){

    // Pointer to constant
    const int* pp1 = new const int(3);

    // Update: almost recently uninitialized const free memory, both pointers and arrays
    // Before we could create a useless const pointer to a const int or array of ints,
    // and leave it uninitialized.
    // TODO: try on different standards
#if 0
    const int* pp2 = new const int;

    // You can initialize it with any legal way
    // Even const_cast will not work
    //const_cast<int>(*pp2) = 1;

    // You can create uninitialized const dynamic array
    // You can't even use default initialization, and can't change its values in any way
    const int* pp3 = new const int[100];
#endif

    // Standard permits dynamic array of zero size
    int* pp5 = new int[0];

    // unlike regular array
    //int p6[0];

    // Windows-specific function _msize allows to determine real size of dynamic array
    // ()
#if defined(_WIN32) || defined(_WIN64)
    size_t sz = _msize(pp5);
#endif

    delete pp1;
#if 0
    delete pp2;
    delete[] pp3;
#endif

    delete[] pp5;

    // carefully use typedef with arrays
    typedef construct week_array[7];
    construct* s = new week_array;

    // You should use delete with this type (delete[] is correct), but from the name it's not that obvious
    delete[] s;

    //X* x = new Y[2];
    // delete[] x;
    //It invokes undefined behavior.
    //This is correct.
    //When deleting an array, the dynamic and the static type of the object must be the same,
    //or the behavior is undefined (C++ Standard 5.3.5/3).

    // If the class has only automatic c-tor,
    // this form will call default init...
    construct* pc1 = new construct();
    // ... but this does not
    construct* pc2 = new construct;
}


int main()
{
    show_const_pointers();
    return 0;
}
