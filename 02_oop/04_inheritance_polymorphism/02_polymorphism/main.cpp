#include "virtual.h"

// Base::func() is not virtual: Derived::func() never participates in the
// call, even though it uses the `virtual` keyword on its own declaration
// (virtual-ness only matters if the function it overrides was virtual too).
void show_static_vs_dynamic_binding()
{
    Derived d;
    cout << "Base::func (static):      " << d.call_protected_function(5) << endl;

    Derived_virt dv;
    cout << "Derived_virt::func (dyn):  " << dv.call_protected_function(5) << endl;
}

// Virtual calls made from a constructor or destructor always use the type
// of the object under construction/destruction at that point in time, not
// the most-derived type -- this is why both A_virt() and B_virt() print
// their own class's f(), never the most-derived override.
void show_virtual_call_in_ctor_dtor()
{
    B_virt b;
}

// v_base's destructor is pure virtual but still has a definition: pure
// virtuality only means a derived class may not inherit it as-is, not that
// it can't do useful work. v_derived never defines its own destructor, so
// the (non-pure) inherited one runs.
void show_pure_virtual_destructor_with_body()
{
    v_derived vd;
    vd.pure();
}

// Default arguments on a virtual function are bound statically: Derived1::g
// is called with whichever default the STATIC type's declaration specifies.
// pf() demonstrates a covariant return type (B_virt* substitutes for A_virt*).
// open_me()/hide_me() show that overriding can change a virtual function's
// access level in the derived class without affecting callability through
// a base pointer.
void show_defaults_and_covariance_and_access()
{
    Base1 b;
    Derived1 d;

    Base1& rb = b;
    Base1& rd = d;

    rb.g();
    rd.g();

    A_virt* p = rd.pf();
    delete p;

    rb.hide_me();
    rd.hide_me();
}

int main()
{
    show_static_vs_dynamic_binding();
    show_virtual_call_in_ctor_dtor();
    show_pure_virtual_destructor_with_body();
    show_defaults_and_covariance_and_access();
    return 0;
}
