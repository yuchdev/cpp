#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <codecvt>
#include <ctime>
#include <cctype>
#include <iterator>
#include "wide_char_streams.h"

using namespace std;

// see system-specific table of locales
// https://docs.moodle.org/dev/Table_of_locales
#if defined(_WIN32) || defined(_WIN64)
#define US_AU_locale "English_Australia.1252"
#define GER_DE_locale "German_Germany.1252"
#define SW_SW_locale "Swedish_Sweden.1252"
#define JA_JP_locale "Japanese_Japan.932"
#else
#define US_AU_locale "en.UTF-8"
#define GER_DE_locale "de_DE.UTF-8"
#define SW_SW_locale "sv_SE.UTF-8"
#define JA_JP_locale "ja_JP.UTF-8"
#endif

/*

New features:

Examples:
1. Current locale
2. New locale with customized facets (39.2.1.1)
3. Locale and wide stream examples
4. Locale-sensitive compare strings (39.2.2.)
5. collate facet (39.4.1)
6. num_put, num_get, numpunct facets (39.4.2)
7. money_put, money_get, moneypunct facets (39.4.3)
8. Date and time facets (39.4.4)
9. ctype facet (39.4.5)
10. codecvt facet (39.4.6)
11. Messages base (39.4.7)
12. Convenience Interfaces (39.5)
13. User-defined facet (39.3.2)

*/

// Note that telling std::cout to use L has no effect on other streams' locales

// std::locale is a type-safe heterogeneous container class
// Locales have very little to do with localization; 
// all of the real work is performed by the objects contained in locales
// Those objects are called facets

// First, once a locale has been constructed, there is no way to add new elements
// Second, a locale can contain at most one element of a particular type. The type serves as a sort of index: 
// you can write std::has_facet<T>(L) to ask a locale L if it has an element of type T, and, 
// if it does, you can use std::use_facet<T>(L) to get a reference to that element
// (If a locale has no element of type T, then std::use_facet<T>(L) will throw an exception) 
// Third, any type that you put in a locale object must inherit from the base class std::locale::facet 
// and must contain a static member variable, called id, whose type is std::locale::id

// That's what std::locale("de") really means: you're getting a locale where the facet objects are 
// std::ctype_byname<char>("de"), std::numpunct_byname<char>("de"), and so on

// C++ locales were designed to be extensible in two ways: by deriving a new class from an existing facet, 
// or by adding new kinds of facets
// The Standard C++ library has already given us examples of the first method, the *_byname facets

// If the source contains non-ASCII symbols, it's better to standard the encoding (UTF-8 or UTF-16, last is better)

