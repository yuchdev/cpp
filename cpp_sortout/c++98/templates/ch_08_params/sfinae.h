#pragma once

// SFINAE principle (Substitution Failure is not an error)

// Let's declare two types
// sizeof first == 1, second == 2
typedef char RT1;
typedef struct { char a[2]; }  RT2;

struct X
{
    char a[1];
};

class test_X
{
public:
    X x;
};

// Now overload 2 templaytes, returning different types depending on param
// If the type has member X, it returns first, otherwise returns second
template <typename T> RT1 test_sfinae_x(typename T::X const&)
{
    return 0;
}

template <typename T> RT2 test_sfinae_x(T)
{
    RT2 rt = {};
    rt.a[0] = 'a';
    rt.a[1] = 0;
    return rt;
}

// Declare the macros checking the size of a class (since C++11 could be done in compile-time)
#define type_has_member_type_X(T) ( ( sizeof(test_sfinae_x<T>() ) ) == 1 )


// Note: SFINAE protects against wrong types, but not against wrong calculations

// One more SFINAE exmaple
template <int N> int get_int() { return N; }
template <int* P> int get_int() { return *P; }

struct Test
{
    typedef int foo;
};

template <typename T>
void f(typename T::foo)
{
    std::cout << "Definition #1" << std::endl;
} // Definition #1

template <typename T>
void f(T)
{
    std::cout << "Definition #2" << std::endl;
}                // Definition #2

void test_sfinae()
{
    f<Test>(10); // Call #1.
    f<int>(10);  // Call #2. Without error (even though there is no int::foo) thanks to SFINAE.
}

void test_sfinae_int()
{
    // Uncomment for error
    //return get_int<1>();
}
