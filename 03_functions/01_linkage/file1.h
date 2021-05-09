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
};

// Passing the function pointer to a C application
// one also should specify the linkage convention
// points to a function which will be linked as C function
extern "C" int fc(int) { return 1; }      // fc has C linkage
int fcpp(int) { return 1; }               // fcpp has C++ linkage

// fp1 and fp2 have C++ linkage
int (*fp1)(int) = fc;                     // Mismatch
int (*fp2)(int) = fcpp;                   // OK 

// fp3 and fp4 have C linkage 
extern "C" int (*fp3)(int) = fc;          // OK 
extern "C" int (*fp4)(int) = fcpp;        // Mismatch


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

