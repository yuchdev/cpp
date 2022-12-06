#include "simple_array.h"
#include "efficient_array.h"

// Here we demonstrate the most suboptimal way of vector arithmetic
// All objects are directly added and multiplied, creating all
// temporary objects
void show_unefficient()
{

    SArray<double> x(10);
    SArray<double> y(10);
    x[0] = 1.0; x[3] = 2.0; x[6] = 3.0;
    y[0] = 2.0; y[3] = 4.0; y[6] = 2.0;

    print_debug("start expression");
    x = 1.2 * x + x * y;
    print_debug("end expression");

    // 3 temporary objects and 4 copy operations
    // it would be too much of a burden on vectors with a dimension of 1000!
    // we need some kind of universal tool, 
    // that will perform all the operations within 
    // only one pass of the cycle
}

// let's try to move some of the calculations to the compile stage
void show_efficient()
{

    Array<double> x(10);
    Array<double> y(10);
    x[0] = 1.0; x[3] = 2.0; x[6] = 3.0;
    y[0] = 2.0; y[3] = 4.0; y[6] = 2.0;

    print_debug("start expression");
    x = 1.2 * x + x * y;
    print_debug("end expression");

    // All operations are collapsed into a single assignment operator
    // The rest is done "in place" due to the deep instantiation
    std::cout << x[0] << std::endl;
    std::cout << x[3] << std::endl;
}

int main()
{
    show_unefficient();
    show_efficient();

    return 0;
}
