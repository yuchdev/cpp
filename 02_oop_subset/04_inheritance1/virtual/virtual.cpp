#include "virtual.h"


// Execute function f() on class creation and destruction
A_virt::A_virt()
{
    cout << "A_virt(): ";
    f();
}

A_virt::~A_virt()
{
    cout << "~A_virt(): ";
    f();
}

ostream& A_virt::f() const
{
    return (cout << "A::f" << endl);
}

// The same for B_virt class
B_virt::B_virt()
{
    cout << "B_virt(): ";
    f();
}

B_virt::~B_virt()
{
    cout << "~B_virt(): ";
    f();
}

ostream& B_virt::f() const
{
    return (cout << "B::f" << endl);
}

v_base::v_base()
{
    cout << "v_base::v_base()" << endl;
}

// a pure virtual destructor can have an implementation
v_base::~v_base()/* = 0*/
{
    cout << "v_base::~v_base() = 0" << endl;
}

void v_base::pure()
{
    cout << "v_base::pure()" << endl;
}

// ---------------  --------------- 
v_derived::v_derived()
{
    cout << "v_derived::v_derived()" << endl;
}
