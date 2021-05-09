#pragma once

//make OsX happy
#include <cstddef>

void use_extern();

// extern "C" specifies the linkage conventions not only for plain C,
// but cross-language communication for Fortran, Assembler, etc.
extern "C" void c_function1();

// You can declare group of function for convenience
extern "C"{
    void c_function2();
    void c_function3();

    // Passing the function pointer to a C application
    // one also should specify the linkage convention
    typedef void(qsort_t)(void*, size_t, size_t);
    // points to a function which will be linked as C function
};

// Wrap the entire file as extern "C"
extern "C"{
#include "string.h"
};

// Declare extern "C" only for C++ code (__cplusplus macro defined)
#ifdef __cplusplus
#define EXTERNC  extern "C"
#else
#define EXTERNC
#endif

