
#include <iostream>

namespace my {
    int a = 1;
}

namespace my {
    int b = 2;
}

namespace {
    int hidden = 42;
}

namespace my_alias = my;

int main()
{
    my::a = 10;
    my::b = 20;
    my_alias::a = 30;

    std::cout << "my::a=" << my::a << " my::b=" << my::b << "\n";
    std::cout << "hidden=" << hidden << "\n";
    return 0;
}
