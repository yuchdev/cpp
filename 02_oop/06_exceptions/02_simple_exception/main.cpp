#include "triangle.h"

#include <iostream>

using std::cout;
using std::endl;

void show_simple_ex()
{
    triangle* pt1 = 0;
    try {
        // Everything is nice here:
        // triangle t1(4, 5, 6);

        // Throwing during construction means the destructor never runs
        // for the object that failed to come into existence -- there is
        // no partially-constructed triangle to clean up, because an
        // object's lifetime begins only once its constructor completes:
        // triangle t2(1, 2, 3); // 1+2 == 3: degenerate, not a real triangle

        // In case of an exception here, no memory is leaked: `new` either
        // fully succeeds (returns a valid pointer) or the exception
        // propagates before pt1 is ever assigned.
        pt1 = new triangle(4, 5, 6);

        // throw_test() throws triangle::unspecified_defect, a type that
        // is private to triangle and not named by any catch clause below
        // -- it is caught only through its public base, triangle_exception.
        // Because it throws unconditionally, the rest of this try block
        // (test_nothrow()/test_throw_some1()/test_throw_some2() below)
        // never runs in this particular pass; comment this call out to
        // see the other three exercised instead.
        pt1->throw_test();

        pt1->test_nothrow();
        pt1->test_throw_some1();
        pt1->test_throw_some2();
    }
    // First, catch the derived classes individually...
    catch (triangle::wrong_param& w) {
        cout << w.what() << endl;
        cout << w.get_wrong_param() << endl;
    }
    catch (triangle::non_triangle_exception& w) {
        cout << w.what() << endl;
        cout << w.wrong_size() << endl;
    }
    // ...then the base class; by polymorphism, this also matches any
    // triangle_exception-derived type not explicitly named above, such as
    // the private unspecified_defect thrown by throw_test().
    catch (triangle::triangle_exception& w) {
        cout << w.what() << endl;
    }

    // Release the memory in any case.
    delete pt1;
}

int main()
{
    show_simple_ex();
    return 0;
}
