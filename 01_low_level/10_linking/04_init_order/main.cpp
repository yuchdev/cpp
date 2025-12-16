#include <iostream>
#include <string>

/*
## Example 4 — **Static Initialization Order & `constinit` (C++20)**

**Folder:** `04_init_order/`

**What it covers:**

* Static initialization order fiasco
* Cross-translation-unit global initialization
* Construct-on-first-use idiom (function-local static)
* Thread safety guarantees since C++11
* **`constinit` (C++20)** to enforce static initialization
* Difference between `const`, `constexpr`, and `constinit`

**Key takeaway:**

> Initialization order across translation units is undefined — unless you explicitly force it.

*/

void print_a();
std::string& global_name();

#if __cplusplus >= 202002L
constinit int g_static_init = 42;
#endif

int main()
{
#if __cplusplus >= 202002L
    std::cout << "constinit g_static_init=" << g_static_init << "\n";
#else
    std::cout << "[info] constinit requires C++20\n";
#endif

    print_a();
    std::cout << "global_name()=" << global_name() << "\n";
    return 0;
}
