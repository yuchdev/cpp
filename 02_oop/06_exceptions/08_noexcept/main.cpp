#include <iostream>
#include <vector>
#include <stdexcept>
#include <type_traits>

using namespace std;

/*

C++14 features
* Preventing exception propagation: the noexcept specifier (13.5.1.1) Section 44.2.1 Language Features 1269
* Testing for the possibility of a throw in an expression: the noexcept operator (13.5.1.1)

Examples:
1. Finally
2. assert/static_assert
3. Conditional noexcept, noexcept operator
4. noexcept check

5. set_terminate
6. vector implementation
*/

namespace cpp
{

// 1. `finally` implementation
// The 'finally action' is provided as an argument to the constructor
template<typename F>
struct final_action
{
    final_action(F f) : clean { f }
    {
    }

    ~final_action()
    {
        clean();
    }

    F clean;
};

// we define a function that conveniently deduces the type of action
template<class F>
final_action<F> finally(F f)
{
    return final_action<F>(f);
}

} // namespace cpp

void show_finally()
{
    // allocate some raw data
    int* i = new int[100];

    auto finally1 = cpp::finally([&] () {
        delete[] i;
        std::cout << "Finally cation" << std::endl;
        });
}


// 2. assert/static_assert
// obvious

// 3. Conditional noexcept, noexcept operator
// 4. noexcept check
namespace cpp
{

// However, noexcept is not completely checked by the compiler and linker
void check_noexcept1() noexcept
{
    // use an operation that potentially throws
    int* i = new int[1000];
    std::memset(i, 0, sizeof(int) * 1000);
    delete[] i;
}

void check_noexcept2() noexcept
{
    // explicitly throw an exception
    // warning C4297: 'cpp::check_noexcept2': function assumed not to throw an exception but does
    throw std::runtime_error("boom");
}

// conditional noexcept (predicate must be constexpr)
template<typename T>
void copy_function(T& x) noexcept(std::is_pod<T>::value)
{
    T tmp(x); // POD does not throw
}

// zero memory is noexcept
template <typename T>
void func(T t) noexcept
{
    std::memset(&t, 0, sizeof(T));
}

// noexcept operator - returns true if operand is nothrow
// Here we check whether called func is noexcept, and make call_func()
// noexcept too
template <typename T>
void call_func(std::vector<T>& v) noexcept(noexcept(func(v[0])))
{
    for (auto x : v)
        func(x);
}

/*
Conditional noexcept specifications and the noexcept() operator
are common and important in standard-library operations that apply to containers

template<class T, size_t N>
void swap(T (&a)[N], T (&b)[N]) noexcept(noexcept(swap(*a, *b)));
*/

} // namespace cpp

void show_noexcept()
{
    try {
        cpp::check_noexcept1();

        // terminate() is called if noexcept throws
        // see our own terminate handler below
        cpp::check_noexcept2();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    struct S { int i {}; };
    S s;
    cpp::copy_function(s); // s is POD, should be noexcept

    std::vector<int> v { 1,2,3 };
    cpp::call_func(v); // POD = zero all values
}


// 5. set_terminate

using terminate_handler = void(*)();

[[noreturn]]
void my_handler()
{
    // handle termination my way
    // exit calls destructors
    exit(1);
}

// 6. vector implementation
// TODO

int main()
{
    // set our own terminate
    terminate_handler old_handler = ::set_terminate(&my_handler);

    show_finally();
    show_noexcept();
    return 0;
}

/*
 * Abandoning a process: quick_exit(15.4.3)
 */
void my_cleanup()
{
    std::cout << "Custom at_quick_exit() cleanup\n";
}

// The quick_exit() function is like exit() except that it does not invoke any destructors
// You register functions to be invoked by quick_exit() using at_quick_exit()
[[noreturn]]
void my_return()
{
    quick_exit(0);
}

int main()
{
    if (0 == std::at_quick_exit(&my_cleanup)) {
        // ok
    }
    else {
        std::cout << "Too many at_quick_exit() functions\n";
    }

    my_return();

    return 0;
}
