#include "templ.hpp"
#include <iostream>

/*
## Example 3 — **Templates and Explicit Instantiation**

**Folder:** `03_templates/`

**What it covers:**

* Template instantiation model
* Why templates defined only in `.cpp` files fail to link
* Implicit vs explicit instantiation
* How to intentionally control where code is generated
* Diagnosing "undefined reference" template errors

**Key takeaway:**

> Templates are compiled *on demand* — if the linker can't find an instantiation, it won't invent one.
*/

int main()
{
    std::cout << "mul(2,3)=" << mul(2,3) << "\n";
    std::cout << "declared_only<int>(2,3)=" << declared_only<int>(2,3) << "\n";
    std::cout << "declared_only<double>(2,3)=" << declared_only<double>(2,3) << "\n";

    // Uncomment to trigger link error:
    // std::cout << declared_only<long long>(1,2) << "\n";
    return 0;
}
