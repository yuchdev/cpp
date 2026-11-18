#include "currency.h"

#include <cassert>
#include <iostream>
#include <sstream>

namespace
{
    // print() writes directly to std::cout; capturing it lets these
    // demos assert on what actually ran instead of only printing it for
    // a human to eyeball.
    template <typename F>
    std::string captured_output(F&& f)
    {
        std::ostringstream out;
        std::streambuf* old = std::cout.rdbuf(out.rdbuf());
        f();
        std::cout.rdbuf(old);
        return out.str();
    }
}

// Upcasting: a Usd is-a Currency, so a Usd converts implicitly to a
// Currency reference. print() is NOT declared virtual in Currency, so
// calling it through a Currency& always runs Currency::print(), never
// Usd::print() -- without `virtual`, there is no runtime polymorphism,
// only the illusion of it from the matching method name.
void show_upcasting()
{
    Usd u(42.0);
    Currency& as_base = u;

    assert(captured_output([&] { as_base.print(); }) == "42 dollars\n"); // Currency::print(), NOT Usd::print()
    assert(captured_output([&] { u.print(); }) == "42 rubles\n");        // Usd::print(), called through its own type
}

// Downcasting: converting a base-class object into a derived one.
// Usd provides a converting constructor Usd(const Currency&) and an
// overloaded operator=(const Currency&) specifically so a Currency can be
// turned into a Usd explicitly -- the language does not do this
// automatically, and most style guides consider needing it a design smell.
void show_downcasting()
{
    Currency c(10.0);

    Usd converted(c);
    assert(captured_output([&] { converted.print(); }) == "10 rubles\n");

    Usd u(1.0);
    u = c; // Usd::operator=(const Currency&)
    assert(captured_output([&] { u.print(); }) == "10 rubles\n");
}

// operator+= is non-virtual by design: both Currency and its only override
// point (operator*=/operator/=) behave identically whether called on a
// Currency or a Usd, so reference polymorphism (calling the base's
// implementation through a derived object) is enough -- no virtual
// dispatch needed for +=/-=.
void show_friend_operators()
{
    Currency a(5.0);
    Currency b(3.0);

    Currency sum = a + b;
    assert(captured_output([&] { sum.print(); }) == "8 dollars\n");

    Currency diff = a - b;
    assert(captured_output([&] { diff.print(); }) == "2 dollars\n");
}

int main()
{
    show_upcasting();
    show_downcasting();
    show_friend_operators();
    return 0;
}
