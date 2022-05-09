#include "format_stream.h"
#include <iostream>
#include <iomanip> // manipulators with args
#include <limits>

using namespace std;
using namespace cpp4;


// Flags
void cpp4::show_formattings_flags()
{

    // Get current stream flags
    ios::fmtflags flags = cout.flags();

    // Flags for the streams are set with the following methods:
    // flags()  - works as =
    // setf() - works as OR
    // unsetf() - zero all flags
    // stream.copyfmt(cout) - copy from other stream

    // adjust left:
    {
        // ios::adjustfield - for adjustment flags group
        cout.setf(ios::left, ios::adjustfield);
        cout << "left: " << 10 << endl;
        cout.unsetf(ios::left);
    }

    // adjust right:
    {
        // ios::adjustfield - for adjustment flags group
        cout.setf(ios::right, ios::adjustfield);
        cout << "right: " << 20 << endl;
        cout.unsetf(ios::right);
    }

    // sign right, number left
    {
        // ios::adjustfield - for adjustment flags group
        // should be applied just before input
        cout << "internal: ";
        cout.width(6);
        cout.setf(ios::internal, ios::adjustfield);
        cout << -2 << endl;
        cout.unsetf(ios::internal);
    }

    // hex + prefix
    {
        // ios::basefield - for numeric base flags groups
        cout << "hex|showbase: ";
        cout.setf(ios::hex, ios::basefield);
        cout.setf(ios::showbase);

        cout << 0xffff << endl;
        cout.unsetf(ios::showbase);
        cout.unsetf(ios::hex);
    }

    // fp with '.'
    {
        cout.setf(ios::fixed);
        cout << "fixed: " << 0.0000001 << endl;
        cout.unsetf(ios::fixed);
    }

    // fp with 'e'
    {
        cout.setf(ios::scientific);
        cout << "scientific: " << 0.0000001 << endl;
        cout.unsetf(ios::scientific);
    }

    // out bool as true/false
    {
        cout.setf(ios::boolalpha);
        cout << "boolalpha: " << true << endl;
        cout.unsetf(ios::boolalpha);
    }

    // out numeric base
    {
        cout.setf(ios::showbase);
        cout << "showbase: " << std::hex << 0xff << endl;
        cout.unsetf(ios::showbase);
    }

    // out trailing zeros
    {
        cout.setf(ios::showpoint);
        cout << "showpoint: " << std::dec << 100.00 << endl;
        cout.unsetf(ios::showpoint);
    }

    // always out sign
    {
        cout.setf(ios::showpos);
        cout << "showpos: " << 100 << endl;
        cout.unsetf(ios::showpos);
    }

    // E instead of e
    {
        cout.setf(ios::uppercase);
        cout << "uppercase: " << 0.000000001 << endl;
        cout.unsetf(ios::uppercase);
    }

    {
        // use hexadecimal floats
        cout.setf(ios_base::fixed | ios_base::scientific, ios_base::floatfield);
        cout << "hexadecimal floats: " << 0.000000001 << endl;
        cout.unsetf(ios_base::fixed | ios_base::scientific);
    }

    // important flag unitbuf
    // does not change formatting, but turns off buffering
    {
        cout.setf(ios::unitbuf);
        cout << "unitbuf: " << 10000 << endl;
        cout.unsetf(ios::unitbuf);
    }
    // If you have a genuine need for frequent flushing, consider cerr and unitbuf
}

// Methods
void cpp4::show_formattings_methods()
{

    // precision() method allows to set a number of digits after '.'
    // setting forever
    cout.setf(ios::fixed);
    streamsize prev_pr = cout.precision(10);
    cout << "precision(10): " << 0.000000001 << endl;
    cout.unsetf(ios::fixed);


    // width() method set the width of output field
    // setting for only one operation
    // fill() method setting filling symbol
    // setting forever
    streamsize prev_sz = cout.width(10);
    char prev_fill = cout.fill('-');
    cout << 123 << endl;
    cout.fill(prev_fill);

    // get back old precision
    cout.precision(prev_pr);
}

