#include <cfloat>
#include <cmath>
#include "zero_divide.h"

// suppress floating-point warnings
#define _CRT_SECURE_NO_WARNINGS

#if defined(_WIN32) || defined(_WIN64)

void set_fpe_zero_div2()
{
    //Set the x86 floating-point control word according to what
    //exceptions you want to trap.
    _clearfp(); //Always call _clearfp before setting the control word

    //Because the second parameter in the following call is 0, it
    //only returns the floating-point control word

    //Get the default control word
    unsigned int cw = _controlfp(0, 0);

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

    _asm {
        stmxcsr[cntrReg] //Get MXCSR register
        and [cntrReg], 0FFFFFF7Fh //bit 7 - invalid instruction mask
            //bit 9 - divide-by-zero mask
            //bit 10 - overflow mask
            //bit 11 - underflow mask
            ldmxcsr[cntrReg] //Load MXCSR register
    }
}

// warning C4996: '_controlfp': This function or variable may be unsafe
void set_zero_div_exception()
{
    // clear FP flags is performed before each FP word chang
    _clearfp();

    // get the current state
    unsigned int cw = _controlfp(0, 0);

    // an exception will be thrown for division by 0, overflow, invalid operation (0/0, inf/inf)
    cw &= ~(EM_OVERFLOW | EM_ZERODIVIDE | EM_DENORMAL | EM_INVALID);

    // set FP word
    unsigned int cwOriginal = _controlfp(cw, MCW_EM);
}

#endif

int zero_div(double s)
{
    double sn = sin(s);
    int isn = static_cast<int>(sn);
    isn = 1 / isn;
    return isn;
}

double fpe_zero_div(double s)
{
    double sn = sin(s);
    return 1.0 / sn;
}
