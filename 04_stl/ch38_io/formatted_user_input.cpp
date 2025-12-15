#include "formatted_user_input.h"

using namespace std;
using namespace cpp;

formatted_date::format_flags formatted_date::format = 0x00;

const char* formatted_date::months[] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };

// Manipulators definition
// ostream friend manipulators
ostream& cpp::point(ostream& os)
{
    formatted_date::point();
    return os;
}

ostream& cpp::intgr(ostream& os)
{
    formatted_date::intgr();
    return os;
}

ostream& cpp::stringed(ostream& os)
{
    formatted_date::stringed();
    return os;
}

// istream friend manipulators
istream& cpp::point(istream& os)
{
    formatted_date::point();
    return os;
}

istream& cpp::intgr(istream& os)
{
    formatted_date::intgr();
    return os;
}

istream& cpp::stringed(istream& os)
{
    formatted_date::stringed();
    return os;
}


void cpp::show_formatted_overloads()
{

    formatted_date d1;
    formatted_date d2(19820521);
    formatted_date d3(1982, 5, 21);
    formatted_date d4(1982, "MAY", 21);
    formatted_date d5(d3);

    // point by default
    // 21.05.1982
    cout << d1 << endl;
    cout << d2 << endl;
    cout << d3 << endl;
    cout << d4 << endl;
    cout << d5 << endl;

    // 19820521
    cout << intgr << d1 << endl;
    cout << intgr << d2 << endl;
    cout << intgr << d3 << endl;
    cout << intgr << d4 << endl;
    cout << intgr << d5 << endl;

    // 21-MAY-1982
    cout << stringed << d1 << endl;
    cout << stringed << d2 << endl;
    cout << stringed << d3 << endl;
    cout << stringed << d4 << endl;
    cout << stringed << d5 << endl;

    cin >> point >> d1;
    cout << point << d1 << endl;
    cout << intgr << d1 << endl;
    cout << stringed << d1 << endl;

    cin >> intgr >> d1;
    cout << point << d1 << endl;
    cout << intgr << d1 << endl;
    cout << stringed << d1 << endl;

    cin >> stringed >> d1;
    cout << point << d1 << endl;
    cout << intgr << d1 << endl;
    cout << stringed << d1 << endl;
}