// Manipulators
void cpp4::show_formattings_manipulators()
{
    // Manipulators changes flags and methods as they are simplier in usage
    // and could be passed just to the stream

    // Manipulators - flags analogs

    // adjust left
    cout << "left: " << std::left << 10 << endl;

    // adjust right
    cout << "right: " << std::right << 20 << endl;

    // sign right, digit left
    int n = -77;
    cout << "internal: ";
    std::cout.width(6); std::cout << std::internal << n << '\n';

    // hex + prefix
    cout << "hex: " << std::hex << 0xffff << endl;

    // fp with '.'
    cout << "fixed: " << std::fixed << 0.0000001 << endl;

    // fp with e
    cout << "scientific: " << std::scientific << 0.0000001 << endl;
    cout << "fixed: " << std::fixed << 0.0000001 << endl;

    // bool as true/false
    cout << "boolalpha: " << std::boolalpha << true << endl;
    // ... and switch it off
    cout << "noboolalpha: " << std::noboolalpha << true << endl;

    // out number base
    cout << "showbase: " << std::showbase << 0xaa << endl;
    // ...and switch it off
    cout << "noshowbase: " << std::noshowbase << 0xaa << endl;
    cout << std::dec;

    // out trailing zeros
    cout << "showpoint: " << std::showpoint << 100.00 << endl;
    // ...and switch it off
    cout << "noshowpoint: " << std::noshowpoint << 100.00 << endl;

    // always out sign
    cout << "showpos: " << std::showpos << 100 << endl;
    // ...and switch it off
    cout << "noshowpos: " << std::noshowpos << 100 << endl;

    // out E instead of e
    cout << "uppercase: " << std::scientific << std::uppercase << 0.000000001 << endl;
    // ...and switch it off
    cout << "nouppercase: " << std::scientific << std::nouppercase << 0.000000001 << endl;

    // swich off buffering
    cout << "unitbuf: " << std::unitbuf << 10000 << endl;
    // ...and switch it on
    cout << "nounitbuf: " << std::nounitbuf << 10000 << endl;

    // Manipulators as methods

    // set precision
    cout << "setprecision: " << std::setprecision(16) << 0.000000001 << endl;
    // get back old precision
    cout << std::setprecision(6);

    // same with - setfill(), setflag()
    // same with - setiosflag(), resetiosflag()

    // set number base
    cout << "setbase 16: " << std::setbase(16) << 10 << endl;
    cout << "setbase 8: " << std::setbase(8) << 10 << endl;
    // non-standard are set to setbase(10)
    cout << "setbase 12: " << std::setbase(12) << 0.00001 << endl;

    // immediate flush stream
    cout << "Flush me" << flush << " yeah!" << endl;
}

void cpp4::show_formatted_input()
{

    // Manipulators could be used for input as well
    // E.g. skipws/noskipws

    // Also take effect basefield and boolalpha
    cout << "Enter true or false: ";
    bool b = true;
    cin >> std::boolalpha >> b;
    cin >> std::noboolalpha;
    cout << "You entered: " << b << endl;

    int i = 0;
    cout << "Enter hexidecimal number: ";
    cin >> std::setbase(16) >> i;
    cin >> std::setbase(0);
    cout << "You entered: " << i << endl;

    // setw() set input field width
    // able to deal with array
    char a[10] = {};
    cin >> std::setw(sizeof(a)) >> a;
    cout << "You entered: " << a << endl;
}


void cpp4::show_user_manip()
{
    cout << "Tab user" << cpp4::tab << " manip" << endl;

    cout << "User manipilator with params: " << cpp4::binary<unsigned short>(16) << endl;
    // effector class could be used for complicated user types output
    // dates, currencies etc.

}

std::ostream& cpp4::tab(std::ostream& os)
{
    return (os << '\t');
}

template <typename T>
ostream& cpp4::operator<<(ostream& os, const binary<T>& t)
{
    T MAX = numeric_limits<T>::max();
    T bit = ~(MAX >> 1);
    while (bit) {
        os << (t._num & bit ? '1' : '0');
        bit >>= 1;
    }
    return os;
}
