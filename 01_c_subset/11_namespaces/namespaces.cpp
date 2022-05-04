#include <vector>

// Unnamed and named exceptions, aliases, merge namespaces
// * standard usecases of namespaces:
//   * structure the code base
//   * prevent naming conflicts
//   * wrap plain C code 


// Simple names namespace
namespace my
{
    int a;
}

// When we use same namespace specifier, namespaces are being merged
// Such namespaces could be declared anywhere, in different translation units, in different libraries
namespace my
{
    int aa;
}

// Unnamed namespace
// Could be used for resolvind name conflicts, and preventing access from other translation units
namespace
{
    int anonimous;
}


// One more way to merge two namespaces inside third namespace
namespace your1
{
    int a;
}

namespace your2
{
    int b;
}

namespace your
{
    using namespace your1;
    using namespace your2;
}

// Namespace alias
// Could be convenient while using namespaces vor versioning
namespace aaa = my;


// It is recommended to bring interface of C library into distinct namespace
/*
namespace CLib
{
int printf();
}
*/

// While namespace member is not being used, it can't cause name conflict
// Example:
namespace A
{
    int aaa;
}

namespace B
{
    int aaa;
}

namespace AB
{
    using namespace A;
    using namespace B;
}


void show_namespaces()
{
    my::a = 1;
    your1::a = 2;

    // Merged namespaces your1 and your2
    your::a = 3;

    // using-directive could be utilized in a closed scope
    // to prevent names conflict
    {
        using namespace your;
        a = 4;
    }

    {
        // using-directive
        // Allow usage of all namespace
        using namespace my;


        // using-declaration
        // Allow usage of a single namespace member
        // Prefer using-declaration whinever possible
        // it has priority towards using-directives
        using your1::a;
        a = 5;	// your1::a
    }

    aaa::a = 6;

    // Overload works through all overloaded declarations
    // Declaring operator+(), we allow all overloaded operator+()

    // Example of ambuguosity with the class name
    {
        using namespace std;
        // local names hides using-directive opened names
        int vector = 10; // hides std::vector
    }

    {
        using std::vector;
        // local names overloads with using-declaration opened names
        // int vector = 10; compiler error (hahaha)
    }
}

int main()
{
    // TODO: insert any function call
    return 0;
}
