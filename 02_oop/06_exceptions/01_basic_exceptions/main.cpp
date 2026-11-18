#include "ex.h"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <vector>

using std::cout;
using std::endl;
using std::vector;

// Throwing exceptions of a built-in type
void show_base_type_ex(int b)
{
    // When throwing exceptions of a built-in type, no implicit conversion
    // takes place (double -> int, unsigned -> signed, etc.) -- the
    // handler must match the thrown type exactly (or via a pointer/
    // reference conversion), the same as any other exception type.
    if (b)
        throw int(1);
    else
        throw double(0.);
    // It is better not to throw exceptions of a built-in type, and not to
    // allocate an exception's storage on the heap, e.g. `throw new int(1)`.
}

// Exercises BOTH throw paths in separate try blocks (a single try can
// only ever observe whichever branch actually threw first) and asserts
// that each lands in its own, exactly-matching catch clause -- not the
// pointer catch-all, and not each other's.
void show_base_type()
{
    enum class caught { none, as_int, as_double, as_pointer };
    caught which = caught::none;

    try {
        show_base_type_ex(1);
    }
    catch (const int e) {
        which = caught::as_int;
        assert(e == 1);
    }
    catch (const double) {
        which = caught::as_double;
    }
    catch (const void*) {
        which = caught::as_pointer;
    }
    assert(which == caught::as_int);

    which = caught::none;
    try {
        show_base_type_ex(0);
    }
    catch (const int) {
        which = caught::as_int;
    }
    catch (const double e) {
        which = caught::as_double;
        assert(e == 0.0);
    }
    // This handler would catch any exception thrown by pointer -- but
    // there is not much useful to do with it, since p's pointee type is
    // unknown and cannot be inspected without undefined behavior.
    catch (const void*) {
        which = caught::as_pointer;
    }
    assert(which == caught::as_double);
}

// Catching a standard exception thrown by the standard library itself.
void show_out_of_range()
{
    vector<int> vi;
    vi.push_back(1);
    vi.push_back(1);
    vi.push_back(1);

    bool caught_out_of_range = false;
    try {
        oor_exception_ex(vi);
        assert(false); // unreachable: oor_exception_ex always indexes past the end
    }
    catch (const std::out_of_range& e) {
        caught_out_of_range = true;
        cout << e.what() << endl;
    }
    catch (const std::exception&) {
        assert(false); // vector::at() throws out_of_range specifically, not some other std::exception
    }
    assert(caught_out_of_range);
}

int main()
{
    show_base_type();
    show_out_of_range();
    return 0;
}
