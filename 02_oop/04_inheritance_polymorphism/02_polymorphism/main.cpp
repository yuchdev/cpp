#include "virtual.h"

#include <cassert>
#include <sstream>

// Base::func() is not virtual: Derived::func() never participates in the
// call, even though it uses the `virtual` keyword on its own declaration
// (virtual-ness only matters if the function it overrides was virtual too).
void show_static_vs_dynamic_binding()
{
    Derived d;
    assert(d.call_protected_function(5) == 11); // Base::func(5) = 10, +1 = 11 (never Derived::func)

    Derived_virt dv;
    assert(dv.call_protected_function(5) == 26); // Derived_virt::func(5) = 25, +1 = 26
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

    // Capture what each call actually prints, to prove the default-value
    // claim rather than just asserting it in a comment: rd.g() dispatches
    // to Derived1::g (dynamic), but with Base1's default argument (10),
    // not Derived1's own default (20).
    {
        std::ostringstream out;
        auto* old = std::cout.rdbuf(out.rdbuf());
        rb.g();
        rd.g();
        std::cout.rdbuf(old);
        assert(out.str() == "B::g(int) 10\nD::g(int) 10\n");
    }

    A_virt* p = rd.pf();
    delete p;

    rb.hide_me();
    rd.hide_me();
}

// `final` on Derived1::hide_me() forbids any FURTHER override, while every
// other virtual function Derived1 declares remains overridable --
// MostDerived1::open_me() proves the rest of the hierarchy still works
// normally. FinalDerived1 shows the whole-class form of final; the two
// #if 0 blocks in virtual.h are the actual compile errors final produces,
// kept disabled per this repo's convention for code that must not compile.
void show_final_restricts_only_what_it_marks()
{
    MostDerived1 md;

    md.open_me(); // MostDerived1's own override -- public here, final elsewhere didn't block this
    Base1& r = md;
    r.hide_me(); // still runs Derived1::hide_me() -- MostDerived1 never got to override it
                 // (hide_me is public in Base1, so this call is accessible through Base1&)

    FinalDerived1 fd;
    (void)fd; // constructs fine: final forbids further DERIVATION, not instantiation
}

int main()
{
    show_static_vs_dynamic_binding();
    show_virtual_call_in_ctor_dtor();
    show_pure_virtual_destructor_with_body();
    show_defaults_and_covariance_and_access();
    show_final_restricts_only_what_it_marks();
    return 0;
}
