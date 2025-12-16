#include <iostream>

/*
## Example 4 — Namespaces and Operator Overloading

**File:** `04_namespaces_and_operators.cpp`

### Covers

* Operators defined in namespaces
* Operator lookup via ADL
* Why operators should be free functions, not members

### Key ideas

* `a + b` works because `operator+` is found in the argument’s namespace
* Namespaces are part of overload resolution
* This is why STL operators are not members
*/

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
