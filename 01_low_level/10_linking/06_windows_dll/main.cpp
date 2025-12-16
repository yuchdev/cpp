#include "dll_api.h"
#include <iostream>

/*
## Example 6 — **Windows DLL Import / Export**

**Folder:** `06_windows_dll/`

**What it covers:**

* Windows-specific linking model
* `__declspec(dllexport)` / `__declspec(dllimport)`
* Import libraries
* Cross-platform export macros
* Differences between ELF and PE/COFF models
* Guarded compilation for portability

**Key takeaway:**

> On Windows, symbol export is *opt-in* — forgetting it breaks linking even when code compiles.
*/


int main()
{
    std::cout << "dll_add=" << dll_add(2,3) << "\n";
    return 0;
}
