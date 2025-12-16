
#include <iostream>

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
