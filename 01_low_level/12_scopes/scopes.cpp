#include <cstdlib>
#include <iostream>

// This is a question from one interview. The question is 
// "What value boo(-1) function call return?"
// Got it?
// Answer: The else actually matches with the second if (C++ Standard 6.4.1/1)
// Tabs arranged to make the answer not that obvious
// (especially to Python programmers)
// Clang even shows warning on this situation, which gives this compiler additional points
int boo(int x){
    if (x > 0)
        if (x > 100)
            return 1;
    else
        return 2;
    return 0;
}


// User-defined function of global cleanup
// Passed as function pointer in atexit() function
// Only limited number of atexit() calls is possible (64 for VC++)
void my_cleanup()
{
    std::cout << "cleanup handler" << std::endl;
}

int main()
{
    // Change the way of exiting the abblication
    // exit(); Soft exit, call the destructors
    // abort(); Hard exit, does not call destructors
    if (atexit(&my_cleanup) == 0) {
        // We'll call my_cleanup on exit
    }
    else {
        // Reached the limit of atexit() calls
        return 1;
    }

    return 0;
}
