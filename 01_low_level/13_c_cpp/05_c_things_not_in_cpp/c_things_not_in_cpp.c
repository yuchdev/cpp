#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Demonstrates **valid C** features that are **ill-formed in standard C++**
// (or far more restricted/different).
//
// Notes:
// - VLAs are C99 and optional in C11; some toolchains disable them by default.
// - Some compilers accept some features as extensions in C++ (do not rely on that).

// 1) Variable Length Arrays (VLA) - C99 (optional in C11), not standard C++.
static void demo_vla(int n)
{
    int vla[n];
    for (int i = 0; i < n; ++i) vla[i] = i;
    printf("vla[%d]=%d\n", n-1, vla[n-1]);
}

// 2) Compound literals (C99) - not standard C++.
struct Point { int x, y; };

static void demo_compound_literal(void)
{
    struct Point p = (struct Point){ .x = 1, .y = 2 };
    printf("compound literal: %d %d\n", p.x, p.y);
}

// 3) Designated initializers (C99) - C++20 supports a subset for aggregates,
// but C supports array designators and is often more permissive.
static void demo_designated_initializers(void)
{
    int a[5] = { [2] = 7, [4] = 9 }; // array designators: NOT C++
    printf("a[2]=%d a[4]=%d\n", a[2], a[4]);

    struct Point p = { .y = 10, .x = 3 }; // order doesn't matter in C
    printf("p=%d,%d\n", p.x, p.y);
}

// 4) restrict keyword (C99) - not standard C++.
static void demo_restrict(int* restrict dst, const int* restrict src, size_t n)
{
    for (size_t i = 0; i < n; ++i) dst[i] = src[i];
}

// 5) Implicit void* -> T* conversion is OK in C, not in C++.
static void demo_void_ptr_conversion(void)
{
    int* p = malloc(sizeof(int)); // C: OK; C++: requires cast
    if (!p) return;
    *p = 42;
    printf("*p=%d\n", *p);
    free(p);
}

// 6) C struct tag rules differ from C++.
// In C, `struct S` and `S` are different unless typedef'd.
struct S { int x; };

static void demo_struct_tag(void)
{
    struct S s = { 1 };
    printf("struct S.x=%d\n", s.x);

    // In C, plain `S` is NOT a type unless you typedef it.
    // Uncommenting the next line is an error in C:
    // S s2 = { 2 };
}

int main(void)
{
    demo_vla(7);
    demo_compound_literal();
    demo_designated_initializers();

    int dst[3] = {0,0,0};
    int src[3] = {1,2,3};
    demo_restrict(dst, src, 3);
    printf("dst[2]=%d\n", dst[2]);

    demo_void_ptr_conversion();
    demo_struct_tag();
    return 0;
}
