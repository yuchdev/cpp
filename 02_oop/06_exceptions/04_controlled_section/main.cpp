#include "controlled.h"

#include <cassert>
#include <iostream>
#include <string>

using namespace std;

// The default-constructed case: the constructor's function-try-block
// handler prints the exception once, then implicitly rethrows it -- so
// show_controlled() sees it a SECOND time, in its own outer catch.
void show_controlled_constructor_throws()
{
    bool caught_outer = false;
    try {
        controlled c; // should_fail defaults to true
        (void)c;
        assert(false); // unreachable: construction always throws here
    }
    catch (const std::exception& e) {
        // exception from the constructor's function-try-block is
        // rethrown here, and caught a second time
        caught_outer = true;
        cout << e.what() << endl;
        assert(std::string(e.what()) == "constructor exception");
    }
    catch (...) {
        cout << "Unknown exception" << endl;
    }
    assert(caught_outer);
}

// Passing should_fail = false means the constructor's try-block never
// throws at all, so its catch handlers never run, and the object
// completes construction normally -- this is the only way to ever reach
// controlled_method() at all; the default constructor cannot.
void show_controlled_constructor_succeeds()
{
    controlled c(false);
    (void)c; // construction completed; nothing was thrown, nothing printed
}

// controlled_method()'s function-try-block has NO implicit-rethrow rule
// (unlike the constructor's): the exception it catches internally does
// NOT propagate to this function's own try/catch at all.
void show_controlled_method_contains_its_own_exception()
{
    controlled c(false); // must succeed first to have anything to call this on

    bool outer_catch_ran = false;
    try {
        c.controlled_method(); // should_fail defaults to true
    }
    catch (...) {
        outer_catch_ran = true;
    }
    assert(!outer_catch_ran); // the method's own handler fully absorbed it
}

void show_controlled_method_succeeds()
{
    controlled c(false);
    c.controlled_method(false); // no throw inside, nothing to catch, nothing printed
}

int main()
{
    show_controlled_constructor_throws();
    show_controlled_constructor_succeeds();
    show_controlled_method_contains_its_own_exception();
    show_controlled_method_succeeds();
    return 0;
}
