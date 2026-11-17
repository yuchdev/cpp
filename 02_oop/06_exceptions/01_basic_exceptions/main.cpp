#include "ex.h"

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

void show_base_type()
{
    try {
        // show_base_type_ex(1);
        show_base_type_ex(0);
    }
    catch (const int e) {
        cout << "integer " << e << endl;
    }
    catch (const double e) {
        cout << "double " << e << endl;
    }
    // This handler would catch any exception thrown by pointer -- but
    // there is not much useful to do with it, since p's pointee type is
    // unknown and cannot be inspected without undefined behavior.
    catch (const void* p) {
        (void)p;
    }
}

// Catching a standard exception thrown by the standard library itself.
void show_out_of_range()
{
    vector<int> vi;
    vi.push_back(1);
    vi.push_back(1);
    vi.push_back(1);

    try {
        oor_exception_ex(vi);
    }
    catch (const std::exception& e) {
        cout << e.what() << endl;
    }
}

int main()
{
    show_base_type();
    show_out_of_range();
    return 0;
}
