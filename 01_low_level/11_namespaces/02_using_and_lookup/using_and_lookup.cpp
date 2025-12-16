#include <iostream>

/*
## Example 2 — `using` Declarations vs `using` Directives

**File:** `02_using_and_lookup.cpp`

### Covers

* `using namespace X;` (directive)
* `using X::name;` (declaration)
* Name hiding vs ambiguity
* Why `using` directives are dangerous in headers

### Key ideas

* Prefer **using-declaration** over **using-directive**
* `using namespace` is acceptable only in *local scopes*
* Headers must never contain `using namespace`
*/

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