//1. Current locale
//2. New locale with customized facets(39.2.1.1)
void show_locale_naming() {
    // It is generally a good idea to avoid embedding locale name strings in the program text
    // to use named locales on a given system, a programmer must refer to system documentation and experiment

    // During program startup, the equivalent of std::setlocale(LC_ALL, "C"); is executed before any user code is run
    //LC_COLLATE	selects the collation category of the C locale
    //LC_CTYPE	    selects the character classification category of the C locale
    //LC_MONETARY	selects the monetary formatting category of the C locale
    //LC_NUMERIC	selects the numeric formatting category of the C locale
    //LC_TIME	    selects the time formatting category of the C locale
    //LC_ALL	    selects the entire C locale

    // copy of the current global locale
    locale loc1;
    cout << "loc1 name: " << loc1.name() << '\n';
    
    // copy of the user's preferred locale
    locale loc2{ "" };
    cout << "loc2{ "" } name: " << loc2.name() << '\n';

    // copy of the 'C' locale
    locale loc3{ "C" };
    cout << "loc3{ C } name: " << loc3.name() << '\n';

    // copy of the 'C' locale
    locale loc4{ locale::classic() };
    cout << "loc4{ locale::classic() } name: " << loc4.name() << '\n';

    // these locales may not exist:
    try{
        locale loc5{ "POSIX" };
        cout << "loc5{ POSIX } name: " << loc5.name() << '\n';
    }
    catch (const std::runtime_error& e){
        cout << e.what() << '\n';
    }

    try {
        locale loc6{ "Russian_Russia.1252" };
        cout << "loc6{ Russian_Russia.1252 } name: " << loc6.name() << '\n';
    }
    catch (const std::runtime_error& e) {
        cout << e.what() << '\n';
    }

    try {
        locale loc7{ "ru_RU.UTF-8" };
        cout << "locale loc7{ ru_RU.UTF-8 } name: " << loc7.name() << '\n';
    }
    catch (const std::runtime_error& e) {
        cout << e.what() << '\n';
    }

    try {
        locale loc8{ "en_US.UTF-8" };
        cout << "locale loc8{ en_US.UTF-8 } name: " << loc8.name() << '\n';
    }
    catch (const std::runtime_error& e) {
        cout << e.what() << '\n';
    }

    try {
        locale loc9{ "Russian_Russia.UTF-8" };
        cout << "loc9{ Russian_Russia.UTF-8 } name: " << loc9.name() << '\n';
    }
    catch (const std::runtime_error& e) {
        cout << e.what() << '\n';
    }

    try {
        locale loc10{ "rus_rus.866" };
        cout << "loc10{ rus_rus.866 } name: " << loc10.name() << '\n';
    }
    catch (const std::runtime_error& e) {
        cout << e.what() << '\n';
    }

    // Constructs a copy of other except for all the facets identified by the cat argument, 
    // which are copied from the system locale identified by its std_name. 
    // The locale constructed in this manner has a name if and only if other has a name

    // loc11 is "C" (loc3) except for ctype, which is User-preferred locale
    std::locale loc11(loc3, loc2, std::locale::ctype); 
    cout << "loc11 name: " << loc11.name() << '\n';

    // loc12 is "C" except for codecvt
    std::locale loc12(loc3, new std::codecvt_utf8<wchar_t>); 
    cout << "loc12 name: " << loc12.name() << '\n';
}

// 3. Locale and wide stream examples
void show_wchar_streams(){
    cpp4::wide_symbols();
    cpp4::wide_locales();
    cpp4::wide_strings();
}

//4. Locale - sensitive compare strings(39.2.2)
void show_compare_strings() {

    // This string comparison function is defined as locale’s operator()()
    std::locale loc{ "C" };
    string s1{ "atatat1" };
    string s2{ "atatat2" };
    
    loc(s1, s2); // compare

    // locale could be passed to other comparable, like std::sort
    // sort(v.begin(),v.end(),my_locale);
}

//5. collate facet(39.4.1)
void show_collate() {
    // Class std::collate encapsulates locale-specific collation (comparison) and hashing of strings
    // This facet is used by std::basic_regex and can be applied, by means of std::locale::operator(), 
    // directly to all standard algorithms that expect a string comparison predicate

    std::wcout.imbue(std::locale(""));

    std::vector<std::wstring> v = { 
        L"ar", 
        L"zebra", 
        L"\u00f6grupp", 
        L"Zebra", 
        L"\u00e4ngel",
        L"\u00e5r", 
        L"f\u00f6rnamn" };

    std::wcout << "Default locale collation order: ";
    std::sort(v.begin(), v.end());
    for (auto s : v) 
        std::wcout << s << ' '; std::wcout << '\n';

    std::wcout << "English (AU) locale collation order: ";
    std::sort(v.begin(), v.end(), std::locale(US_AU_locale));
    for (auto s : v) 
        std::wcout << s << ' '; std::wcout << '\n';

    std::wcout << "Swedish (SW) locale collation order: ";
    std::sort(v.begin(), v.end(), std::locale(SW_SW_locale));
    for (auto s : v) 
        std::wcout << s << ' '; std::wcout << '\n';
}

