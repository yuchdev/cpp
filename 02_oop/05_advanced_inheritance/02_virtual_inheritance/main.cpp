#include "diamond.h"

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

    // Counts lines that EXACTLY equal `line` (so "base_v()" never
    // accidentally matches inside "~base_v()", and two adjacent
    // identical lines are both counted -- a plain substring search
    // consumes the newline between them on its first match, which
    // silently undercounts consecutive occurrences).
    std::size_t count_exact_lines(const std::string& text, const std::string& line)
    {
        std::istringstream in(text);
        std::string current;
        std::size_t count = 0;
        while (std::getline(in, current))
            if (current == line)
                ++count;
        return count;
    }
}

// bottom_n inherits base_v twice (through left_n and through right_n),
// non-virtually both times: it ends up with two separate base_v
// sub-objects. An unqualified bn.identify() does not know which one you
// mean and does not compile; you must qualify the path explicitly.
void show_diamond_ambiguity()
{
    // Two independent base_v sub-objects means base_v's constructor runs
    // TWICE for one bottom_n -- proof that goes beyond sizeof() alone.
    assert(count_exact_lines(captured_output([] { bottom_n bn; }), "base_v()") == 2);

    bottom_n bn;

#if 0
    // Ambiguous: two base_v sub-objects, two identify()s. Does not compile.
    bn.identify();
#endif

    assert(captured_output([&] { bn.left_n::identify(); }) == "base_v::identify\n");
    assert(captured_output([&] { bn.right_n::identify(); }) == "base_v::identify\n");

    std::cout << "sizeof(bottom_n) = " << sizeof(bottom_n)
              << " (two base_v sub-objects)\n";
    assert(sizeof(bottom_n) == 2 * sizeof(base_v));
}

// left_v and right_v both inherit base_v virtually, so bottom_v shares a
// single base_v sub-object between them. bottom_v's own constructor is
// responsible for initializing that shared base_v directly -- the
// language requires the most-derived class to do it, bypassing left_v and
// right_v entirely, which is why bottom_v's constructor mentions base_v()
// explicitly even though it is not bottom_v's direct base.
void show_virtual_inheritance_fix()
{
    // Exactly one shared base_v: its constructor runs only ONCE, unlike
    // bottom_n's two runs above -- the sharing is real, not just a
    // sizeof() coincidence.
    assert(count_exact_lines(captured_output([] { bottom_v bv; }), "base_v()") == 1);

    bottom_v bv;
    assert(captured_output([&] { bv.identify(); }) == "base_v::identify\n"); // unambiguous: exactly one base_v exists

    std::cout << "sizeof(bottom_v) = " << sizeof(bottom_v)
              << " (one shared base_v sub-object, plus bookkeeping)\n";
}

// A self-contained progression showing exactly what virtual inheritance
// costs in object layout, independent of the diamond.h hierarchy above:
// each step adds one more thing (a data member, a second virtual branch,
// a virtual destructor) and shows how sizeof() responds. Every number
// printed below is also asserted -- this machine's actual, measured
// sizes, not numbers assumed to generalize to every ABI.
void show_sizes()
{
    {
        // 4 empty classes linked by diamond inheritance; every one is
        // still just 1 byte (the minimum for a distinct object address).
        class A {};
        class B1 : public A {};
        class B2 : public A {};
        class C : public B1, public B2 {};

        std::cout << "empty diamond: sizeof(A)=" << sizeof(A)
                  << " sizeof(B1)=" << sizeof(B1)
                  << " sizeof(B2)=" << sizeof(B2)
                  << " sizeof(C)=" << sizeof(C) << '\n';
        assert(sizeof(A) == 1 && sizeof(B1) == 1 && sizeof(B2) == 1 && sizeof(C) == 2);
    }

    {
        // A adds a 2-byte field. Non-virtual diamond: C pays for A twice.
        class A { short a; };
        class B1 : public A {};
        class B2 : public A {};
        class C : public B1, public B2 {};

        std::cout << "non-virtual diamond: sizeof(A)=" << sizeof(A)
                  << " sizeof(B1)=" << sizeof(B1)
                  << " sizeof(B2)=" << sizeof(B2)
                  << " sizeof(C)=" << sizeof(C) << " (two copies of A)\n";
        assert(sizeof(A) == 2 && sizeof(C) == 2 * sizeof(A));
    }

    {
        // Make one branch virtual: that branch grows a pointer/offset to
        // reach the shared base, but C still carries two copies of A,
        // because the OTHER branch (B2) still inherits non-virtually.
        class A { short a; };
        class B1 : virtual public A {};
        class B2 : public A {};
        class C : public B1, public B2 {};

        std::cout << "one virtual branch: sizeof(B1)=" << sizeof(B1)
                  << " sizeof(B2)=" << sizeof(B2)
                  << " sizeof(C)=" << sizeof(C) << '\n';
        assert(sizeof(B1) > sizeof(B2)); // B1 pays virtual-base bookkeeping; B2 doesn't
    }

    {
        // Make BOTH branches virtual: now C shares exactly one A, at the
        // cost of a virtual-base pointer in each of B1 and B2.
        class A { short a; };
        class B1 : virtual public A {};
        class B2 : virtual public A {};
        class C : public B1, public B2 {};

        std::cout << "both branches virtual: sizeof(B1)=" << sizeof(B1)
                  << " sizeof(B2)=" << sizeof(B2)
                  << " sizeof(C)=" << sizeof(C) << " (one shared A)\n";
        assert(sizeof(B1) == sizeof(B2)); // symmetric now: both pay the same bookkeeping
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

        std::cout << "virtual dtor added: sizeof(A)=" << sizeof(A)
                  << " sizeof(C)=" << sizeof(C) << '\n';
        assert(sizeof(A) > sizeof(short)); // the vptr grew A past its one data member
    }
}

int main()
{
    show_diamond_ambiguity();
    show_virtual_inheritance_fix();
    show_sizes();
    return 0;
}
