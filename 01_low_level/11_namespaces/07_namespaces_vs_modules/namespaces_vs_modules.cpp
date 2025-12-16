#include <iostream>

/*
## Example 7 — Namespaces vs Modules (C++20 Context)

**File:** `07_namespaces_vs_modules.cpp`

### Covers

* Namespace limitations
* Macro leakage across namespaces
* Why namespaces are *not* modules

### Key ideas

* Namespaces do not protect against macros
* They do not prevent ODR violations
* C++20 modules solve problems namespaces never could
*/

namespace demo { int value = 1; }

#define value 100

int main()
{
    std::cout << demo::value << "\n";
    return 0;
}
