#include <utilities/defines.h>
SUPPRESS_PRAGMA_WARNINGS()
#pragma ide diagnostic ignored "UnusedValue"
#pragma ide diagnostic ignored "UnusedLocalVariable"
#pragma ide diagnostic ignored "modernize-use-auto"

#include <iostream>

#include <utilities/bitwise.h>


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
 * Starting from C++11/14, the constexpr keyword can be used to indicate that a variable or function's value
 * can be computed at compile-time.
 * This allows the compiler to optimize code based on the assumption
 * that the value will not change during program execution
 *
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
}

int main(int argc, char* argv[])
{
    runtime_constness();
    compiletime_constness();
    return 0;
}
