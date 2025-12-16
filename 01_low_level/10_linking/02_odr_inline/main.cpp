#include "odr_header.hpp"
void from_a();
void from_b();

/*
## Example 2 — **ODR and Inline Variables (C++17)**

**Folder:** `02_odr_inline/`

**What it covers:**

* One Definition Rule (ODR)
* Why non-inline function definitions in headers break linking
* Correct use of `inline` functions
* **Inline variables (C++17)** as a replacement for `extern` globals
* Header-only constants and objects
* Safe shared state across translation units

**Key takeaway:**

Inline variables finally make "header-defined globals" well-defined and safe.
*/

int main()
{
    from_a();
    from_b();
    bump();
    return 0;
}
