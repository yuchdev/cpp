#include "diamond.h"

#include <iostream>

using std::cout;
using std::endl;

// bottom_n inherits base_v twice (through left_n and through right_n),
// non-virtually both times: it ends up with two separate base_v
// sub-objects. An unqualified bn.identify() does not know which one you
// mean and does not compile; you must qualify the path explicitly.
void show_diamond_ambiguity()
{
    bottom_n bn;

#if 0
    // Ambiguous: two base_v sub-objects, two identify()s. Does not compile.
    bn.identify();
#endif

    bn.left_n::identify();
    bn.right_n::identify();

    cout << "sizeof(bottom_n) = " << sizeof(bottom_n)
         << " (two base_v sub-objects)\n";
}

// left_v and right_v both inherit base_v virtually, so bottom_v shares a
// single base_v sub-object between them. bottom_v's own constructor is
// responsible for initializing that shared base_v directly -- the
// language requires the most-derived class to do it, bypassing left_v and
// right_v entirely, which is why bottom_v's constructor mentions base_v()
// explicitly even though it is not bottom_v's direct base.
void show_virtual_inheritance_fix()
{
    bottom_v bv;
    bv.identify(); // unambiguous: exactly one base_v exists

    cout << "sizeof(bottom_v) = " << sizeof(bottom_v)
         << " (one shared base_v sub-object, plus bookkeeping)\n";
}

// A self-contained progression showing exactly what virtual inheritance
// costs in object layout, independent of the diamond.h hierarchy above:
// each step adds one more thing (a data member, a second virtual branch,
// a virtual destructor) and shows how sizeof() responds.
void show_sizes()
{
    {
        // 4 empty classes linked by diamond inheritance; every one is
        // still just 1 byte (the minimum for a distinct object address).
        class A {};
        class B1 : public A {};
        class B2 : public A {};
        class C : public B1, public B2 {};

        cout << "empty diamond: sizeof(A)=" << sizeof(A)
             << " sizeof(B1)=" << sizeof(B1)
             << " sizeof(B2)=" << sizeof(B2)
             << " sizeof(C)=" << sizeof(C) << '\n';
    }

    {
        // A adds a 2-byte field. Non-virtual diamond: C pays for A twice.
        class A { short a; };
        class B1 : public A {};
        class B2 : public A {};
        class C : public B1, public B2 {};

        cout << "non-virtual diamond: sizeof(A)=" << sizeof(A)
             << " sizeof(B1)=" << sizeof(B1)
             << " sizeof(B2)=" << sizeof(B2)
             << " sizeof(C)=" << sizeof(C) << " (two copies of A)\n";
    }

    {
        // Make one branch virtual: that branch grows a pointer/offset to
        // reach the shared base, but C still carries two copies of A,
        // because the OTHER branch (B2) still inherits non-virtually.
        class A { short a; };
        class B1 : virtual public A {};
        class B2 : public A {};
        class C : public B1, public B2 {};

        cout << "one virtual branch: sizeof(B1)=" << sizeof(B1)
             << " sizeof(B2)=" << sizeof(B2)
             << " sizeof(C)=" << sizeof(C) << '\n';
    }

    {
        // Make BOTH branches virtual: now C shares exactly one A, at the
        // cost of a virtual-base pointer in each of B1 and B2.
        class A { short a; };
        class B1 : virtual public A {};
        class B2 : virtual public A {};
        class C : public B1, public B2 {};

        cout << "both branches virtual: sizeof(B1)=" << sizeof(B1)
             << " sizeof(B2)=" << sizeof(B2)
             << " sizeof(C)=" << sizeof(C) << " (one shared A)\n";
    }

    {
        // Giving the shared base a virtual destructor adds a vtable
        // pointer on top of the virtual-base bookkeeping already paid for.
        class A
        {
        public:
            short a;
            virtual ~A() {}
        };
        class B1 : virtual public A {};
        class B2 : virtual public A {};
        class C : public B1, public B2 {};

        cout << "virtual dtor added: sizeof(A)=" << sizeof(A)
             << " sizeof(C)=" << sizeof(C) << '\n';
    }
}

int main()
{
    show_diamond_ambiguity();
    show_virtual_inheritance_fix();
    show_sizes();
    return 0;
}
