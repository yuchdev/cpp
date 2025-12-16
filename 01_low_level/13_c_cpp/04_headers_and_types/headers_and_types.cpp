#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>

// Demonstrates practical compatibility differences that frequently break mixed code.
//
// 1) Header mapping:
//    C:    <string.h>, <stdlib.h>
//    C++:  <cstring>,  <cstdlib>      (names in namespace std)
//
// 2) `void*` conversion:
//    C allows implicit conversion from void* to typed pointers.
//    C++ requires an explicit cast.
//    Prefer avoiding malloc/free in C++ code (use new/delete or containers).
//
// 3) nullptr vs NULL:
//    C++: nullptr is a real pointer literal (std::nullptr_t).
//    C: no nullptr.
//    NULL is a macro and can cause overload ambiguities in C++.
//
// 4) restrict:
//    C99: `restrict` keyword exists.
//    C++: no standard keyword, but compiler extensions exist (wrapped via macros).

static void show_headers()
{
    const char* s = "abc";
    std::size_t n = std::strlen(s); // from <cstring>
    std::cout << "strlen=" << n << "\n";
}

static void show_void_ptr_and_malloc()
{
    void* raw = std::malloc(sizeof(int));
    if (!raw) {
        std::cout << "malloc failed\n";
        return;
    }

    // C++ requires explicit cast from void*.
    int* ip = static_cast<int*>(raw);
    *ip = 123;
    std::cout << "*ip=" << *ip << "\n";

    std::free(raw);
}

static void show_nullptr_vs_null()
{
    int* p = nullptr;
    (void)p;

    const char* s = nullptr;
    if (!s) {
        std::cout << "nullptr is a real pointer literal\n";
    }
}

static void show_restrict_portably()
{
#if defined(__GNUC__) || defined(__clang__)
#  define CPP_RESTRICT __restrict__
#elif defined(_MSC_VER)
#  define CPP_RESTRICT __restrict
#else
#  define CPP_RESTRICT
#endif

    auto add = [](int* CPP_RESTRICT a, const int* CPP_RESTRICT b, std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            a[i] += b[i];
        }
    };

    int a[3] = {1,2,3};
    int b[3] = {10,20,30};
    add(a, b, 3);

    assert(a[0] == 11);
    std::cout << "restrict-style add ok\n";

#undef CPP_RESTRICT
}

int main()
{
    show_headers();
    show_void_ptr_and_malloc();
    show_nullptr_vs_null();
    show_restrict_portably();
    return 0;
}
