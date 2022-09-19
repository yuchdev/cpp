#include "currency.h"

#include <iostream>

Currency::Currency() : _currency() {}
Currency::Currency(double d) : _currency(d) {}
Currency::Currency(const Currency& c) : _currency(c._currency) {}
Currency::~Currency() {}

Currency& Currency::operator =(const Currency& c)
{
    if (this != &c) {
        _currency = c._currency;
    }
    return *this;
}

Currency& Currency::operator+=(const Currency& c)
{
    _currency += c._currency;
    return *this;
}

Currency& Currency::operator -=(const Currency& c)
{
    _currency -= c._currency;
    return *this;
}

Currency& Currency::operator *=(const Currency& c)
{
    _currency *= c._currency;
    return *this;
}

Currency& Currency::operator /=(const Currency& c)
{
    _currency /= c._currency;
    return *this;
}

void Currency::print()
{
    std::cout << _currency << " dollars" << std::endl;
}

const Currency operator+(const Currency& a, const Currency& b)
{
    Currency tmp(a);
    return tmp += b;
}

const Currency operator-(const Currency& a, const Currency& b)
{
    Currency tmp(a);
    return tmp -= b;
}

const Currency operator*(const Currency& a, const Currency& b)
{
    Currency tmp(a);
    return tmp *= b;
}

const Currency operator/(const Currency& a, const Currency& b)
{
    Currency tmp(a);
    return tmp /= b;
}

// constructors are not inherited, define them explicitly
Usd::Usd(double d) : Currency(d) {}

// explicitly define the upcasting constructor
// In real production practice, this considered a sign of a poor design
Usd::Usd(const Currency& c) : Currency(c) {}

// overridden method
void Usd::print()
{
    std::cout << _currency << " rubles" << std::endl;
}

Usd& Usd::operator=(const Currency& c)
{
    // explicitly call the parent operator in functional form
    Currency::operator =(c);

    // That is, we have explicitly converted a base class into a derived
    // Such transformations are called _downcasting_
    return *this;
}

// This is just an example on how to virtualize operators
// It's not necessary to make them virtual in this case
Currency& Usd::operator*=(const Currency& c)
{
    Currency::operator *=(c);
    return *this;
}

Currency& Usd::operator/=(const Currency& c)
{
    Currency::operator /=(c);
    return *this;
}
