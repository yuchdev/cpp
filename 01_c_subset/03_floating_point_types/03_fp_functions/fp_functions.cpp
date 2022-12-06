#define _USE_MATH_DEFINES
#include <iostream>
#include <iomanip>
#include <cmath>

// OsX workaround
#include <cfloat>
#include <cstdint>

#include <utilities/bitwise.h>


// 4.Floating point functions and applications

void print_roundings_header()
{
    std::cout
        << "val" << '\t'
        << "ceil" << '\t'
        << "floor" << '\t'
        << "trunc" << '\t'
        << "round" << '\t'
        << "lround" << '\t'
        << "rint" << '\t'
        << "lrint" << '\t'
        << "nearbyint" << '\n';
}


void print_roundings(double val)
{
    std::cout << std::fixed;
    std::cout << std::setprecision(4);
    std::cout
        << val << '\t'
        << ceil(val) << '\t'
        << floor(val) << '\t'
        << trunc(val) << '\t'
        << round(val) << '\t'
        << lround(val) << '\t'
        << rint(val) << '\t'
        << lrint(val) << '\t'
        << nearbyint(val) << '\n';
}

void print_fpclassify(double val)
{
    int val_type = fpclassify(val);
    std::cout << val << " is ";
    switch (val_type) {
    case FP_INFINITE:
        std::cout << "infinite" << '\n';
        break;
    case FP_NAN:
        std::cout << "NaN" << '\n';
        break;
    case FP_ZERO:
        std::cout << "zero" << '\n';
        break;
    case FP_SUBNORMAL:
        std::cout << "subnormal" << '\n';
        break;
    case FP_NORMAL:
        std::cout << "normal";
        if (signbit(val)) {
            std::cout << " negative" << '\n';
        }
        else {
            std::cout << " positive or unsigned" << '\n';
        }
        break;
    default:
        std::cout << "Error! Should not be here!" << '\n';
    }
}

