
#include <iostream>

namespace demo { int value = 1; }

#define value 100

int main()
{
    std::cout << demo::value << "\n";
    return 0;
}
