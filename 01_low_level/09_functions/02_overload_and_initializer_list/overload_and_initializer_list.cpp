
#include <iostream>
#include <initializer_list>
#include <vector>
#include <algorithm>

template <typename... Ts>
inline void println(Ts&&... xs)
{
    (std::cout << ... << xs) << '\n';
}

void print(int)        { println("print(int)"); }
void print(double)     { println("print(double)"); }
void print(const char*){ println("print(const char*)"); }

void takes_list(std::initializer_list<int> il)
{
    println("initializer_list size =", il.size());
}

int main()
{
    print(1);        // exact
    print(3.14);     // exact
    print("hello");  // exact

    takes_list({1,2,3,4});

    // takes_list(1); // error: initializer_list preferred only for {}
    return 0;
}
