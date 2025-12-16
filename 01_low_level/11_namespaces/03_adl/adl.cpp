#include <iostream>

/*
## Example 3 — Argument-Dependent Lookup (ADL / Koenig Lookup)

**File:** `03_adl.cpp`

### Covers

* ADL rules
* Unqualified function calls resolved via argument types
* Why operators and free functions belong in the same namespace as their types

### Key ideas

* ADL enables clean syntax: `normalize(v)` instead of `mathlib::normalize(v)`
* ADL is fundamental for operator overloading
* Overusing ADL can cause surprising ambiguities
*/

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
