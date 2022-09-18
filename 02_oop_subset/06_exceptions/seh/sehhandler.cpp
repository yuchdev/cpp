#include "sehhandler.h"
#include <iostream>

#if defined(_WIN32) || defined(_WIN64)
#include "windows.h"

using std::cout;
using std::endl;

// Filter function or translator. Translator accept a SE
// defined by the error code, as well as info structure
// Translator can convert SE into a C++ exception
// Note: In a multithreaded program, each thread has a separate translator
int sehhandler::filter(unsigned code, struct _EXCEPTION_POINTERS* ep)
{

    // Generated a C++ exception on Access violation SE
    if (code == EXCEPTION_ACCESS_VIOLATION) {
        cout << "Access violation" << endl;
        return EXCEPTION_EXECUTE_HANDLER;
    }
    else {
        cout << "Unexpected exception" << endl;
        return EXCEPTION_CONTINUE_SEARCH;
    }

}

void sehhandler::test_seh()
{
    int* p = 0x0;
    // SE protected block
    __try {
        cout << "In try 1" << endl;

        // Both handlers `__except` and `__finally` cannot be implemented at once due to WinAPI restrictions
        // To simulate the expected try-except-finally model, the handlers can be nested inside each other
        __try {
            cout << "In try 2.1" << endl;
            *p = 13;
            cout << "In try 2.2" << endl;
        }
        // Finalization block: we will get here in any case
        __finally {
            cout << "In finally" << endl;

            // If the __try block terminated abnormally, the return value is nonzero.
            // If the __try block terminated normally, the return value is zero.
            BOOL abnormal = AbnormalTermination();
            cout << "Abnormal = " << abnormal << endl;
        }
        // Inside the __try part of the try block you can use `leave` TODO: what was it
    }
    // We come here depending on filter return code; see `__except` keyword MSVC documentation
    // filter is called only if SEH exception occures
    __except (filter(GetExceptionCode(), GetExceptionInformation())) {

        // EXCEPTION_CONTINUE_EXECUTION tries to execute the instruction that caused the exception once again
        // Could be realized if the filter itself tried to fix the problem
        // EXCEPTION_CONTINUE_SEARCH allowing throw an exception further
        // EXCEPTION_EXECUTE_HANDLER execute the `__except` handler body after `__finally`
        // if we want to handle exceptions every time, like in a top-level handler
        // instead of a filter function you can pass the constant
        cout << "In except" << endl;
    }
}
#endif
