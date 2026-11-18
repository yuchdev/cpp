#include "final.h"
#include "member_pointer.h"
#include "multimethod.h"

#include <cassert>
#include <iostream>
#include <sstream>

namespace
{
    template <typename F>
    std::string captured_output(F&& f)
    {
        std::ostringstream out;
        std::streambuf* old = std::cout.rdbuf(out.rdbuf());
        f();
        std::cout.rdbuf(old);
        return out.str();
    }
}

// finalizer itself constructs fine: it is a friend of finalizer_lock, so
// it is allowed to initialize that private virtual base directly.
void show_final()
{
    finalizer f;
    assert(captured_output([&] { f.do_work(); }) == "finalizer::do_work\n");

#if 0
    // Deriving from finalizer does not compile. `more`'s implicit default
    // constructor would need to call finalizer_lock's constructor
    // directly (finalizer_lock is a virtual base, so the most-derived
    // class -- here, `more` -- is always the one responsible for
    // constructing it), but finalizer_lock's constructor is private and
    // friended only to finalizer, not to `more`.
    class more : public finalizer
    {
        int i;
    };
    more m; // error: finalizer_lock::finalizer_lock() is private
#endif
}

// Double dispatch picks one of 4 bodies for (derived_a, derived_b) x
// (derived_a, derived_b) purely from the two operands' dynamic types --
// no switch/typeid/dynamic_cast involved anywhere in this function.
void show_multimethod()
{
    derived_a da;
    derived_b db;

    assert(captured_output([&] { da.multiop(da); }) == "derived_a::multiop(derived_a&)\n");
    assert(captured_output([&] { da.multiop(db); }) == "derived_a::multiop(derived_b&)\n");
    assert(captured_output([&] { db.multiop(da); }) == "derived_b::multiop(derived_a&)\n");
    assert(captured_output([&] { db.multiop(db); }) == "derived_b::multiop(derived_b&)\n");

    // Even when both operands are passed through base&, dispatch still
    // finds the right pair -- that is the entire point of the pattern.
    base& ba = da;
    base& bb = db;
    assert(captured_output([&] { db.multiop(ba); }) == "derived_a::multiop(derived_b&)\n");
    assert(captured_output([&] { db.multiop(bb); }) == "derived_b::multiop(derived_b&)\n");
}

// A pointer to a non-static member function is not a regular pointer and
// cannot be converted to or from one: it must carry enough information
// (an offset into the vtable, for a virtual function) to work correctly
// no matter which derived object it is eventually invoked through.
void show_member_pointer()
{
    member_derived md;

    p_mem start = &member_pointer::start;
    p_mem stop = &member_pointer::stop;

    // Called on the derived object directly...
    assert(captured_output([&] { md.start(); }) == "member_derived::start\n");

    // ...and called indirectly, through the member pointer, via the base
    // interface -- both reach member_derived::start()/stop() because
    // start()/stop() are virtual.
    member_pointer* base_ptr = &md;
    assert(captured_output([&] { (base_ptr->*start)(); }) == "member_derived::start\n");
    assert(captured_output([&] { (md.*stop)(); }) == "member_derived::stop\n");

    static_member_pointer init = &member_derived::static_init;
    init(); // a plain function pointer call: no object, no .*/->*  needed
}

int main()
{
    show_final();
    show_multimethod();
    show_member_pointer();
    return 0;
}
