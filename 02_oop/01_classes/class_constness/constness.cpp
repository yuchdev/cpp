#include <iostream>
#include <utilities/bitwise.h>
#include <utilities/defines.h>
SUPPRESS_PRAGMA_WARNINGS()
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#include <type_traits>
#include <cassert>


// Arguments for using data accessors and mutators
// * distinctive read-write access
// * support of functional abstractness - class member could be replaced with calculation, cached, or even moved outside the class
// * value evaluation, checking the consistency
// * logging
// * if we see open class member in method, we expect it to be this class member


// It is allowed to declare structure and non-structure with the same name
// (same for class, enum, union)
void struct_one_name()
{
    struct struct_one_name
    {
        int a;
    };

    struct_one_name s;
    s.a = 2;
    int struct_one_name = 1;
    static_assert(std::is_same<decltype(s.a), decltype(struct_one_name)>::value, "struct_one_name");
}

// Methods overload
// https://en.cppreference.com/w/cpp/language/overload_resolution

class SimpleClass
{
public:

    SimpleClass(char* const cp1) : cp1(cp1) {}

    // Static members COULD be changed in const members, as it does not change class state
    void set_static(int i) const
    {
        a_stat = i;
    }

    // const and non-const methods with same name
    // When both versions are available, the logic is pretty straightforward: 
    // const version is called for const objects
    // non-const version is called for non-const objects

    // TODO: overload rules
    // https://www.learncpp.com/cpp-tutorial/introduction-to-function-overloading/
    // https://en.cppreference.com/w/cpp/language/overload_resolution
    // https://accu.org/journals/overload/13/66/kilpelainen_268/
    // 
    // TODO: function signature

    // Static method coule be called with the class instance and with the class name
    // construct::out_static_array() and c.construct::out_static_array();

    // You can't set array size with the other class member, even with const static
    // int arr1[sz];

private:
    // friend declarations of non-existent classes are ignored
    friend class do_not_exists;

    static int a6;

    // unnamed enum could be considered const class member
    // could be seen in pre-C++11 code, before in-place initialization was introduced
    enum { c1 = 5, c2 = 7, c3 = 12 };

    // mutable class member could always be changed 
    // even being part of const or static object 
    // Design may require this, if we deal with so-called "conceptual constness" 
    // For example, cache in constant object could be mutable

    mutable int a8;
    // Opposite situation is also possible
    // when class contains pointer, which is being changed inside the const method
    // such method is const formally, but not conceptually

    // constant pointer or pointer to constant?
    // to determine it quickly, you can use "read backward" trick
    char const* cp2; // pointer (*) to 'const' 'char'
    const char* cp3; // pointer (*) to 'char' 'const'
    char* const cp1; // 'const' pointer (*) to 'char'
    static int a_stat;
};



/**
 * In C++, the const keyword is used to indicate that a variable's value is read-only
 * and cannot be modified during program execution.
 * It can be applied to variables on the stack, function parameters, and class members.
 * Runtime constness refers to variables whose values are determined at runtime
 * but cannot be modified after initialization.
 * These variables are still read-only, but their values are not known until the program is executed
 */
void runtime_constness()
{
    const int max_size = 100;
    enum { num_students = 30 };
    const int sum = 5 + 7 * 9;
}

/**
 * In C++, the constexpr keyword can be used to indicate that a variable or function's value
 * can be computed at compile-time.
 * This allows the compiler to optimize code based on the assumption
 * that the value will not change during program execution
 */
// For C++11/14
constexpr int square(int x) {
    return x * x;
}

constexpr int factorial(int n) {
    int result = 1;
    for (int i = 1; i <= n; ++i) {
        result *= i;
    }
    return result;
}

// For C++17
constexpr int fib(int n) {
    if (n == 0) {
        return 0;
    }
    else if (n == 1) {
        return 1;
    }
    else {
        int a = 0;
        int b = 1;
        for (int i = 2; i <= n; ++i) {
            int c = a + b;
            a = b;
            b = c;
        }
        return b;
    }
}

constexpr int sum_fibonacci(int n) {
    int sum = 0;
    for (int i = 0; i <= n; ++i) {
        if (fib(i) % 2 == 0) {
            sum += fib(i);
        }
    }
    return sum;
}


// For C++20
class Circle {
public:
    constexpr Circle(double r) : radius_(r) {}
    constexpr double area() const {
        return pi * radius_ * radius_;
    }
    constexpr void set_radius(double r) {
        radius_ = r;  // allowed in C++20, as long as it doesn't affect the final value
    }
private:
    double radius_;
    static constexpr double pi = 3.14159265358979323846;
};

void compiletime_constness()
{
    // In C++11, constexpr could only be used to specify that a function or variable's
    // value or return value could be computed at compile-time
    constexpr int side = 5;
    constexpr int area = square(side);

    // In C++14, the constexpr specifier was extended to allow
    // more general-purpose programming constructs to be used in constant expressions.
    // For example, a function could contain a constexpr variable
    constexpr int square_factorial = factorial(2) * factorial(2);  // computed at compile-time

    // In C++17, constexpr was further extended to allow if-statements and loops
    // to be used in constant expressions
    constexpr int sum_fib = sum_fibonacci(10);  // computed at compile-time

    // In C++20, constexpr was extended further to allow constexpr functions
    // to be used in more contexts, such as template arguments, virtual functions,
    // and non-static member functions
    constexpr Circle c1(1.0);
    constexpr Circle c2(2.0);
    constexpr double total_area = c1.area() + c2.area();
}

/**
 * Bitwise constness refers to the fact that a const object cannot be modified
 * through its non-const pointer or reference. In other words, the bits of the object are read-only,
 * but the object itself may not be conceptually read-only. Here's an example of bitwise constness
 */
void print(const int* ptr)
{
    // error: cannot modify const object
    //*ptr = 10;
}

int bitwise_constness()
{
    const int x = 5;
    const int* ptr = &x;
    print(ptr);
    return 0;
}

/**
 * Conceptual constness, on the other hand, refers to the fact that an object is conceptually read-only,
 * regardless of whether its bits can be modified or not.
 * In other words, the constness is a property of the object's logical state,
 * rather than its physical state. Here's an example of conceptual constness
 */
struct Point
{
    int x{};
    int y{};
};

class Polygon {

public:
    Polygon(int num_points) : points_{new Point[num_points]} {}
    void set_point(int index, int x, int y)
    {
        points_[index].x = x;
        points_[index].y = y;
    }

    Point get_point(int index) const
    {
        // violate conceptual constness on purpose
        points_[index].x = 0;
        return points_[index];
    }

private:
    Point* const points_;

};


void conceptual_constness()
{
    Polygon poly1(5);
    poly1.set_point(0, 10, 20);

    const Polygon poly2(5);
    // violate conceptual constness on purpose
    Point p = poly2.get_point(0);
}


int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <function>\n";
        return 1;
    }
    std::string func = argv[1];

    if (func == "runtime_constness") {
        runtime_constness();
    }
    else if (func == "compiletime_constness") {
        compiletime_constness();
    }
    else if (func == "bitwise_constness") {
        bitwise_constness();
    }
    else if (func == "conceptual_constness") {
        conceptual_constness();
    }

    else {
        std::cerr << "Unknown function: " << func << '\n';
        return 1;
    }

    return 0;
}
