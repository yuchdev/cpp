#include <iostream>

using namespace std;

/*

New features:
* inline namespaces (14.4.6)

Examples:
1. Inline namespace
*/

namespace cpp4
{

namespace V1_0
{
void f()
{
    std::cout << "API version 1.0\n";
}
}

namespace V1_5
{
void f()
{
    std::cout << "API version 1.5\n";
}
}

// The inline specifies that V2_0 is the default meaning of cpp4
inline namespace V2_0
{
void f()
{
    std::cout << "API version 2.0\n";
}
}

} // namespace cpp4 

void show_inline_namespaces()
{
    cpp4::V1_0::f();
    cpp4::V1_5::f();
    cpp4::V2_0::f();

    // inline namespace works
    cpp4::f();
}

int main()
{
    show_inline_namespaces();
    return 0;
}
