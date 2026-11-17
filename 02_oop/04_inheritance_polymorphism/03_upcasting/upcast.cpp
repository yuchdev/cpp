#include "currency.h"

#include <iostream>

// Upcasting: a Usd is-a Currency, so a Usd converts implicitly to a
// Currency reference. print() is NOT declared virtual in Currency, so
// calling it through a Currency& always runs Currency::print(), never
// Usd::print() -- without `virtual`, there is no runtime polymorphism,
// only the illusion of it from the matching method name.
void show_upcasting()
{
    Usd u(42.0);

    Currency& as_base = u;
    as_base.print(); // Currency::print(), NOT Usd::print()
    u.print();        // Usd::print(), called through its own type
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
    converted.print();

    Usd u(1.0);
    u = c; // Usd::operator=(const Currency&)
    u.print();
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
    sum.print();

    Currency diff = a - b;
    diff.print();
}

int main()
{
    show_upcasting();
    show_downcasting();
    show_friend_operators();
    return 0;
}
