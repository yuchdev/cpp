#include <stdexcept>
#include "complex.h"

complex_t::complex_t(double r/* = 0.*/, double i /*= 0.*/) : re(r), im(i) {}
complex_t::complex_t(int r) : re(r), im(0) {}

// Functions explicitly declared as inline could be implemented in *.cpp
double complex_t::real() const { return re; }
double complex_t::imag() const { return im; }


complex_t& complex_t::operator+=(const complex_t& a)
{
    re += a.re; im += a.im;
    return *this;
}

complex_t& complex_t::operator-=(const complex_t& a)
{
    re -= a.re; im -= a.im;
    return *this;
}

complex_t& complex_t::operator*=(const complex_t& a)
{
    re += (re * a.re) - (im * a.im);
    im += (re * a.im) + (im * a.re);
    return *this;
}

complex_t& complex_t::operator/=(const complex_t& a)
{
    if (a == 0)
        throw std::runtime_error("null division");

    double tmp = re * re + a.re * a.re;
    re = ((re * a.re) + (im * a.im)) / tmp;
    im = ((a.re * im) - (re * a.im)) / tmp;
    return *this;
}

bool complex_t::operator==(const complex_t& a) const
{
    return ((re == a.re) && (im == a.im));
}

bool complex_t::operator!=(const complex_t& a) const
{
    return !(*this == a);
}

complex_t& complex_t::operator++()
{
    re += 1.;
    im += 1.;
    return *this;
}

complex_t& complex_t::operator--()
{
    re -= 1.;
    im -= 1.;
    return *this;
}

complex_t complex_t::operator++(int)
{
    // postfix operation, which means returning by value
    // save previous value first, increment later
    complex_t tmp = *this;
    re += 1.;
    im += 1.;
    return tmp;
}

complex_t complex_t::operator--(int)
{
    complex_t tmp = *this;
    re -= 1.;
    im -= 1.;
    return tmp;
}

complex_t::operator int()
{
    return static_cast<int>(re);
}

complex_t::operator double()
{
    return re;
}
