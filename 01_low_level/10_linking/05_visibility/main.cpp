#include "visibility.h"
#include <iostream>

/*
## Example 5 — **Symbol Visibility and Shared Libraries (ELF / Mach-O)**

**Folder:** `05_visibility/`

**What it covers:**

* Symbol visibility in shared libraries
* `-fvisibility=hidden`
* `__attribute__((visibility))`
* Exported vs hidden symbols
* Dynamic symbol tables
* Reducing ABI surface and link-time costs

**Key takeaway:**

> Not every global symbol should be part of your public ABI — visibility is a design decision.
*/

int main()
{
    std::cout << "exported_add=" << exported_add(1,2) << "\n";
    std::cout << "hidden_add=" << hidden_add(1,2) << "\n";
    return 0;
}
