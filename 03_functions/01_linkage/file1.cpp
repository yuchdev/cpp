#include "file1.h"

// global variable x defined in file2.cpp, but declared extern here in file1.cpp
extern int x;

// we can repeat extern declaration more than one time, because extern does not deplare anything new
extern int x;

// function show_extern defined in file2.cpp as well
extern void show_extern();

// constant and typedefs nave internal linkage
// therefore could be defined in more than one file
const int y = 2;
typedef int T;

// It's possible to define explicitly extern const variable though
// It defines a global constant variable that can be referenced from other modules.
extern const int x1 = 10;

void use_extern(){
	show_extern();
	x = 10;
}

void c_function1(){}

void c_function2(){}

void c_function3(){}
