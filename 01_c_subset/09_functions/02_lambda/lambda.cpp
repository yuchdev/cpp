#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <functional>

using namespace std;

/*

New features:
* Lambdas (3.4.3,11.4) Note lambda changed in C++14
* Local classes as template arguments (11.4.2, 25.2.1)


Examples:
1. Bitwise operations
2. Initializer list
3. Initializer list with auto
4. Lambda
5. C++14 Generic Lambda and generalized lambda capture
6. emplace {} and copy = {} difference
*/


int show_initializer_list()
{
    // The basic idea of initializer lists as expressions is that if you can initialize a variable x using the notation
    int x { 1 };

    // Qualified by a type, T{...}, meaning 'create an object of type T initialized by T{...}'
    int i1 { 42 };
    int* i2 = new int { 42 };
    delete i2;

    // Unqualified {...}, for which the the type must be determined from the context of use

    // An unqualified list is used where an expected type is unambiguously known.
    // It can be used as an expression only as:
    // * A function argument
    // * A return value
    // * The right-hand operand of an assignment operator (=, but not +=, *=, etc.)
    // * An array subscript

    // When used as the initializer for a named object without the use of a = (as for v above), 
    // an unqualified{}-list performs direct initialization(16.2.6)
    // In all other cases, it performs copy initialization(16.2.6)

    // initializer (direct initialization)
    int v { 7 };

    // initializer (copy initialization)
    int v2 = { 7 };

    // assume m takes value pairs
    std::pair<int, int> m = { 2, 3 };

    // overloaded (!) array subscript
    std::map<int, int> arr;
    v = arr[{0}];

    // right-hand operand of assignment
    v = { 8 };

    // right-hand operand of assignment (?)
    // error C3079: an initializer list cannot be used as the right operand of this assignment operator
    //v += {88};

    // error: not left-hand operand of assignment
    // This is not an insurmountable problem, but it was decided not to extend C++ in that direction
    // {v} = 9;

    // error: not an operand of a non-assignment operator
    // v = 7 + {10};

    // return value
    return{ 11 };
}

/*
Unfortunately, we do not deduce the type of unqualified list for a plain template argument

template<typename T> void f(T);

f({}); // error: type of initializer is unknown
f({ 1 }); // error: an unqualified list does not match 'plain T'
f({1,2}); // error: an unqualified list does not match 'plain T'

this is a language restriction, rather than a fundamental rule
*/

void show_initializer_auto()
{
    // The type of {}-list can be deduced(only) if all elements are of the same type
    // and at least one element present

    //auto x0 = {};

    // initializer_list<int>:
    auto x1 = { 1 };
    auto x2 = { 1,2 };
    auto x3 = { 1,2,3 };

    //auto x4 = { 1,2.0 };
}


void initializer_list_example()
{
    int v = {1,2,3,4,5,6,7,8,9};
    std::set<int> s1(std::begin(v), std::end(v));
    // C++17
    std::set s2{std::begin(v), std::end(v)};
    std::cout << s1.size() << '\n';
    std::cout << s2.size() << '\n';
}

// Pass variadic template params to lambda
namespace cpp4
{

template<typename T, typename... Tail>
void recursive_variadic_call(T s, Tail... v)
{
    std::cout << s << '\n';
    recursive_variadic_call(v...);
}

void recursive_variadic_call() {}

template<typename... Var>
void algo(Var... args)
{
    // first param always int
    auto helper = [&, args...](int i){
        recursive_variadic_call(i, args...);
    };

    std::vector<int> v = { 1,2,3,4,5,6 };
    std::for_each(v.begin(), v.end(), helper);
}

}

void show_lambda()
{
    int val1 = 42;
    int val2 = 666;
    std::vector<int> v = { 1,2,3,4,5,6 };

    // capture lists:

    // no capture
    std::for_each(v.begin(), v.end(), [] (int i) {
        std::cout << i << std::endl;
        });

    // capture all by ref
    std::for_each(v.begin(), v.end(), [&] (int i) {
        std::cout << i + val1 << std::endl;
        });

    // capture all by val
    std::for_each(v.begin(), v.end(), [=] (int i) {
        std::cout << i + val2 << std::endl;
        });

    // capture one by ref
    std::for_each(v.begin(), v.end(), [&val1] (int i) {
        std::cout << i + val1 << std::endl;
        });

    // capture one by val
    std::for_each(v.begin(), v.end(), [val2] (int i) {
        std::cout << i + val2 << std::endl;
        });

    // capture list by ref
    std::for_each(v.begin(), v.end(), [&, val1, val2] (int i) {
        std::cout << i + val1 + val2 << std::endl;
        });

    // capture list by val
    std::for_each(v.begin(), v.end(), [val1, val2] (int i) {
        std::cout << i + val1 + val2 << std::endl;
        });

    // this is captured only by value, members - by ref

    // C++14 has generic lambda (see example below) and support expressions in capture list
    // It may be just value, function or other lambda
    std::unique_ptr<int> ptr(new int(10));
    auto lambda = [value = std::move(ptr)] { return *value; };

    // Naming the lambda is often a good idea
    auto l = [val1, val2] (int i) {
        std::cout << i + val1 + val2 << std::endl;
    };
    std::for_each(v.begin(), v.end(), l);

    // If you need to capture a variadic template (28.6) argument, use ...
    cpp4::algo(1, 2, 3, 4, 5, 6);

    // In the unlikely event that we want to modify the state, we can declare the lambda mutable
    size_t count = v.size();
    std::generate(v.begin(), v.end(), [count] () mutable {return --count; });
    // this is only for 'outer' state modification

    // we can use it to initialize a variable declared auto or std::function<R(AL)>
    // where R is the lambda's return type and AL is its argument list:
    // recursive-lambda string reverse
    std::function<void(char* b, char* e)> rev = [&] (char* b, char* e) {
        if (1 < e - b) { swap(*b, *--e); rev(++b, e); }
    };

    // A lambda might outlive its caller
    // This can happen if we pass a lambda to a different thread 
    // or if the callee stores away the lambda for later use
    // Think of the capture list as the initializer list for the closure object and [=] and [&]

    // A lambda that captures nothing can be assigned to a pointer to function of an appropriate type. For example
    double (*p1)(double) = [] (double a) { return sqrt(a); };
}

namespace cpp4
{

// C++14 has generic lambda
// Container element may be int or casted to int, or anything behaves like int
template <typename T>
size_t count_more_than(const T& container, typename T::value_type val)
{
    size_t count = std::count(v.begin(), v.end(), [val] (auto v) {
        return v > val;
        });
}


class test_emplace_copy
{
public:

    test_emplace_copy(int i) : i_ { i }
    {
        std::cout << "int i" << std::endl;
    }

    test_emplace_copy(const test_emplace_copy& cp) :i_ { cp.i_ }
    {
        std::cout << "const test_emplace_copy& cp" << std::endl;
    }

private:
    int i_;
};

} // namespace cpp4 

void show_emplace_copy()
{
    // When used as the initializer for a named object without the use of a = (as for v above), 
    // an unqualified {}-list performs direct initialization (16.2.6)
    // In all other cases, it performs copy initialization (16.2.6)
    // In particular, the otherwise redundant = in an initializer restricts the set 
    // of initializations that can be performed with a given {}-list
    cpp4::test_emplace_copy a1 { 1 };
    cpp4::test_emplace_copy a2 = { 2 };

    // me: it anyway calls direct initialization
}

int main()
{
    show_bitwise();
    show_initializer_list();
    show_initializer_auto();
    show_lambda();
    show_emplace_copy();
    return 0;
}
