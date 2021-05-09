#include <cstdlib>
#include <iostream>

// User-defined function of global cleanup
// Passed as function pointer in atexit() function
// Only limited number of atexit() calls is possible (64 for VC++)
void my_cleanup()
{
	// нельзя увидеть из отладчика
	std::cout << "cleanup handler" << std::endl;
}

int main()
{
	// Change the way of exiting the abblication
	// exit(); Soft exit, call the destructors
	// abort(); Hard exit, does not call destructors
	if ( atexit( &my_cleanup ) == 0 )
	{
		// We'll call my_cleanup on exit
	}
	else
	{
		// Reached the limit of atexit() calls
		return 1;
	}

	return 0;
}
