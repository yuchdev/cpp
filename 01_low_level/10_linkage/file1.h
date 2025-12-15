#pragma once

//make OsX happy
#include <cstddef>

void use_extern();

// Call agreement extern "C" is being used for cross-language communication
// between C++ and C/FORTRAN/Assembler etc.
extern "C" void c_function1();

// It's okay to merge extern "C" declarations like that
extern "C" {
    void c_function2();
    void c_function3();

    // passing the pointer between languages, pass explicitly the linking convention
    typedef void(qsort_t)(void*, size_t, size_t);
    // then after passing such pointer, caller function receive the same linking convention
    // assume we have caller function
    // void i_sort(qsort_t sort_function); // it will have extern "C" linking convention
};

// wrap the whole header
extern "C" {
#include "string.h"
};

// Use __cplusplus macro for declaring extern "C"
#ifdef __cplusplus
#define EXTERNC  extern "C"
#else
#define EXTERNC
#endif

