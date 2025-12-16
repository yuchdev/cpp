#include "file1.h"
#include "file2.h"
#include <iostream>

/*

## Example 1 — **Basic Linkage, `extern`, and Translation Units**

* Translation units and separate compilation
* `extern` declarations vs definitions
* Repeated `extern` declarations
* Internal vs external linkage
* Anonymous namespaces vs `static`
* `extern "C"` for ABI compatibility
* Header wrapping for C libraries
* Name visibility vs linkage

**Key takeaway:**

> Names may be visible, invisible, shared, or isolated — *visibility and linkage are not the same thing*.

---
*/

int main()
{
    use_extern();
    show_extern();

    c_function1();
    c_function2();
    c_function3();

    std::cout << "ok\n";
    return 0;
}