namespace cpp4 {

// format and convert double in particular locale
double apply_locale_to_double(const std::string& formatted_double, std::locale& loc){
    std::istringstream my_stream(formatted_double);
    my_stream.imbue(loc);
    double f1;
    my_stream >> f1;
    return f1;
}

std::string apply_locale_to_double(double formatted_double, std::locale& loc) {
    std::stringstream my_stream;
    my_stream.imbue(loc);
    my_stream << formatted_double;
    return my_stream.str();
}

// facet with French true/false names
struct french_bool : std::numpunct<char> {
    string_type do_truename() const { return "oui"; }
    string_type do_falsename() const { return "non"; }
};

} // namespace cpp4

//6. num_put, num_get, numpunct facets(39.4.2)
void show_numbers() {


    // Class std::num_get encapsulates the rules for parsing string representations of numeric values

    // How double is formatted (different ,/. separators)
    std::string de_formatted_double = "1.234.567,89";
    std::string us_formatted_double = "1,234,567.89";

    locale us_loc(US_AU_locale);
    locale de_loc(GER_DE_locale);

    std::cout << "DE-formatted in DE locale: " << std::fixed << cpp4::apply_locale_to_double(de_formatted_double, de_loc) << '\n';
    std::cout << "DE-formatted in US locale: " << std::fixed << cpp4::apply_locale_to_double(de_formatted_double, us_loc) << '\n';
    std::cout << "US-formatted in DE locale: " << std::fixed << cpp4::apply_locale_to_double(us_formatted_double, de_loc) << '\n';
    std::cout << "US-formatted in US locale: " << std::fixed << cpp4::apply_locale_to_double(us_formatted_double, us_loc) << '\n';

    // use the facet directly: get it from the locale and call get() - that is internally do_get() method
    std::istringstream us_au_stream(us_formatted_double);
    us_au_stream.imbue(std::locale(US_AU_locale));

    // get the formatting facet from the locale
    auto& get_f = std::use_facet<std::num_get<char>>(us_au_stream.getloc());
    
    std::istreambuf_iterator<char> beg(us_au_stream), end;
    double f3 = 0.;
    std::ios::iostate err;

    // perform converting "formatted double" -> double
    double n = 1234567.89;
    get_f.get(beg, end, us_au_stream, err, f3);
    std::cout << "Parsing " << us_formatted_double << " as double using raw en_AU facet gives " << f3 << '\n';

    // Vise versa, std::num_put encapsulates the rules for formatting numeric values as strings
    std::cout << "Double in DE locale: " << cpp4::apply_locale_to_double(n, de_loc) << '\n';
    std::cout << "Double in US locale: " << cpp4::apply_locale_to_double(n, us_loc) << '\n';

    // use the facet directly
    auto& put_f = std::use_facet<std::num_put<char>>(std::cout.getloc());
    put_f.put(std::ostreambuf_iterator<char>(std::cout), std::cout, ' ', n);
    std::cout << '\n';

    // The facet std::numpunct encapsulates numeric punctuation preferences
    // (. , true/false)
    std::cout << "default locale: " << std::boolalpha << true << ", " << false << '\n';

    std::locale french_bool_numpunct(std::cout.getloc(), new cpp4::french_bool);
    std::locale prev = cout.imbue(french_bool_numpunct);
    std::cout << "Custom locale: " << std::boolalpha << true << ", " << false << '\n';
    
    // take back normal locale
    cout.imbue(prev);
}


//7. money_put, money_get, moneypunct facets(39.4.3)
namespace cpp4{

long double money_to_double(const std::string& money_str, std::locale& loc){
    std::istringstream s(money_str);
    s.imbue(loc);
    long double val{};
    s >> std::get_money(val);
    return val;
}

std::string double_to_money(long double money_val, std::locale& loc) {
    std::stringstream s;
    s.imbue(loc);
    // 2nd param is currency code (does not work Win?)
    s << std::put_money(money_val, true);
    return s.str();
}


// Here is a simple format defined as a facet
class my_money_io : public moneypunct<char, true> {
public:

