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
 * In C++, the constexpr keyword can be used to indicate that a variable or function's value
 * can be computed at compile-time.
 * This allows the compiler to optimize code based on the assumption
 * that the value will not change during program execution
 */
 constexpr int square(int x) {
    return x * x;
}

void compiletime_constness()
{
    constexpr int side = 5;
    constexpr int area = square(side);
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
    Point p = poly.get_point(0);
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
