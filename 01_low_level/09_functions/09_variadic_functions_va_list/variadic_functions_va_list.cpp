/*
================================================================================
09_variadic_functions_va_list.cpp

Advanced topic: variadic functions (...) are ABI-sensitive and unsafe.

Key facts:
  - default argument promotions apply (float->double, etc.)
  - on some ABIs, varargs changes register passing rules
  - you cannot forward "..." directly; you must use va_list
  - va_list is single-use unless you va_copy it
  - format-string/type mismatches are classic Undefined Behavior

This file demonstrates:
  1) logger using va_list
  2) correct forwarding
  3) va_copy for multiple traversals
================================================================================
*/

#include <cstdarg>
#include <cstdio>
#include <iostream>

static void vlogf(const char* fmt, va_list ap) {
    std::vfprintf(stderr, fmt, ap);
    std::fputc('\n', stderr);
}

static void logf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vlogf(fmt, ap);
    va_end(ap);
}

static void logf_twice(const char* fmt, ...) {
    va_list ap1;
    va_start(ap1, fmt);

    va_list ap2;
    va_copy(ap2, ap1);

    vlogf(fmt, ap1);
    vlogf(fmt, ap2);

    va_end(ap2);
    va_end(ap1);
}

int main() {
    std::cout << "== Variadic demo ==\n";
    logf("x=%d s=%s", 7, "hello");
    logf_twice("twice: pi~%.2f, n=%d", 3.14159, 42);

    // UB examples (do not do):
    // logf("x=%d", 3.14);
    // logf("%s", 123);

    std::cout << "\nTakeaway: avoid variadics unless required for C interop.\n";
    return 0;
}
