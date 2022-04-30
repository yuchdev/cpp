#include "file1.h"
#include "file2.h"

#include <cstdlib>
#include <iostream>

// Global variables definition order is not defined
int g = 0;
int d = g*g;

// Use Singleton instead or static object returned 
// by reference (Meyers Singleton, thread-safe)
int& use_count()
{
	static int res = 0;
	return res;
}

// User-defined function of global cleanup
// Passed as a pointer to a function into atexit() function
// It is possible to pass multiple, but limited number of functions
void my_cleanup()
{
	std::cout << "cleanup handler" << std::endl;
}

// Static and extern examples:
// in namespace or global scope
int i = 0; // extern by default
const int ci = 0; // static (intern) by default
extern const int eci; // explicitly extern
static int si = 0; // explicitly static (intern)

// the same goes for functions (but there are no const functions)
int foo(); // extern by default
static int bar(); // explicitly static (intern)

// Note that instead of using static for internal linkage 
// it is better to use anonymous namespaces into which you can also put classes.
// The linkage for anonymous namespaces has changed between C++98 and C++11 
// but the main thing is that they are unreachable from other translation units.

namespace {
	int i; // external linkage but unreachable from other translation units.
	class invisible_to_others { }; // external linkage but unreachable 
}

int main()
{
	// Here we use external definitions in different object files
	use_extern();
	show_extern();

	// Different ways to interrupt the application execution
	// exit(); //  calls global destructors
	// abort(); // does not call global destructors

	if ( atexit( &my_cleanup ) == 0 )
	{
		// my_cleanup will be called
	}
	else
	{
		// some problem of global cleanup
		return 1;
	}

	return 0;
}
