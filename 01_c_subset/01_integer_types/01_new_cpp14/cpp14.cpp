#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedLocalVariable"
#pragma ide diagnostic ignored "UnusedValue"

#include <iostream>
#include <vector>

/*
Examples:
* Sizes and alignment
* Declaration and decltype, -> suffix (see ch.12)
* Scopes (text only)
* Initialization, auto and {}
* Type aliases (see Ch.05 for examples)
*/


void show_align()
{
    // In addition, on some machine architectures, 
    // the bytes used to hold it must have proper alignment 
    // for the hardware to access it efficiently (or in extreme cases to access it at all)

    // The alignof() operator returns the alignment of its argument expression.For example :

    // TODO: alignof

    // Sometimes, we have to use alignment in a declaration, where an expression, 
    // such as alignof(x + y) is not allowed.
    // Instead, we can use the type specifier alignas: alignas(T)means 'align just like a T'

    //TODO: alignas
}

// C++14 auto return type deduction (impossible in C++11)
template <typename T>
auto add_things(T a, T b)
{
    const auto ret = a + b;
    return ret;
}

void show_init()
{
    // Google's auto guidelines
    // Use type deduction only if it makes the code clearer to readers 
    // who aren't familiar with the project, or if it makes the code safer. 
    // Do not use it merely to avoid the inconvenience of writing an explicit type.
    // https://google.github.io/styleguide/cppguide.html#auto

    // An initializer can use one of four syntactic styles:
    int a1 { 1 };
    int a2 = { 1 };
    int a3 = 1;
    int a4(1);

    // In the cases above, the = is redundant

    // list initialization, does not allow narrowing

    // The trap is that if the initializer is a{}-list, we may not want its type deduced

    // Important note: z1 is an `initializer_list<int>` in C++11 and `int` in C++14 and further
    auto z1 { 99 };

    // z2 is an int
    auto z2 = 99;

    // The classical example is a vector of integers :
    std::vector<int> v1 { 99 }; // v1 is a vector of 1 element with the value 99
    std::vector<int> v2(99); // v2 is a vector of 99 elements each with the default value 0

    // We can decorate a deduced type with specifiers and modifiers, such as const and &
    for (const auto& x : v1) {
        std::cout << x << std::endl;
    }
}

/*
template<class T, class U>
auto operator+(const Matrix<T>& a, const Matrix<U>& b) -> Matrix<decltype(T{}+U{})>;

I use the suffix return type syntax to be able to express the return type in terms of the arguments

Important note: decltype has new behavior in C++14
*/

int main()
{
    show_align();
    show_init();
    return 0;
}

#pragma clang diagnostic pop