    explicit my_money_io(size_t r = 0) : moneypunct<char, true>(r) { }

    char_type do_decimal_point() const { return '.'; } 
    char_type do_thousands_sep() const { return ','; } 
    string do_grouping() const { return "\003\003\003"; }

    string_type do_curr_symbol() const { return "USD "; } 
    string_type do_positive_sign() const { return ""; } 
    string_type do_negative_sign() const { return "()"; }

    // two digits after decimal point
    int do_frac_digits() const { return 2; } 
    pattern do_pos_format() const { return pat; }
    pattern do_neg_format() const { return pat; } 
private:
        static const pattern pat;
};

const my_money_io::pattern my_money_io::pat{ sign, symbol, value, none };
} // namespace cpp4


void show_money() {

    locale us_loc(US_AU_locale);
    locale de_loc(GER_DE_locale);
    locale sw_loc(SW_SW_locale);

    // Class template std::money_get encapsulates the rules for parsing monetary values from character streams
    // The standard I/O manipulator std::get_money uses the std::money_get facet of the I/O stream's locale
    string usd1 = "$1.11";
    string usd2 = "2.22";
    string usd3 = "3.33 USD";

    string eur1 = "ˆ1.11";
    string eur2 = "2.22";
    string eur3 = "3.33 EUR";

    std::cout << "Money string " << usd1 << " converted to double " << cpp4::money_to_double(usd1, us_loc) << '\n';
    std::cout << "Money string " << usd2 << " converted to double " << cpp4::money_to_double(usd2, us_loc) << '\n';
    std::cout << "Money string " << usd3 << " converted to double " << cpp4::money_to_double(usd3, us_loc) << '\n';

    // wtf with euro?
    std::cout << "Money string " << eur1 << " converted to double " << cpp4::money_to_double(eur1, de_loc) << '\n';
    std::cout << "Money string " << eur2 << " converted to double " << cpp4::money_to_double(eur2, de_loc) << '\n';
    std::cout << "Money string " << eur3 << " converted to double " << cpp4::money_to_double(eur3, de_loc) << '\n';

    // use money_get facet directly
    long double val{};
    std::istringstream us_au_stream(usd1);
    us_au_stream.imbue(std::locale(us_loc));
    auto& f = std::use_facet<std::money_get<char>>(us_au_stream.getloc());
    std::ios_base::iostate err;
    std::istreambuf_iterator<char> beg(us_au_stream), end;
    f.get(beg, end, true, us_au_stream, err, val);
    std::cout << val / 100 << " USD" << '\n';

    // Vise versa, class std::money_put encapsulates the rules for formatting monetary values as strings
    // The standard I/O manipulator std::put_money uses the std::money_put facet of the I/O stream's locale
    double money_value = 123.05;
    std::cout << "Double value " << money_value << " converted to money using US locale " << cpp4::double_to_money(money_value, us_loc) << '\n';
    std::cout << "Double value " << money_value << " converted to money using DE locale " << cpp4::double_to_money(money_value, de_loc) << '\n';
    std::cout << "Double value " << money_value << " converted to money using SW locale " << cpp4::double_to_money(money_value, sw_loc) << '\n';
    
    // using the facet directly
    locale prev = std::cout.imbue(de_loc);
    std::cout << "German locale: ";
    std::ostreambuf_iterator<char> out(std::cout);
    auto& money_p = std::use_facet<std::money_put<char>>(std::cout.getloc());
    money_p.put(out, false, std::cout, std::cout.fill(), 12345678.9);
    std::cout << '\n';
    std::cout.imbue(prev);

    // The facet std::moneypunct encapsulates monetary value format preferences
    // Stream I/O manipulators std::get_money and std::put_money use std::moneypunct through 
    // std::money_get and std::money_put for parsing monetary value input and formatting monetary value output
    // (- . , etc)

    // E.g. representing a negative number using parentheses is achieved by having negative_sign() 
    // return a string containing the two characters "()"
}

