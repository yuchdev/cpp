#pragma once

//make OsX happy
#include <cstddef>

void use_extern();

// Соглашение по вызову extern "C" применяется для межязыкового взаимодействия
// между C++ и С/FORTRAN/Assembler etc.
extern "C" void c_function1();

// Можно оъединять объявления так
extern "C"{
    void c_function2();
    void c_function3();

    // при передаче указателя на функцию программе на C
    // также следует указывать соглашение о компоновке
    typedef void(qsort_t)(void*, size_t, size_t);
    // Тогда при передаче этого указателя сооствтствующая ф-ция
    // будет скомпонована так же
    // void i_sort(qsort_t sort_function); - будет скомпонована в "C"
};

// Можно обернуть целый файл
extern "C"{
#include "string.h"
};

// Но лучше так
#ifdef __cplusplus
#define EXTERNC  extern "C"
#else
#define EXTERNC
#endif

