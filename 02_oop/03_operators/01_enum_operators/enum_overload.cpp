#include <cassert>

// Enumerations are user-defined types too: the built-in arithmetic
// operators do not automatically exist for them. This overload has to be
// a non-member -- you cannot add member functions to an enum -- and
// returns Wday& (not Wday) to match the convention for prefix increment,
// so the result can itself be used as an lvalue (e.g. ++(++d)).
enum Wday { mon = 1, tus, wed, thr, frd, sat, sun };

Wday& operator++(Wday& d)
{
    // enum arithmetic promotes to int; there is no implicit int -> Wday
    // conversion, so the cast back to Wday is required even here, inside
    // the enum's own increment operator.
    return d = (d == sun) ? mon : Wday(d + 1);
}

int main()
{
    Wday d = sun;

    ++d;
    assert(d == mon); // the wrap-around case: sun + 1 wraps to mon, not an out-of-range value

    ++d;
    assert(d == tus); // the ordinary case: no wrap-around needed

    // Chained use works specifically because operator++ returns a
    // reference to the (already mutated) object, not a value.
    assert(&(++d) == &d);

    return 0;
}
