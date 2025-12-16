#include "file2.h"
#include <iostream>

int x = 1;

const int y = 1;
using T = char;

namespace
{
    int i = 0;
    struct invisible_to_others {};
}

void show_extern()
{
    std::cout << "x=" << x << " y=" << y << " i=" << i << "\n";
    ++i;
}
