#include <iostream>
#include <cstdint>
#include <utilities.h>

// This is a question from one interview. The question is 
// "What value boo(-1) function call return?"
// Got it?
// Answer: The else actually matches with the second if (C++ Standard 6.4.1/1)
// Tabs arranged to make the answer not that obvious
// (especially for Python programmers)
// Clang even shows warning on this situation, which gives it additional points
int boo(int x){
    if (x > 0)
        if (x > 100)
            return 1;
    else
        return 2;
    return 0;
}

// Why rand() is a very slow function. It calls Enter/LeavCriticalSection 
// and locks data bus, which may kill your application performance, 
// especially if the architecture relying on multiple CPU.
// Here's some example
void show_rand()
{
    volatile int x;
    int n = 10000000;
    MeasureTime m;

    std::cout << "Start random number generation with rand()\n";
    for (int i = 0; i < n; ++i) {
        x = rand();
    } 
    std::cout << "Random number generation with rand() took " <<  m.elapsed_mcsec() << '\n';
}


int main()
{
    return 0;
}