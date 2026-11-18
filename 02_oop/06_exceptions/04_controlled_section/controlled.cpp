#include "controlled.h"
#include <iostream>
#include <stdexcept>

using namespace std;

// Controlled constructor intercepts all exceptions from the constructor
// body and the initialization list, and passes them further.
//
// For a CONSTRUCTOR's function-try-block specifically, the standard
// requires the exception to propagate out once the handler finishes --
// either by throwing something else, or (as here) implicitly: falling
// off the end of the handler re-throws the original exception
// automatically. There is no way to "catch and swallow" a constructor's
// own exception from inside its own function-try-block; a constructor
// that throws never produced a valid object, so there is nothing a
// handler here could sensibly return instead.
controlled::controlled(bool should_fail) try : _vect(1000)
{
    for (int i = 0; i < 1000; i++)
        _vect.push_back(1);

    if (should_fail)
        throw std::runtime_error("constructor exception");
}
catch (const exception& e) {
    cout << e.what() << endl;
}
catch (...) {
    cout << "Unknown exception" << endl;
}

// Standard says, if the `return` expression occurs inside the function-try-block handler
// of the constructor, then the program is considered to be ill-formed

// Destructors should not throw exceptions
// At least they must not leave its scope
controlled::~controlled() throw() {}

// Controlled method: an ORDINARY member function's function-try-block has
// no such implicit-rethrow rule -- unlike the constructor above, this
// handler can simply finish normally and the exception does NOT
// propagate further. That asymmetry (constructor vs. ordinary method) is
// the whole point of comparing the two.
void controlled::controlled_method(bool should_fail) const try {
    if (should_fail)
        throw std::runtime_error("controlled method exception");
}
catch (std::exception& e) {
    cout << e.what() << endl;
}
