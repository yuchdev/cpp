#pragma once

// The SEH mechanism is demonstrated
// It is specific for the Windows platform
#if defined(_WIN32) || defined(_WIN64)

// Structured Exception Handling (SEH) in fact is a collection of tools
// provided by Windows API and available to programs written in any language

class sehhandler
{
public:
    // The good thing is to translate SE into C++ exception by setting _set_se_translator()
    int filter(unsigned code, struct _EXCEPTION_POINTERS* ep);
    void test_seh();
};

/*
MS have defined several synonyms to the same function
E.g. `excpt.h` has the following declarations:
`unsigned long __cdecl _exception_code(void)`
`#define GetExceptionCode _exception_code `
`#define exception_code _exception_code`

This is likely done to support different coding conventions
*/


/*

SEH Access Violation exception means,  using invalid pointer, or heap damage
This cannot be handled, so the better option is to crash and debug the dump
However, for some applications, it could be a requirement to continue working, issuing only a warning
EXCEPTION_STACK_OVERFLOW definitely cannot be handled like a regular exception,
it requires restoring the stack after overflow calling `_resetstkoflw()` function,
likely from `__except` block

*/

#endif
