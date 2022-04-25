#define _USE_MATH_DEFINES
#include <iostream>
#include <iomanip>
#include <cmath>

// OsX workaround
#include <cfloat>
#include <cstdint>

#include <utilities/bitwise.h>

// Comparing two floating-point numbers one should remember, that calculation error for floating-point 
// is proportional to its modulus, unlike fixed-point, where error is fixed as well.
//
// Example above shows exact comparison of two Pi approximation does not work (and does not expected to do so).
// Compare to Standard Library constant does not work as well.
// To solve this problem, we should use Epsilon value, also provided by math standard library: DBL_EPSILON
// Its critical to understand that this is not universal error value, 
// it's a minimum value which is change floating-point number bitwise representation when added to 1.0
// In other words, (1 +/- DBL_EPSILON) != 1
// Numbers bigger than 1.0 do not "feel the difference", you can add DBL_EPSILON to 2.0 as many times as you want, 
// it will not have any effect. These number consider DBL_EPSILON as ZERO
// In the meantime, DBL_EPSILON value is not zero, it's 1e-16
// It means, all Planck-range numbers would be also all equal to zero for DBL_EPSILON.
// However, Planck constant as much important as the light speed, 
// representation of such constants is a primary purpose on floating-point numbers.
//
// So, how to use DBL_EPSILON is a right way?
// Imagine you need to compare two floating-point numbers, like approximations of Pi from the given example.
// As we have two numbers, we need to pick up the biggest one, and multiply it to DBL_EPSILON - 
// this way we'll get a value, which guarantees both of compared number being sensitive to its addition
// Wrapping it up, let's write some code
bool close_enough(double a, double b)
{
    if (fabs(a - b) <= DBL_EPSILON * fmax(fabs(a), fabs(b))) {
        return true;
    }
    return false;
}

// However, this algorithm also requires some improvements
// DBL_EPSILON defines difference in one bit of floating-point number bitwise representation.
// On practice however, numbers mostly differ on more than one bit,
// that is why realistic Epsilon approximation could be multiplied to something like 16
bool close_enough2(double a, double b)
{
    if (fabs(a - b) <= 16 * DBL_EPSILON * fmax(fabs(a), fabs(b))) {
        return true;
    }
    return false;
}

void compare_floating_point()
{
    double d1 = 0;
    double d2 = sin(1.0);

    if (d1 == d2) {
        std::cout << "It's not that scary\n";
    }

    // Let's get Pi using 2 different approximations

    // 1.Machin-like formula
    double pi1 = 4 * (4 * atan(0.2) - atan(1. / 239.));
    // 2.60-base approximation
    double pi2 = 3 + 8. / 60. + 29. / pow(60., 2) + 44. / pow(60., 3);

    if (pi1 != pi2) {
        std::cout << "Well, this does not compute...\n";
    }

    if (close_enough(pi1, pi2) || close_enough2(pi1, pi2)) {
        std::cout << "What about now?\n";
    }

}

void show_close_enough()
{

    // nextafter() returns the next representable value after x in the direction of y
    double d = 1.0;
    double e = nextafter(d, 2.0);
    if (close_enough(d, e)) {
        std::cout << d << " == " << e << '\n';
        std::cout << "Bitwise d == " << bitwise(d) << '\n';
        std::cout << "Bitwise e == " << bitwise(e) << '\n';
    }
        
    d = 0.00000000000000000001;
    e = nextafter(d, 1.0);
    if (close_enough(d, e)) {
        std::cout << d << " == " << e << '\n';
        std::cout << "Bitwise d == " << bitwise(d) << '\n';
        std::cout << "Bitwise e == " << bitwise(e) << '\n';
    }
}


int main()
{
    // TODO: insert any function call
    return 0;
}