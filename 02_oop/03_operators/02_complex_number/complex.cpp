#include <stdexcept>
#include "complex.h"

complex_t::complex_t(double r/* = 0.*/, double i /*= 0.*/) : re(r), im(i) {}
complex_t::complex_t(int r) : re(r), im(0) {}

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
    // (re + im*i) * (a.re + a.im*i) = (re*a.re - im*a.im) + (re*a.im + im*a.re)*i
    // both new parts need the ORIGINAL re/im, so compute them before assigning either
    double new_re = re * a.re - im * a.im;
    double new_im = re * a.im + im * a.re;
    re = new_re;
    im = new_im;
    return *this;
}

complex_t& complex_t::operator/=(const complex_t& a)
{
    if (a == 0)
        throw std::runtime_error("null division");

    // divide by multiplying with a's conjugate over |a|^2
    double denom = a.re * a.re + a.im * a.im;
    double new_re = (re * a.re + im * a.im) / denom;
    double new_im = (im * a.re - re * a.im) / denom;
    re = new_re;
    im = new_im;
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
