#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <cstdint>


/*

C+14 New features:
* Generalized and guaranteed constant expressions: constexpr (2.2.3, 10.4, 12.1.6)
* Suffix return type syntax (12.1.4)
* A syntax for attributes and two standard attributes
* C99 features: extended integral types (i.e., rules for optional longer integer types; 6.2.4);
concatenation of narrow/wide strings; __func__ and __STDC_HOSTED__ (12.6.2);
_Pragma(X) (12.6.3); vararg macros and empty macro arguments (12.6)

Examples:
1. Suffix return
2. Local static is undefined
3. List arguments
4. Overload resolution
5. Pointers to function - new stuff
6. Useful macros
*/


// 1. Suffix return

namespace cpp4
{

// The essential use for a suffix return type comes in function template declarations
// in which the return type depends on the arguments
// Important note: decltype has new behavior in C++14
template<typename X, typename Y>
auto product(X x, Y y) -> decltype(x* y)
{
    return x * y;
}

} // namespace cpp4

void show_suffix()
{
    auto prod = cpp4::product(1, 1.0);
    std::cout << prod << std::endl;
}

// 2. noreturn
namespace cpp4
{

// noreturn function does not have normal return (throw, exit)
[[noreturn]]
void exit(int)
{
    std::exit(0);
}

} // namespace cpp4


// 3. Local static is undefined

namespace cpp4
{

//  The effect of initializing a local static recursively is undefined
int recursive_static(int i)
{
    static int n1 = i;

    // Do not run! result is undefined
    static int n2 = recursive_static(i - 1) + 1;
    return n1 + n2;
}

} // namespace cpp4

// 3.1 Array

// If an array is used as a function argument, a pointer to its initial element is passed
// These three declarations are equivalent and declare the same function
/*
void odd(int* p)
void odd(int a[])
void odd(int buf[1020])
*/
// The size of an array is not available to the called function


// 4. List arguments
// A {}-delimited list can be used as an argument to a parameter of:
// [1] Type std::initializer_list<T>, where the values of the list can be implicitly converted to T
// [2] A type that can be initialized with the values provided in the list
// [3] A reference to an array of T, where the values of the list can be implicitly converted to T

namespace cpp4
{

template<class T>
void f1(std::initializer_list<T>) {}

struct S
{
    int a;
    std::string s;
};

void f2(S) {}

template<class T, int N>
void f3(T(&r)[N]) {}

void f4(int) {}

} // namespace cpp4

void show_list_args()
{
    // T is int and the initializer_list has size() 4 
    cpp4::f1({ 1,2,3,4 });

    // f2(S{1,"MKS"})
    cpp4::f2({ 1,"MKS" });

    // T is int and N is 4
    //cpp4::f3({ 1,2,3,4 }); 
    // cannot convert argument 1 from 'initializer list' to 'int (&)[4]'

    //f4(int{1});
    cpp4::f4({ 1 });

    // If there is a possible ambiguity, an initializer_list parameter takes priority
    // It can affect overloads
}

// 5. Overload resolution
/*
To approximate our notions of what is reasonable, a series of criteria are tried in order:
1. Exact match; that is, match using no or only trivial conversions (for example, array name
to pointer, function name to pointer to function, and T to const T)
2. Match using promotions; that is, integral promotions (bool to int, char to int, short to int,
and their unsigned counterparts; 10.5.1) and float to double
3. Match using standard conversions and narrowing (e.g., int to double, double to int, double to long double,
Derived* to Base* (20.2), T* to void* (7.2.1), int to unsigned int (10.5))
4. Match using user-defined conversions (e.g., double to complex<double>; 18.4)
5. Match using the ellipsis ... in a function declaration (12.2.4)

*/
namespace cpp4
{

void print(int)
{
    std::cout << "void print(int)" << std::endl;
}

void print(const char*)
{
    std::cout << "void print(const char*)" << std::endl;
}

void print(double)
{
    std::cout << "void print(double)" << std::endl;
}

void print(long)
{
    std::cout << "void print(long)" << std::endl;
}

void print(char)
{
    std::cout << "void print(char)" << std::endl;
}

} // namespace cpp4

