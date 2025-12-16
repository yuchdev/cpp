
#include <iostream>

namespace A {
    int x = 1;
    void f() { std::cout << "A::f\n"; }
}

namespace B {
    int x = 2;
    void f() { std::cout << "B::f\n"; }
}

int main()
{
    using A::x;
    x = 100;
    A::f();
    B::f();
    return 0;
}
