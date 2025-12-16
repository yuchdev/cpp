
#include <iostream>

namespace mathlib {
struct Vec { int x; };
void normalize(const Vec&) {
    std::cout << "mathlib::normalize\n";
}
}

int main()
{
    mathlib::Vec v{1};
    normalize(v); // ADL
    return 0;
}
