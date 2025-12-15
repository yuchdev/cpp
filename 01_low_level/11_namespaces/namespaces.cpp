#include <vector>
#include <iostream>

// Unnamed and named exceptions, aliases, merge namespaces
// * standard usecases of namespaces:
//   * structure the code base
//   * prevent naming conflicts
//   * wrap plain C code 


// Simple names namespace
namespace my
{
    int belongs_to_my{};
}

// When we use same namespace specifier, namespaces are being merged
// Such namespaces could be declared anywhere, in different translation units, in different libraries
namespace my
{
    int also_belongs_to_my{};
}

// Unnamed namespace anonymous
// Could be used for resolving name conflicts, and preventing access from other translation units
namespace
{
    int belongs_to_anonymous{};
}


// One more way to merge two namespaces inside third namespace
namespace your1
{
    int belongs_to_your1;
}

namespace your2
{
    int belongs_to_your2;
}

namespace your
{
    using namespace your1;
    using namespace your2;
}

// Namespace alias
// Could be convenient while using namespaces vor versioning
namespace my_namespace_alias = my;


// It is recommended to bring interface of C library into distinct namespace
/*
namespace CLib
{
int printf();
}
*/

// While namespace member is not being used, it can't cause name conflict
// Example:
namespace unused_namespace_a
{
    int unused_int{};
}

namespace unused_namespace_b
{
    int unused_int{};
}

namespace unused_namespaces
{
    using namespace unused_namespace_a;
    using namespace unused_namespace_b;
}


void show_namespaces()
{
    my::belongs_to_my = 1;
    your1::belongs_to_your1 = 2;

    // Merged namespaces your1 and your2
    your::belongs_to_your1 = 3;

    // using-directive could be utilized in a closed scope
    // to prevent names conflict
    {
        using namespace your;
        belongs_to_your1 = 4;
    }

    {
        // using-directive
        // Allow usage of all namespace
        using namespace my;


        // using-declaration
        // Allow usage of a single namespace member
        // Prefer using-declaration whenever possible
        // it has priority towards using-directives
        using your1::belongs_to_your1;
        belongs_to_your1 = 5;	// your1::a
    }

    my_namespace_alias::belongs_to_my = 6;

    // Overload works through all overloaded declarations
    // Declaring operator+(), we allow all overloaded operator+()

    // Example of ambiguity with the class name
    {
        using namespace std;
        // local names hides using-directive opened names
        int vector = 10; // hides std::vector
    }

    {
        using std::vector;
        // local names overloads with using-declaration opened names
        // int vector = 10; compiler error (haha)
    }
}

/*
 *Inline namespace
 */
namespace cpp
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

// The inline specifies that V2_0 is the default meaning of cpp
inline namespace V2_0
{
void f()
{
    std::cout << "API version 2.0\n";
}
}

} // namespace cpp

void show_inline_namespaces()
{
    cpp::V1_0::f();
    cpp::V1_5::f();
    cpp::V2_0::f();

    // inline namespace works
    cpp::f();
}

int main()
{
    // TODO: insert any function call
    return 0;
}
