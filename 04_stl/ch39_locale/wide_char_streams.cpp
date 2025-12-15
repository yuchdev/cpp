#include "wide_char_streams.h"
#include <iostream>
#include <locale>
#include <sstream>


using namespace std;

void cpp::wide_symbols()
{

    // wide symbols, literals or strings
    wchar_t wc = L'A';
    wchar_t str[] = L"Wide char literal";
    wstring wstr(L"Wide char string");

    wcout << str << endl;
    wcout << wstr << endl;

    // widen() method
    wcout << wcout.widen('W') << endl;

    // reverse operation narrow()
    // second param is returned if unable to narrow
    cout << cout.narrow(L'W', 'W') << endl;

    // method read() counts symbols, not bytes
    wchar_t wc1 = '\0';
    wcin.read(&wc1, 1);
    wcout << L"You entered: " << wc1 << endl;
    wcin.ignore(numeric_limits<std::streamsize>::max(), L'\n');
}

void cpp::wide_locales()
{

    // It is required to set up a locale
    // so that out wide symbols
    locale current_loc_cin = wcin.getloc();
    locale current_loc_cout = wcout.getloc();

    locale current_loc;

    cout << "Locale name: " << current_loc.name() << '\n';

    // Windows console codepage is always 866
    // (however, string ID is implementation-defined and could be incorrect)
    locale russian_loc866("rus_rus.866");

    // 65001 is a locale of this course code
    locale russian_loc1251("rus_rus.65001");

    wcin.imbue(russian_loc866);
    wcout.imbue(russian_loc1251);

    cout << "Locale name: " << russian_loc866.name() << '\n';
    cout << "Locale name: " << russian_loc1251.name() << '\n';

    wcout << L"Введена строка: "
        //<< cyrrilic_str 
        << endl;

    wstring cyrrilic_str;
    wcin >> cyrrilic_str;

    //wcin.clear(/*ios_base::goodbit*/);
    //wcin.seekg(0);
    if (wcin.fail()) {
        wcin.clear(/*ios_base::goodbit*/);
    }

    // \n should be removed after wcin usage
    wcin.ignore();

    // all other methods also work with wide symbols
    wstring cyrrilic_str1;
    getline(wcin, cyrrilic_str1);
    wcout << L"Введена строка: "
        //<< cyrrilic_str1 
        << endl;

    // back system locales
    wcin.imbue(current_loc_cin);
    wcout.imbue(current_loc_cout);
}

void cpp::wide_strings()
{

    // wide string streams example
    wstring cyrillic_name;
    int age = 0;

    wstringstream ss;

    locale current_loc_cout = wcout.getloc();
    locale russian_loc("rus_rus.866");

    // should be set without first usage!
    ss.imbue(russian_loc);
    wcout.imbue(russian_loc);

    wcout << L"Введите имя на кириллице и возраст (через пробел): " << endl;

    ss << L"ататат"
        << L' '
        << 32;

    // перед чтением надо установить укуазатель в начало потока
    ss.seekg(0);

    ss >> cyrillic_name >> age;

    wcout << L"Введены имя и возраст: " << cyrillic_name << wcout.widen(' ') << age << endl;


    wstring out(ss.str());
    wcout << out << endl;

    wcout.imbue(current_loc_cout);

    // file strams are the same
}