//8. Date and time facets(39.4.4)
void show_time() {
    
    locale us_loc(US_AU_locale);
    locale de_loc(GER_DE_locale);
    locale jp_loc(JA_JP_locale);

    // Class template std::time_get encapsulates date and time parsing rules
    // The I/O manipulator std::get_time uses the std::time_get facet of the I/O stream's locale 
    // to convert text input to a std::tm object
    std::wstring input = L"2011-Februar-18 23:12:34";
    std::tm t = {};
    std::wistringstream ss(input);
    ss.imbue(std::locale(de_loc));

    // uses std::time_get<wchar_t>
    ss >> std::get_time(&t, L"%Y-%b-%d %H:%M:%S"); 

    // Converts given calendar time std::tm to a textual representation
    std::cout << std::asctime(&t);

    // Class template std::time_put encapsulates date and time formatting rules
    // The I/O manipulator std::put_time uses the std::time_put facet of the I/O stream's locale 
    // to generate text representation of an std::tm object
    
    std::time_t t1 = std::time(NULL);
    std::wbuffer_convert<std::codecvt_utf8<wchar_t>> conv(std::cout.rdbuf());
    std::wostream out(&conv);
    
    // use Japanese locale to output time
    out.imbue(std::locale(jp_loc));
    // this I/O manipulator std::put_time uses std::time_put<wchar_t>
    out << std::put_time(std::localtime(&t1), L"%A %c") << '\n';
}

//9. ctype facet(39.4.5)
namespace cpp4 {

// The following example demonstrates modification of a ctype other than ctype<char> to tokenize a CSV file
struct csv_whitespace : std::ctype<wchar_t>
{
    bool do_is(mask m, char_type c) const
    {   
        // space will NOT be classified as whitespace
        if ((m & space) && c == L' ') {
            return false; 
        }
        
        // comma will be classified as whitespace
        if ((m & space) && c == L',') {
            return true; 
        }
        return ctype::do_is(m, c); // leave the rest to the parent class
    } 
};

} // namespace cpp4 

void show_ctype() {

    // Class ctype encapsulates character classification features
    // All stream input operations performed through std::basic_istream<charT> use the std::ctype<charT> 
    // of the locale imbued in the stream to identify whitespace characters for input tokenization
    // Stream output operations apply std::ctype<charT>::widen() to narrow-character arguments prior to output

    std::locale loc(std::locale{""}, new cpp4::csv_whitespace());

    std::wstring in = L"Column 1,Column 2,Column 3\n123,456,789";
    std::wstring token;

    std::wcout << "default locale:\n";
    std::wistringstream s1(in);
    while (s1 >> token) {
        std::wcout << "  " << token << '\n';
    }

    std::wcout << "locale with modified ctype:\n";
    std::wistringstream s2(in);
    s2.imbue(loc);
    while (s2 >> token) {
        std::wcout << "  " << token << '\n';
    }
}


//10. codecvt facet(39.4.6)
void show_code_convert() {
    // Class std::codecvt encapsulates conversion of character strings, including wide and multibyte, 
    // from one encoding to another
    // TODO: 
}

