
#include <iostream>

namespace units {
struct Meter { int v; };
Meter operator+(Meter a, Meter b) { return {a.v + b.v}; }
}

int main()
{
    units::Meter a{2}, b{3};
    auto c = a + b;
    std::cout << c.v << "\n";
    return 0;
}
