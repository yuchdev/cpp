#include "final.h"
#include "member_pointer.h"
#include "multimethod.h"

#include <iostream>

using std::cout;
using std::endl;

// finalizer itself constructs fine: it is a friend of finalizer_lock, so
// it is allowed to initialize that private virtual base directly.
void show_final()
{
    finalizer f;
    f.do_work();

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

    da.multiop(da);
    da.multiop(db);
    db.multiop(da);
    db.multiop(db);

    // Even when both operands are passed through base&, dispatch still
    // finds the right pair -- that is the entire point of the pattern.
    base& ba = da;
    base& bb = db;
    db.multiop(ba);
    db.multiop(bb);
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
    md.start();

    // ...and called indirectly, through the member pointer, via the base
    // interface -- both reach member_derived::start()/stop() because
    // start()/stop() are virtual.
    member_pointer* base_ptr = &md;
    (base_ptr->*start)();
    (md.*stop)();

    static_member_pointer init = &member_derived::static_init;
    init();
}

int main()
{
    show_final();
    show_multimethod();
    show_member_pointer();
    return 0;
}