//11. Messages base(39.4.7)
void show_messages() {
    
    // Class template std::messages is a standard locale facet that encapsulates retrieval of strings 
    // from message catalogs, such as the ones provided by GNU gettext or by POSIX catgets

    // SO: Message catalogs are part of POSIX (e.g., gencat(1), catopen(3))
    // Windows also has message catalogs, though I'm not sure if they've ever bothered to implement 
    // the standard locale message facets

    std::locale de_loc(GER_DE_locale);
    locale prev = std::cout.imbue(de_loc);
    auto& facet = std::use_facet<std::messages<char>>(de_loc);

    // it seems MS does implement no one catalog
    auto cat = facet.open("sed", de_loc);
    if (cat < 0)
        std::cout << "Could not open German \"sed\" message catalog\n";
    else
        std::cout << "\"No match\" in German: " << facet.get(cat, 0, 0, "No match") << '\n'
        << "\"Memory exhausted\" in German: " << facet.get(cat, 0, 0, "Memory exhausted") << '\n';
    facet.close(cat);
    std::cout.imbue(prev);
}
//12. Convenience Interfaces(39.5)
void show_convenience_interface() {

    std::locale loc("");
    // Characters classification and conversion wrappers
    
    // same as use_facet<ctype<C>>(loc).isspace(c)
    std::cout << std::isspace(' ', loc) << '\n';
    std::cout << std::isspace(' ') << '\n'; // same, declared <cctype>

    // same as use_facet<ctype<C>>(loc).toupper(c)
    std::cout << std::toupper('u', loc) << '\n';
    std::cout << std::tolower('U') << '\n'; // same, declared <cctype>

    // Character code conversions can be locale sensitive
    // Class template wstring_convert performs conversions between a wide string and a byte string
    wstring_convert<codecvt_utf8<wchar_t>> myconv; 
    string s = myconv.to_bytes(L"Hello\n");
    cout << s;

    wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
    string s8 = u8"This is a UTF-8 string"; 
    wstring s16 = converter.from_bytes(s8); 
    string s88 = converter.to_bytes(s16);
    if (s8 != s88) {
        cerr << "Insane!\n";
    }
        
    // We also can use a code conversion facet wbuffer_convert
    // to write directly into or to read directly from a stream buffer
}

//13. User-defined facet (39.3.2)
namespace cpp4 {

// very simple user-defined type
enum season { spring, summer, fall, winter }; 

// define facet for seasons (at first interface class only)
class season_io : public locale::facet {
public:
    
    // lifetime managed by locale
    season_io(size_t lifetime = 0) : locale::facet(lifetime) {}

    // trivial destructor
    ~season_io() {}

    // string representation of x 
    virtual const string& to_str(season x) const = 0; 
    
    // place season for s in x
    virtual bool from_str(const string& s, season& x) const = 0; 

    static locale::id id; // facet identifier object 
};

locale::id season_io::id; // define the identifier object

// define output for our class using facet
std::ostream& operator<<(std::ostream& os, season s) {
    locale loc(os.getloc());

    if (std::has_facet<cpp4::season_io>(loc)) {
        return os << std::use_facet<cpp4::season_io>(loc).to_str(s);
    }
    
    // we don't use such a facet, just cast to int
    return os << static_cast<int>(s);
}
// also operator << must be implemented

// implement now seasons representation in English
class us_seasons_io : public cpp4::season_io {
public:

    // string representation of x 
    virtual const string& to_str(season x) const override {
        if (x < spring || winter < x) {
            static const string ss = "no-such-season"; 
            return ss;
        }
        return us_seasons[x];
    }

    // place season for s in x
    virtual bool from_str(const string& s, season& x) const override {
        auto it = std::find(std::begin(us_seasons), std::end(us_seasons), s);
        if (it != us_seasons.end()) {
            x = season(it - us_seasons.begin());
            return true;
        }
        return false;
    }

private:
    static vector<string> us_seasons;
};

vector<string> us_seasons_io::us_seasons = {
    "spring",
    "summer", 
    "fall", 
    "winter"
};
} // namespace cpp4

void show_user_defined_facet() {
    locale loc(locale{}, new cpp4::us_seasons_io);
    locale prev = cout.imbue(loc);

    cout << cpp4::summer << std::endl;
    cout.imbue(prev);
}

int main() {

    // TODO: rus_rus.65001 on Windows does not exists
    //show_wchar_streams();

    show_locale_naming();
    show_compare_strings();
    show_collate();
    show_numbers();
    show_money();
    show_time();
    show_ctype();
    show_messages();
    show_convenience_interface();
    show_user_defined_facet();

    return 0;
}
