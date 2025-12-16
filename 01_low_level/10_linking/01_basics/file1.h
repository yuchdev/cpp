#pragma once
#include <cstddef>

void use_extern();

extern "C" void c_function1();

extern "C" {
    void c_function2();
    void c_function3();

    using c_api_func_t = void(*)(void*, std::size_t, std::size_t);
}

extern "C" {
#include <string.h>
}

#ifdef __cplusplus
#  define EXTERNC extern "C"
#else
#  define EXTERNC
#endif
