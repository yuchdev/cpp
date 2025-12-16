#include <cstdio>

extern "C" {
#include <string.h>
}

/*
## Example 5 — Wrapping C Libraries with Namespaces

**File:** `05_wrapping_c.cpp`

### Covers

* `extern "C"` headers
* Wrapping C APIs into C++ namespaces
* Avoiding global namespace pollution
* ABI stability

### Key ideas

* C has no namespaces — C++ must compensate
* Wrapping C APIs is *recommended practice*
* Namespaces do **not** change linkage or ABI
*/

namespace cwrap {

inline size_t length(const char* s) { return strlen(s); }

}

int main()
{
    const char* s = "hello";
    std::printf("len=%zu\n", cwrap::length(s));
    return 0;
}
