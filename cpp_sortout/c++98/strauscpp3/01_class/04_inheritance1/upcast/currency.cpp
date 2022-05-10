#include "currency.h"

#include <iostream>

currency_t::currency_t() : _currency() {}
currency_t::currency_t(double d) : _currency(d) {}
currency_t::currency_t(const currency_t& c) : _currency(c._currency) {}
currency_t::~currency_t() {}

currency_t& currency_t::operator =(const currency_t& c)
{
    if (this != &c) {
        _currency = c._currency;
    }
    return *this;
}

currency_t& currency_t::operator+=(const currency_t& c)
{
    _currency += c._currency;
    return *this;
}

currency_t& currency_t::operator -=(const currency_t& c)
{
    _currency -= c._currency;
    return *this;
}

currency_t& currency_t::operator *=(const currency_t& c)
{
    _currency *= c._currency;
    return *this;
}

currency_t& currency_t::operator /=(const currency_t& c)
{
    _currency /= c._currency;
    return *this;
}

void currency_t::print()
{
    std::cout << _currency << " dollars" << std::endl;
}

const currency_t operator+(const currency_t& a, const currency_t& b)
{
    currency_t tmp(a);
    return tmp += b;
}

const currency_t operator-(const currency_t& a, const currency_t& b)
{
    currency_t tmp(a);
    return tmp -= b;
}

const currency_t operator*(const currency_t& a, const currency_t& b)
{
    currency_t tmp(a);
    return tmp *= b;
}

const currency_t operator/(const currency_t& a, const currency_t& b)
{
    currency_t tmp(a);
    return tmp /= b;
}

// constructors are not inherited, define them explicitly
roubles_t::roubles_t(double d) : currency_t(d) {}

// explicitly define the upcasting constructor
// In real production practice, this considered a sign of a poor design
roubles_t::roubles_t(const currency_t& c) : currency_t(c) {}

// overridden method
void roubles_t::print()
{
    std::cout << _currency << " rubles" << std::endl;
}

roubles_t& roubles_t::operator=(const currency_t& c)
{
    // explicitly call the parent operator in functional form
    currency_t::operator =(c);

    // That is, we have explicitly converted a base class into a derived
    // Such transformations are called _downcasting_
    return *this;
}

// This is just an example on how to virtualize operators
// It's not necessary to make them virtual in this case
currency_t& roubles_t::operator*=(const currency_t& c)
{
    currency_t::operator *=(c);
    return *this;
}

currency_t& roubles_t::operator/=(const currency_t& c)
{
    currency_t::operator /=(c);
    return *this;
}
