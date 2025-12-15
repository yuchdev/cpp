#pragma once
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>


using namespace std;

namespace cpp
{

// demo of user-defined input and output
class formatted_date
{
public:

    // flag constants
    typedef long format_flags;
    static const format_flags _point = 0x00;
    static const format_flags _intgr = 0x01;
    static const format_flags _stringed = 0x02;

    // flag methods
    static format_flags get_flags() { return format; }

    static void point() { format = _point; }
    static void intgr() { format = _intgr; }
    static void stringed() { format = _stringed; }

    // construction
    formatted_date() :_date() {}

    formatted_date(unsigned long date) :_date(date) {}

    formatted_date(unsigned long year, unsigned long month, unsigned long day) :
        _date(10000 * year + 100 * month + day)
    {
    }

    formatted_date(unsigned long year, const char* month, unsigned long day) :
        _date(10000 * year + 100 * get_month(month) + day)
    {
    }

    formatted_date(const formatted_date& date) :_date(date._date) {}

    // month string to unsigned
    static unsigned long get_month(const char* m)
    {
        for (size_t i = 0; i < 12; ++i) {
            if (0 == strcmp(m, months[i])) {
                return i + 1;
            }
        }
        // month not found
        return 0xFFFFFFFF;
    }

    // input-output friend functions
    friend ostream& operator<<(ostream& os, const formatted_date& date)
    {
        ostringstream s;
        int y = date._date / 10000;
        int m = (date._date / 100) % 100;
        int d = date._date % 100;
        int month_str = (m == 0) ? m : m - 1;

        if (formatted_date::get_flags() == _point) {
            s << setfill('0') <<
                setw(2) << d << '.'
                << setw(2) << m << '.'
                << setw(4) << y;
        }
        else if (formatted_date::get_flags() == _intgr) {
            s << date._date;
        }
        else if (formatted_date::get_flags() == _stringed) {
            s << setfill('0') <<
                setw(2) << d << '-'
                << setw(2) << months[month_str] << '-'
                << setw(4) << y;
        }

        return os << s.str();
    }

#define CHECK_BADBIT(expr) if(expr) in.setstate(ios::badbit)


    friend istream& operator>>(istream& in, formatted_date& date)
    {
        string field;
        istringstream s;
        int d = 0, m = 0, y = 0;
        int days[12] = { 31, 28, 31, 30, 31, 30, 30, 31, 30, 31, 30, 31 };
        if (formatted_date::get_flags() == _point) {

            // DAY
            in.width(2);
            in >> field;
            s.str(field);

            // stream error
            CHECK_BADBIT(!(s >> d));
            // non-positive day
            CHECK_BADBIT(d <= 0);
            // next symbol should be '.'
            CHECK_BADBIT(in.peek() != '.');

            in.ignore();

            // MONTH
            in.width(2);
            in >> field;
            s.str(field);
            s.clear();

            // stream error
            CHECK_BADBIT(!(s >> m));
            // month should be in [1,12]
            CHECK_BADBIT((m > 12) || (m <= 0));
            // wrong day of month
            CHECK_BADBIT(d > days[m - 1]);
            // next symbol should be '.'
            CHECK_BADBIT(in.peek() != '.');

            in.ignore();

            // YEAR
            in.width(4);
            in >> field;
            s.str(field);
            s.clear();
            CHECK_BADBIT(!(s >> y));
            date._date = 10000 * y + 100 * m + d;
        }
        else if (formatted_date::get_flags() == _intgr) {

            in >> date._date;
            CHECK_BADBIT(date._date < 0);

            // MONTH
            m = (date._date / 100) % 100;
            // month should be in [1,12]
            CHECK_BADBIT((m > 12) || (m <= 0));

            // wrong day of month
            CHECK_BADBIT(d > days[m - 1]);
        }
        else if (formatted_date::get_flags() == _stringed) {

            // DAY
            in.width(2);
            in >> field;
            s.str(field);

            // stream error
            CHECK_BADBIT(!(s >> d));
            // non-positive day
            CHECK_BADBIT(d <= 0);
            // next symbol should be '-'
            CHECK_BADBIT(in.peek() != '-');

            in.ignore();

            // MONTH
            in.width(3);
            in >> field;
            m = get_month(field.c_str());
            // month should be in [1,12]
            CHECK_BADBIT((m > 12) || (m <= 0));
            // wrong day of month
            CHECK_BADBIT(d > days[m - 1]);
            // next symbol should be '-'
            CHECK_BADBIT(in.peek() != '-');

            in.ignore();

            // YEAR
            in.width(4);
            in >> field;
            s.str(field);
            s.clear();
            CHECK_BADBIT(!(s >> y));
            date._date = 10000 * y + 100 * m + d;
        }

        return in;
    }

    // ostream friend manipulators
    friend ostream& point(ostream& os);
    friend ostream& intgr(ostream& os);
    friend ostream& stringed(ostream& os);

    // istream friend manipulators
    friend istream& point(istream& os);
    friend istream& intgr(istream& os);
    friend istream& stringed(istream& os);

private:
    // формат хранения 10000*y + 100*m + day
    unsigned long _date;
    static format_flags format;
    static const char* months[];
};


// examples
void show_formatted_overloads();


} // namespace cpp



