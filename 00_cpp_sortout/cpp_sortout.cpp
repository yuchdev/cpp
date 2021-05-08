#include <iostream>
#include <cstdint>
#include <elapsed.h>

// This is a question from one interview. The question is 
// "What value boo(-1) function call return?"
// Got it?
// Answer: The else actually matches with the second if (C++ Standard 6.4.1/1)
// Tabs arranged to make the answer not that obvious
// (especially for Python programmers)
// Clang even shows warning on this situation, which gives this compiler additional points
int boo(int x){
    if (x > 0)
        if (x > 100)
            return 1;
    else
        return 2;
    return 0;
}

void naming_conventions()
{
    // Give variables with large scope long and clear names
    static int test_counter = a;

    // It's okay to give short names to variables with small scope
    for (int i = 0; i < test_counter; i++){
        ::g_global += test_counter;
        cout << ::g_global << endl;
    }

    // access global variable
    ::g_global = 0;

    // global variable shadowing
    int g_global = 0;

    // There's no way to access 
    // a shadowed local variable
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


// Alignment
struct S { char c; int i; };

// Wrong compare
bool is_equal(S a, S b) { return 0 == memcmp(&a, &b, sizeof(S)); }

int main()
{
    return 0;
}