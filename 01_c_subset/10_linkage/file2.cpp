#include "file2.h"
#include <iostream>

// global variable x defined in file2.cpp, but declared extern here in file1.cpp
int x = 1;

// constant and typedefs nave internal linkage
// therefore could be defined in more than one file
const int y = 1;
typedef char T;

// "i" has external linkage
// Even though "i" is not visible outside the compilation unit, 
// it still has external linkage (C++ Standard 7.3.1.1/1 - See footnote)
namespace{
	int i;
}

void show_extern()
{
	std::cout << x << std::endl;
}
