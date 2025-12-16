#include "file1.h"
#include "file2.h"

extern int x;
extern int x;

const int y = 2;
using T = int;

// External linkage const via explicit extern (legacy pattern; prefer inline constexpr variable in C++17 headers)
extern const int x1 = 10;

void use_extern()
{
    show_extern();
    x = 10;
}

extern "C" void c_function1() {}
extern "C" void c_function2() {}
extern "C" void c_function3() {}
