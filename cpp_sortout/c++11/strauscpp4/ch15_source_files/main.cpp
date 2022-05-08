#include <iostream>

using namespace std;

/*

New features:
* Abandoning a process: quick_exit(15.4.3)

Examples:
1. quick_exit()
*/
void my_cleanup()
{
    std::cout << "Custom at_quick_exit() cleanup\n";
}

// The quick_exit() function is like exit() except that it does not invoke any destructors
// You register functions to be invoked by quick_exit() using at_quick_exit()
[[noreturn]] 
void my_return() 
{
    quick_exit(0);
}

int main() 
{
    if (0 == std::at_quick_exit(&my_cleanup)) {
        // ok
    }
    else {
        std::cout << "Too many at_quick_exit() functions\n";
    }

    my_return();

    return 0;
}
