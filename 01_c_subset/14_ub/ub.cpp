#include <utilities/defines.h>
SUPPRESS_PRAGMA_WARNINGS()
SUPPRESS_UNINITIALIZED_WARNINGS()
#include <iostream>


#pragma clang diagnostic ignored "-Wuninitialized"


void initialization_order()
{
    // example of UB due to bad initialization
    int i = i * 0;

    // no defined order of initialization within one expression

    // Suppress optimization
    std::cout << "Value of this variable is undefined: i = " << i << std::endl;
}


int main()
{
    return 0;
}

// Examples of int/uint UB
// https://stackoverflow.com/questions/18195715/why-is-unsigned-integer-overflow-defined-behavior-but-signed-integer-overflow-is
// https://stackoverflow.com/questions/7488837/why-is-int-rather-than-unsigned-int-used-for-c-and-c-for-loops
// https://stackoverflow.com/questions/199333/how-do-i-detect-unsigned-integer-multiply-overflow
// https://stackoverflow.com/questions/7488837/why-is-int-rather-than-unsigned-int-used-for-c-and-c-for-loops
// https://stackoverflow.com/questions/22587451/c-c-use-of-int-or-unsigned-int
