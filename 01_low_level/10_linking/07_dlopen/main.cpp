#include "plugin_api.h"
#include <iostream>

/*
## Example 7 — **Dynamic Loading at Runtime (`dlopen` / `dlsym`)**

**Folder:** `07_dlopen/`

**What it covers:**

* Runtime linking vs static linking
* `dlopen`, `dlsym`, `dlclose`
* Why `extern "C"` is mandatory for plugins
* ABI stability requirements
* Function pointer casting
* Plugin architectures

**Key takeaway:**

> Runtime linking bypasses the linker — *you* become responsible for correctness.
*/

#if defined(__unix__) || defined(__APPLE__)
  #include <dlfcn.h>
#endif

int main()
{
#if defined(__unix__) || defined(__APPLE__)
    void* h = dlopen("./libplugin.so", RTLD_NOW);
    if (!h) { std::cerr << "dlopen failed: " << dlerror() << "\n"; return 1; }

    void* sym = dlsym(h, "plugin_add");
    if (!sym) { std::cerr << "dlsym failed: " << dlerror() << "\n"; return 2; }

    auto add = reinterpret_cast<plugin_add_fn>(sym);
    std::cout << "plugin_add(2,3)=" << add(2,3) << "\n";

    dlclose(h);
    return 0;
#else
    std::cout << "[info] dlopen/dlsym demo is POSIX-only\n";
    return 0;
#endif
}
