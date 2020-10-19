#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <iomanip>
#include <cmath>

// OsX workaround
#include <cfloat>
#include <cstdint>
#include <fenv.h>

#include <bitwise.h>

// The floating-point environment maintains a series of status flags and specific control modes.
// Specific about the contents of the floating-point environment depend on the implementation,
// but the status flags generally include the floating-point exceptions and their associated information,
// and the control modes include at least the rounding direction.

void fp_coltrol_noexcept()
{
	std::cout << "Floating point state flags:\n";
    std::cout << " FE_DIVBYZERO = " << FE_DIVBYZERO << '\n';
    std::cout << " FE_INEXACT = " << FE_INEXACT << '\n';
    std::cout << " FE_INVALID = " << FE_INVALID << '\n';
    std::cout << " FE_OVERFLOW = " << FE_OVERFLOW << '\n';
	std::cout << " FE_UNDERFLOW = " << FE_UNDERFLOW << '\n';

	std::cout << "Floating point rounding flags:\n";
	std::cout << " FE_DOWNWARD = " << FE_DOWNWARD << '\n';
	std::cout << " FE_TONEAREST = " << FE_TONEAREST << '\n';
	std::cout << " FE_TOWARDZERO = " << FE_TOWARDZERO << '\n';
	std::cout << " FE_UPWARD = " << FE_UPWARD << '\n';

	std::cout << "Current exceptions flags = " << fetestexcept(FE_ALL_EXCEPT) << '\n';
	std::cout << "Current rounding mode = " << fegetround() << '\n';

	// Clear exception flags
	feclearexcept(FE_ALL_EXCEPT);

    // Create Inexact/Underflow error
	double d1 = 1.0;
	size_t steps{};
	int res{};
	do {
		d1 /= 2.0;
        ++steps;
	} 
	while ((res = fetestexcept(FE_ALL_EXCEPT)) == 0);
	std::cout << "Inexact/Underflow Exceptions in " << steps <<" steps: " << res << '\n';
	std::cout << "2^-inf: " << d1 << '\n';

	feclearexcept(res);

	// Create Inexact/Overflow error
	double d2 = 1.0;
    steps = 0;
	do {
		d2 *= 2.0;
        ++steps;
	} 
	while ((res = fetestexcept(FE_ALL_EXCEPT)) == 0);
	std::cout << "Inexact/Overflow Exceptions in " << steps << " steps: " << res << '\n';
	std::cout << "2^+inf: %g\n" << d2 << '\n';

	feclearexcept(res);

	// Create zero division error
	double d3 = 1.0 / d1;
	res = fetestexcept(FE_ALL_EXCEPT);
	std::cout << "Zero Div Exceptions:" << res << '\n';
	std::cout << "1/0:" << d3 << '\n';

	feclearexcept(res);

	// Create Inexact error
	double s = 1.0;
    steps = 0;
	do {
		s = (s + 2 / s) * 0.5;
        ++steps;
	} 
	while ((s * s - 2) > 0);
	std::cout << "Inexact Exceptions in " << steps << " steps\n";
	std::cout << "sqrt (2): %g\n" << '\n';
}

void fp_coltrol()
{
	// Floating Point Environment is a system-wide variable
	// which value is being set (and never clear!) on floating point exception/error situation, 
    // and keep additional information about the error
	// FP is probably set by 'stmxcsr' assembly instruction in special SSE MXCSR register

	// default FP environment (system-dependent!)
	fenv_t fenv = *FE_DFL_ENV;

	// get current FP environment
	int ret = fegetenv(&fenv);
	std::cout << "fegetenv() returned " << ret << '\n'; // 0 is no errors

	// pair function setting fenv_t
	ret = fesetenv(&fenv);
	std::cout << "fesetenv() returned " << ret << '\n'; // 0 is no errors

	// Attempts to clear the floating-point exceptions specified
	feclearexcept(FE_ALL_EXCEPT);
	ret = fegetenv(&fenv);

	// feholdexcept() saves exception flags clears 
	double x = sin(0.0); // = 0
	fenv_t fe;
	feholdexcept(&fe);
	x = log(x); // should raise exception, but fenv is clear

	// feupdateenv() function is responsible quite complicated actions
	// It saves in local memory information about exception,
	// set the new environment by fenvp value and raise in that environment saved exception
	feupdateenv(&fe);
	std::cout << "log 0 = " << x << '\n';

	if (!fetestexcept(FE_ALL_EXCEPT)) {
		std::cout << "no exceptions raised" << '\n';
	}
}

#ifdef _MSC_VER
void fp_exceptions_ms()
{
	// MS-specific way to trap FP exceptions
    //Set the x86 floating-point control word according to what
    //exceptions you want to trap
    _clearfp(); //Always call _clearfp before setting the control

    //word
    //Because the second parameter in the following call is 0, it
    //only returns the floating-point control word

    unsigned int cw = _controlfp(0, 0); //Get the default control

    //word
    //Set the exception masks off for exceptions that you want to
    //trap. When a mask bit is set, the corresponding floating-point
    //exception is //blocked from being generating.

    cw &= ~(EM_OVERFLOW | EM_ZERODIVIDE | EM_DENORMAL | EM_INVALID);

    //For any bit in the second parameter (mask) that is 1, the
    //corresponding bit in the first parameter is used to update
    //the control word.

    unsigned int cwOriginal = _controlfp(cw, MCW_EM); //Set it.

    //MCW_EM is defined in float.h.
    //Restore the original value when done:
    _controlfp(cwOriginal, MCW_EM);
}
#endif // _MSC_VER

int main()
{
    // TODO: insert any function call
    return 0;
}