void show_cmath_fpoint_operations()
{
    std::cout << "Rounding functions" << '\n';
    print_roundings_header();
    print_roundings(0.0);
    print_roundings(0.3);
    print_roundings(0.5);
    print_roundings(0.8);
    print_roundings(1.3);
    print_roundings(2.3);
    print_roundings(2.5);
    print_roundings(2.8);

    //
    std::cout << "Trigonometric functions" << '\n';
    double pi = M_PI;

    double c1 = cos(pi);
    std::cout << "cos PI = " << c1 << '\n';

    double s1 = sin(pi); // close to 0, but not a 0
    std::cout << "sin PI = " << s1 << '\n';

    double t1 = tan(pi);
    std::cout << "tan PI = " << t1 << '\n';

    double ac1 = acos(c1);
    std::cout << "acos c1 = " << ac1 << '\n';

    double as1 = asin(s1);
    std::cout << "asin s1 = " << as1 << '\n';

    double at1 = atan(t1);
    std::cout << "atan t1 = " << at1 << '\n';

    double at2 = atan2(1.0, 2.0);
    std::cout << "atan2 1/2 = " << at2 << '\n';

    // Hyperbolic supported as well

    std::cout << "Exponential functions" << '\n';
    double param = 5.0;

    double result = exp(param);
    std::cout << "exp (" << param << ") = " << result << "\n";

    // Breaks the floating point number x into its binary significand
    // (a floating point value between 0.5(included)and 1.0(excluded))
    // and an integral exponent for 2, such that :
    // x = significand * 2^exponent
    int n = 0;
    param = 8.0;
    result = frexp(param, &n);
    std::cout << param << " = " << result << "*2^" << n << '\n';

    // Compose value back
    double back = ldexp(result, n);
    std::cout << back << " = " << result << "*2^" << n << '\n';

    // modf() splits value
    double fractpart = 0.0;
    double intpart = 0.0;
    fractpart = modf(pi, &intpart);
    std::cout << pi << " = " << intpart << " + " << fractpart << '\n';

    // exp2() returns the base-2 exponential function of x
    // which is 2 raised to the power x
    param = 8.0;
    double e2 = exp2(param);
    std::cout << "2 ^ " << param << " = " << e2 << '\n';

    // expm1() returns e raised to the power x minus one : e^x - 1
    std::cout << "expm1(1.0) = e^x - 1 = " << expm1(1.0) << '\n';

    // 
    std::cout << "Logarithmic functions" << '\n';
    // logb() returns the integral part of the logarithm of |x|, using FLT_RADIX as base
    // log1p() returns the natural logarithm of one plus x (log(1+x))
    // log2() returns the binary(base - 2) logarithm of x
    std::cout << "log(10.0) = " << log(10.0) << '\n';
    std::cout << "logb(10.0) = " << ilogb(10.0) << '\n';
    std::cout << "log1p(10.0) = log(1+x) = " << log1p(10.0) << '\n';
    std::cout << "log2(1024) = " << log2(1024) << '\n';

    // scalbn(x, n) = x * FLT_RADIX^n
    double x = 2.0;
    int n1 = 4;
    std::cout << "FLT_RADIX = " << FLT_RADIX << '\n';
    std::cout << "x = " << x << '\n';
    std::cout << "n1 = " << n1 << '\n';
    std::cout << "scalbn(x, n) = x * FLT_RADIX^n = " << scalbn(x, n1) << '\n';

    // 
    std::cout << "Power functions" << '\n';
    std::cout << "sqrt(2.0) = " << sqrt(2.0) << '\n';
    std::cout << "cbrt(2.0) = " << cbrt(2.0) << '\n';

    // hypot() returns the hypotenuse of a right - angled triangle whose legs are x and y
    std::cout << "Hypotenuse of (3, 4) =" << hypot(3, 4) << '\n';

    //
    std::cout << "Error function/Gamma function (used in probability, statistics, PDE)" << '\n';
    // http://en.wikipedia.org/wiki/Error_function
    // http://en.wikipedia.org/wiki/Gamma_function
    std::cout << "Error function of 1 =" << erf(1.0) << '\n';
    std::cout << "Complimentary error function of 1 =" << erfc(1.0) << '\n';
    std::cout << "Gamma function of 1 =" << tgamma(1.0) << '\n';
    std::cout << "Log of Gamma function of 1 =" << tgammal(1.0) << '\n';

    std::cout << "Miscellaneous functions" << '\n';

    // copysign copies sign of second value to the first
    std::cout << "copysign(-10.0, 1.0)" << copysign(-10.0, 1.0) << '\n';
    std::cout << "copysign(-10.0, -1.0)" << copysign(-10.0, -1.0) << '\n';
    std::cout << "copysign(10.0, -1.0)" << copysign(10.0, -1.0) << '\n';

    // nextafter() returns the next representable value after x in the direction of y
    std::cout << "Next representable value after 0 = " << nextafter(0.0, 1.0) << '\n';
    std::cout << "Next representable value before 0 = " << nextafter(0.0, -1.0) << '\n';
    // nexttoward() the same with long y

    // NAN (not a number)
    // he NaN values are used to identify undefined or non-representable values
    // for floating-point elements, such as the square root of negative numbers or the result of 0/0
    // Generate quiet NAN (const char* param is implementation-specific)
    double nn = nan("");
    float nf = nanf("");
    std::cout << "For " << nn << " isnan(nn) = " << std::isnan(nn) << '\n';
    std::cout << "For " << nf << " isnan(nf) = " << std::isnan(nf) << '\n';
    std::cout << "For " << "isnan sqrt(-1.0) = " << std::isnan(sqrt(-1.0)) << '\n';
    // isnormal is opposite to isnan, but also checks for INF and 0
    std::cout << "For " << "isnormal sqrt(-1.0) = " << isnormal(sqrt(-1.0)) << '\n';

    // isunordered() check at least one of values is NAN
    if (isunordered(sqrt(-1.0), 0.0))
        std::cout << "sqrt(-1.0) and 0.0 cannot be ordered\n";

    double myinf = INFINITY;
    if ((1 / sin(0.0) == myinf) && (!isnormal(myinf))) {
        std::cout << "1/0 is " << myinf << '\n';
    }

    // HUGE_VAL macro
    double huge = pow(10.0, 1000000000);
    if (huge == HUGE_VAL) {
        std::cout << huge << " is HUGE_VAL\n";
    }

    // fpclassify() returns a value of type int that matches one of the classification macro constants
    //FP_INFINITE	Positive or negative infinity(overflow)
    //FP_NAN	Not - A - Number
    //FP_ZERO	Value of zero
    //FP_SUBNORMAL	Sub - normal value(underflow)
    //FP_NORMAL	Normal value(none of the above)
    print_fpclassify(1 / sin(0.0));
    print_fpclassify(sin(0.0) / sin(0.0));
    print_fpclassify(0.0);
    print_fpclassify(1.0);
    print_fpclassify(-1.0);

    // subnormal numbers could not be represent a normal double
    // (less than minimal double)
    // requires expanded representation, works slower (10-100 times)
    double subnorm = 1.0;
    while (fpclassify(subnorm) != FP_SUBNORMAL) {
        subnorm /= 2;
    }
    print_fpclassify(subnorm);
}



int main()
{
    // TODO: insert any function call
    return 0;
}