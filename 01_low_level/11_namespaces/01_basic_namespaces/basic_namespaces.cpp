#include <iostream>

/*
## Example 1 — Basic Namespaces, Merging, Aliases, Unnamed Namespace

**File:** `01_basic_namespaces.cpp`

### Covers

* Named namespaces
* Namespace merging (same namespace opened multiple times)
* Namespace aliases
* Unnamed (anonymous) namespaces
* Internal linkage via unnamed namespaces

### Key ideas

* Namespaces can be reopened anywhere — even across libraries
* Anonymous namespaces replace `static` globals
* Namespace aliases are essential for versioning and long names
*/

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
