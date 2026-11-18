#include "triangle.h"

#include <cassert>
#include <iostream>
#include <string>

using std::cout;
using std::endl;

// A valid triangle constructs without throwing at all.
void show_valid_construction()
{
    triangle t(4, 5, 6);
    (void)t;
}

// Throwing during construction means the destructor never runs for the
// object that failed to come into existence -- there is no
// partially-constructed triangle to clean up, because an object's
// lifetime begins only once its constructor completes.
void show_degenerate_triangle_throws()
{
    bool caught = false;
    try {
        triangle t(1, 2, 3); // 1+2 == 3: degenerate, not a real triangle
        (void)t;
        assert(false); // unreachable
    }
    catch (triangle::non_triangle_exception& w) {
        caught = true;
        cout << w.what() << endl;
        cout << w.wrong_size() << endl;
        assert(w.wrong_size() == 0); // exactly degenerate: a+b+c-2*max == 0
    }
    assert(caught);
}

void show_invalid_side_throws()
{
    bool caught = false;
    try {
        triangle t(0, 5, 6); // a side of length 0 is invalid before the inequality is even checked
        (void)t;
        assert(false);
    }
    catch (triangle::wrong_param& w) {
        caught = true;
        cout << w.what() << endl;
        cout << w.get_wrong_param() << endl;
        assert(w.get_wrong_param() == 0);
    }
    assert(caught);
}

// throw_test() throws triangle::unspecified_defect, a type that is
// private to triangle and not named by any catch clause here -- caught
// only through its public base, triangle_exception. This is exactly the
// scenario where catching by base class (not by exact type) matters.
void show_catch_by_base_catches_unnamed_derived()
{
    triangle t(4, 5, 6);

    bool caught_as_base = false;
    try {
        t.throw_test();
        assert(false);
    }
    // First, the derived classes individually...
    catch (triangle::wrong_param&) {
        assert(false); // unspecified_defect is not a wrong_param
    }
    catch (triangle::non_triangle_exception&) {
        assert(false); // nor a non_triangle_exception
    }
    // ...then the base class; by polymorphism, this also matches any
    // triangle_exception-derived type not explicitly named above.
    catch (triangle::triangle_exception& w) {
        caught_as_base = true;
        cout << w.what() << endl;
    }
    assert(caught_as_base);
}

void show_test_nothrow()
{
    triangle t(4, 5, 6);
    t.test_nothrow(); // declared noexcept; if this function's body ever
                       // threw, the program would terminate() instead of
                       // propagating -- the absence of a try/catch here
                       // IS the test.
}

void show_test_throw_methods()
{
    triangle t(4, 5, 6);

    bool caught1 = false;
    try {
        t.test_throw_some1();
        assert(false);
    }
    catch (triangle::wrong_param& w) {
        caught1 = true;
        assert(w.get_wrong_param() == -1);
    }
    assert(caught1);

    bool caught2 = false;
    try {
        t.test_throw_some2();
        assert(false);
    }
    catch (triangle::non_triangle_exception& w) {
        caught2 = true;
        // Forced unconditionally, regardless of whether (4,5,6) is
        // actually degenerate -- it isn't, so wrong_size() here is
        // POSITIVE, unlike show_degenerate_triangle_throws()'s exactly-0.
        assert(w.wrong_size() == 4 + 5 + 6 - 2 * 6);
    }
    assert(caught2);
}

int main()
{
    show_valid_construction();
    show_degenerate_triangle_throws();
    show_invalid_side_throws();
    show_catch_by_base_catches_unnamed_derived();
    show_test_nothrow();
    show_test_throw_methods();
    return 0;
}
