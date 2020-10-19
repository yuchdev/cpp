#define _USE_MATH_DEFINES
#include <iostream>
#include <iomanip>
#include <cmath>

// OsX workaround
#include <cfloat>
#include <cstdint>

#include <bitwise.h>

void fp_coltrol_noexcept()
{

	double d1 = 0., d2 = 0., d3 = 0., s = 0.;
	int res = 0;

	printf("Floating point state flags:\n");
	printf(" FE_DIVBYZERO: %x\n", FE_DIVBYZERO);
	printf(" FE_INEXACT: %x\n", FE_INEXACT);
	printf(" FE_INVALID: %x\n", FE_INVALID);
	printf(" FE_OVERFLOW: %x\n", FE_OVERFLOW);
	printf(" FE_UNDERFLOW: %x\n", FE_UNDERFLOW);

	printf("Floating point rounding flags\n");
	printf(" FE_DOWNWARD: %x\n", FE_DOWNWARD);
	printf(" FE_TONEAREST: %x\n", FE_TONEAREST);
	printf(" FE_TOWARDZERO: %x\n", FE_TOWARDZERO);
	printf(" FE_UPWARD: %x\n", FE_UPWARD);

	printf("Current exceptions flags: %x\n", fetestexcept(FE_ALL_EXCEPT));

	printf("Current rounding mode: %x\n", fegetround());

	// Clear exception flags
	feclearexcept(FE_ALL_EXCEPT);

        // Create Inexact/Underflow error
	d1 = 1.0;
    size_t steps = 0;
	do {
		d1 /= 2.0;
        ++steps;
	} while ((res = fetestexcept(FE_ALL_EXCEPT)) == 0);
    printf("Inexact/Underflow Exceptions in %d steps: %x\n", steps, res);
	printf("2^-inf: %g\n", d1);

	feclearexcept(res);

	// Create Inexact/Overflow error
	d2 = 1.0;
    steps = 0;
	do {
		d2 *= 2.0;
        ++steps;
	} while ((res = fetestexcept(FE_ALL_EXCEPT)) == 0);
    printf("Inexact/Overflow Exceptions in %d steps: %x\n", steps, res);
	printf("2^+inf: %g\n", d2);

	feclearexcept(res);

	// Create zero division error
	d3 = 1 / d1;
	res = fetestexcept(FE_ALL_EXCEPT);
	printf("Zero Div Exceptions: %x\n", res);
	printf("1/0: %g\n", d3);

	feclearexcept(res);

	// Create Inexact error
	s = 1;
    steps = 0;
	do {
		s = (s + 2 / s) * 0.5;
        ++steps;
	} while ((s * s - 2) > 0);
    printf("Inexact Exceptions in %d steps: %x\n", steps, fetestexcept(FE_ALL_EXCEPT));
	printf("sqrt (2): %g\n", s);
}

void fp_coltrol(){

	/*
	The floating-point environment maintains a series of status flags and specific control modes.
	Specific about the contents of the floating-point environment depend on the implementation,
	but the status flags generally include the floating-point exceptions and their associated information,
	and the control modes include at least the rounding direction.
	*/

	// FE_* macro - floating point exceptions and rounding rules

	// 1. State (exception) flags
	//FE_DIVBYZERO Pole error exception(macro)
	//FE_INEXACT Inexact result exception(macro)
	//FE_INVALID Invalid argument exception(macro)
	//FE_OVERFLOW Overflow range error exception(macro)
	//FE_UNDERFLOW Underflow range error exception(macro)
	//FE_ALL_EXCEPT All exceptions(macro)

	// 2. Rounding rules
	//FE_DOWNWARD Downward rounding direction mode
	//FE_TONEAREST To - nearest rounding direction mode(macro)
	//FE_TOWARDZERO Toward - zero rounding direction mode(macro)
	//FE_UPWARD Upward rounding direction mode(macro)

	int ret = 0;

	// http://www.intuit.ru/studies/courses/53/53/lecture/1585?page=9
	// http://ru.wikipedia.org/wiki/Fenv.h
	// http://www.cplusplus.com/reference/cfenv/

	// Floating Point Environment is a system-wide variable
	// which value is being set (and never clear!) on floating point exception/error situation, 
        // and keep additional information about the error

	// default FP environment (system-dependent!)
	fenv_t fenv = *FE_DFL_ENV;

	// get current FP environment
	ret = fegetenv(&fenv);
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
	if (!fetestexcept(FE_ALL_EXCEPT))
		std::cout << "no exceptions raised" << '\n';
}


void fp_exceptions_MXCSR()
{
    //Set the x86 floating-point control word according to what
    //exceptions you want to trap.

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
    //_controlfp(cwOriginal, MCW_EM);

    unsigned long cntrReg;

    _asm
    {
        stmxcsr[cntrReg] //Get MXCSR register

        //bit 7 - invalid instruction mask
        //bit 9 - divide-by-zero mask
        //bit 10 - overflow mask
        //bit 11 - underflow mask
        and [cntrReg], 0FFFFFF7Fh

        ldmxcsr[cntrReg] //Load MXCSR register
    }
}

int main()
{
    // TODO: insert any function call
    return 0;
}