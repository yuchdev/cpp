#include <iostream>

/*
## Example 6 — Inline Namespaces (Versioning & ABI Evolution)

**File:** `06_inline_namespaces.cpp`

### Covers

* Inline namespaces
* API versioning
* ABI-safe evolution
* Default namespace selection

### Key ideas

* Inline namespaces are the **only namespace feature that affects ABI**
* Used heavily in standard library implementations
* Allows backward compatibility without breaking user code
*/

namespace api {

namespace v1 { void f(){ std::cout<<"v1\n"; } }
inline namespace v2 { void f(){ std::cout<<"v2\n"; } }

}

int main()
{
    api::v1::f();
    api::f();
    return 0;
}