void show_overload_resolution()
{
    char c {};
    int i {};
    short s {};
    float f {};

    // exact match: invoke print(char)
    cpp4::print(c);

    // exact match: invoke print(int)
    cpp4::print(i);

    // integral promotion: invoke print(int)
    cpp4::print(s);

    // float to double promotion: print(double)
    cpp4::print(f);

    // exact match: invoke print(char)
    cpp4::print('a');

    // exact match: invoke print(int)
    cpp4::print(49);

    // exact match: invoke print(int)
    cpp4::print(0);

    // exact match: invoke print(const char*)
    cpp4::print("a");

    // nullptr_t to const char* promotion: invoke print(cost char*)
    cpp4::print(nullptr);
}

// 6. Pointers to function - new stuff

// A pointer to a noexcept function can be declared noexcept
// A pointer to function must reflect the linkage of a function(15.2.6)
// Neither linkage specification nor noexcept may appear in type aliases

namespace cpp4
{

// exception-noexcept
void noexcept_f(int) noexcept {}
void plain_f(int) {}

// calling convention specifiers
void __stdcall std_call_func(int) {}
void __fastcall fast_call_func(int) {}
void __cdecl cdecl_call_func(int) {}

// Linking convention - the same
}

void show_function_ptrs_specifiers()
{
    // OK: but we throw away useful information 
    void(*p1)(int) = cpp4::noexcept_f;

    // OK: we preserve the noexcept information 
    void(*p2)(int) noexcept = cpp4::noexcept_f;

    // error: we don't know that g doesn't throw
    //  error C2440: 'initializing': cannot convert from 'void (__cdecl *)(int)' to 'void (__cdecl *)(int) noexcept'
    //void(*p3)(int) noexcept = cpp4::plain_f;

    // A pointer to function must reflect the linkage of a function(15.2.6)
    void(__cdecl * p3)(int) = &cpp4::cdecl_call_func;
    void(__stdcall * p4)(int) = &cpp4::std_call_func;
    void(__fastcall * p5)(int) = &cpp4::fast_call_func;

    // aliases
    //  error C2143: syntax error
    //using ptr_c = extern "C" void(int);

    // error C2279: exception specification cannot appear in a typedef declaration
    //using ptr_noexcept = void(int) noexcept;

    // OK
    using ptr_stdcall = void __stdcall (int);
}

// 7. Useful macros

// A string can be created by concatenating two strings using the ## macro operator
#define NAME2(a,b) a##b

// A single # before a parameter name in a replacement string means a string containing the argument
#define printx(x) std::cout << #x " = " << x << '\n';

// Macros can even be variadic.For example:
#define err_print(...) fprintf(stderr,"error: %s %d\n", __VA_ARGS__) 


void show_macro()
{
    printx(42);
    err_print("The answer", 54);

    // Useful predefined macros
    std::cout << "__cplusplus = " << __cplusplus << '\n';
    std::cout << "__DATE__ = " << __DATE__ << '\n';
    std::cout << "__TIME__ = " << __TIME__ << '\n';
    std::cout << "__FILE__ = " << __FILE__ << '\n';
    std::cout << "__LINE__ = " << __LINE__ << '\n';

#ifdef __FUNC__
    std::cout << "__FUNC__ = " << __FUNC__ << '\n';
#endif

#ifdef __STDC_HOSTED__
    std::cout << "__STDC_HOSTED__ = " << __STDC_HOSTED__ << '\n';
#endif

#ifdef __STDC_MB_MIGHT_NEQ_WC__
    std::cout << "__STDC_MB_MIGHT_NEQ_WC__ = " << __STDC_MB_MIGHT_NEQ_WC__ << '\n';
#endif

#ifdef __STDCPP_STRICT_POINTER_SAFETY__
    std::cout << "__STDCPP_STRICT_POINTER_SAFETY__ = " << __STDCPP_STRICT_POINTER_SAFETY__ << '\n';
#endif

#ifdef __STDCPP_THREADS__
    std::cout << "__STDCPP_THREADS__ = " << __STDCPP_THREADS__ << '\n';
#endif    
}

int main()
{
    show_suffix();
    show_list_args();
    show_overload_resolution();
    show_function_ptrs_specifiers();
    show_macro();
    return 0;
}
