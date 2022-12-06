#include "simple_stream.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

using namespace std;
using namespace cpp4;

void cpp4::simple_ostream()
{

    // Standard streams

    // Standard input stream (from keyboard by default)
    string s1;
    cin >> s1;

    // Standard output stream (screen by default)
    cout << "cout: " << s1 << endl;

    // Standard log stream, buffered (screen by default)
    clog << "clog: " << s1 << endl;

    // Standard error stream, non-buffered (screen by default)
    cerr << "cerr: " << s1 << endl;

    // When the stream is creating, also creating an internal "sentry" 
    // RAII class

    // Elementary types output
    int i = 5;
    long l = 50;
    short s = 10;
    double d = 0.000000000000001;
    float f = 5.5678235f;
    char c = 'a';
    unsigned u = 0xffff;
    bool b = true;

    // see above
    enum { sz = 10 };

    // integer types out in a decimal system by default
    cout << "int: " << i << endl;
    cout << "long: " << l << endl;
    cout << "short: " << s << endl;

    // even if the const is HEX
    cout << "hex uint: " << u << endl;

    // floating point restricted to 6 numbers
    cout << "float: " << f << endl;

    // ... or exponential form
    cout << "double: " << d << endl;

    // chars out as chars (not numbers)
    cout << "char: " << c << endl;

    // bool as 1/0
    cout << "bool: " << b << endl;

    // enum same as int
    cout << "enum: " << sz << endl;

    // symbol could be printed with put() method as well
    cout << "cout.put: ";
    cout.put(c);

    // or even write(), however this method is better for binary mode
    cout << " cout.wtite: ";
    cout.write(&c, 1);
    // it also does not notice trailing \0

    // endl cleans up buffer every time
    cout << endl;

    // pointers are out as HEX for any type
    int* pi = &i;
    cout << "int*: " << pi << endl;

    // the only exception are char literals
    const char* str1 = "atatat";
    cout << "const char*: " << str1 << endl;

    // so that out literal as a pointer it is required to cast to const void*
    cout << "(void*)char*: " << static_cast<const void*>(str1) << endl;

    // also write() method could be used
    cout.write(str1, strlen(str1));

    // >> << operators priority is not high
    // cout <<	d=f << '\n'; ->
    // (cout <<	d)=(f << '\n');
}

void cpp4::simple_istream()
{

    // input interrupts by pressing Enter
    // or any unacceptable symbol (. in integer)
    int i = 0;

    // enter 123.456
    cin >> i;

    cout << "You've entered int: " << i << endl;

    // Only 123 accepted
    // However left symbols are not disappeared, they can be used with the next variable
    double d = 0.;
    cin >> d;
    cout << "You've entered double: " << d << endl;

    // ignore() throws away Enter symbol
    cin.ignore();
    // enum requires its own operator >>
}


void cpp4::istream_end_input()
{
    // enter, spaces, tabs are "end of input" by default
    // enter '1 2 3'
    int a, b, c;
    cin >> a >> b >> c;
    cout << "You've entered int " << a
        << " int " << b
        << " int " << c
        << endl;

    // ignore() throws away Enter symbol
    cin.ignore();
}

void cpp4::istream_noskipws()
{
    // this mode could be switched off using manipulator noskipws
    char a, b, c;
    // enter '1 2'
    cin >> noskipws >> a >> b >> c;
    cout << "You've entered char " << a
        << " char " << b
        << " char " << c
        << endl;

    // ignore() throws away Enter symbol
    cin.ignore();
}

void cpp4::istream_get_read()
{
    // opposite methods to put/write are get/read;
    char c1, c2;
    cin.get(c1);
    cin.ignore();
    cin.read(&c2, 1);

    cout << "You've entered char " << c1
        << " char " << c2
        << endl;

    // ignore() throws away Enter symbol
    cin.ignore();
}



void cpp4::istream_strings_get()
{

    // It is possible to enter a string with spaces using get() method

    // it can be done with char array (setting number of symbols)
    char s[50] = {};

    // last position in any case set to '\0'
    cin.get(s, 50);

    cout << "You've entered:  " << s << endl;

    cin.ignore();

    char s1[50] = {};

    // end-of-input could be set . (enter atatat.a)
    cin.get(s1, 50, '.');

    cout << "You've entered:  " << s1
        << endl;

    // remove not only /n, but delimiter character as well
    // (2 last characters)
    cin.ignore();
    cin.ignore(1, '.');

    if (cin.fail()) {
        cin.clear(/*ios_base::goodbit*/);
        cout << "Fail" << endl;
    }

}


void cpp4::istream_strings_ignore()
{

    // ignore() is able to remove any number of symbols
    // standard pattern is remove everything from end-of-input 
    // to the real end of string
    char s[50] = {};
    cin.get(s, 50, '.');
    cin.ignore(numeric_limits<std::streamsize>::max(), '\n');

    cout << "You've entered:  " << s
        << endl;

    if (cin.fail()) {
        cin.clear(/*ios_base::goodbit*/);
        cout << "Fail" << endl;
    }
}


void cpp4::istream_strings_getline()
{

    // for entering the whole string getline() function is useful
    // it perform ignore() also

    string s;

    std::getline(cin, s);

    // cin.gcount() is counts number of really entered symbols
    cout << "Entered  " << s << ", " << cin.gcount() << " symbols" << endl;

    if (cin.fail()) {
        cin.clear(/*ios_base::goodbit*/);
        cout << "Fail" << endl;
    }

    // also has end-of-input param
    std::getline(cin, s, '.');

    // std::getline() perforas only ignore(),
    // so you should manually remove delimiter
    cin.ignore(1, '.');

    cout << "Entered:  " << s << endl;

    if (cin.fail()) {
        cin.clear(/*ios_base::goodbit*/);
        cout << "Fail" << endl;
    }

}


void cpp4::stream_bad_state()
{
    // Every stream can have one of 4 states
    // good - everything ok
    // fail - small error, recover is possible
    // bad - fatal error, recover is impossible
    // eof - end of input (Ctrl-Z for console)

    // stream stae flags could be set by setstate() or clear()

    // fail situation modeling
    int aint[10] = {};
    size_t i = 0;
    // enter the following sequence 1235e67
    cin >> noskipws;

    // !! Here we implicitly convert eof state using operator void*()
    // e.g. nullptr if an eof
    while ((cin >> aint[i++]) && (i < 10));

    // ...one symbol is broken
    for (int i = 0; i < 10; i++) {
        cout << aint[i] << '\n';
    }

    // ...fix the stream state
    if (cin.fail()) {
        // clear() also set the flags
        cin.clear(/*ios_base::goodbit*/);
        cout << "Fail" << endl;
    }

    cin.ignore(numeric_limits<std::streamsize>::max(), '\n');

    // reading current state: rdstate()
    ios::iostate state = cin.rdstate();

    // switch behavior on fail/bad/eof - failbit or exception
    cin.exceptions(ios::badbit | ios::failbit);

    // 0 discards exceptions
#ifdef __GNUC__
    // because GCC has a typesafe hack to prevent assignment from integers
    cin.exceptions(ios_base::iostate());
#else	
    cin.exceptions(0);
#endif
}


void cpp4::stream_place_back()
{

    // read symbols could be placed pack to the stream
    char s[50];
    cin.get(s, 50); // enter atatat

    cout << "Entered: " << s << endl;

    // place to buffer last read symbol (t)
    cin.unget();

    cout << "Still entered: " << s << endl;
    cout << s << endl;

    char s1[50];
    // check whether the symbol 't' exists in the stream,
    // but do not read from it
    if (cin.peek() == 't')
        cin.get(s1, 50);

    // symbol 't' has been read again
    cout << s1 << endl;

    cin.ignore();

    // put symbol to the stream and re-read
    cin.putback('c');
    if (cin.peek() == 'c')
        cin.get(s1, 50); // enter atatat
    cout << "Entered and one symbol has been placed before: " << s1 << endl;

    cin.ignore();
}


void cpp4::stream_redirect_buffers()
{

    // bu default all streams are buffered
    // every stream has associated object called streambuf

    // for every buffered stream it's possible to get the buffer itself
    // and work with it directly
    // This make possible many operations in a few lines of code
    // E.g. copy one file to another

    // Also:
    // 1. (out file into stdout)
    ifstream f("cmake_install.cmake");
    cout << f.rdbuf() << endl;

    // 2. tie one buffer to several streams
    ostream hexout(cout.rdbuf());
    hexout.setf(ios::hex, ios::basefield);
    hexout.setf(ios::showbase);
    hexout << 10 << endl;

    // redirect output to file
    ofstream out_file("out_file.txt");

    // save prev buffer for cout
    streambuf* buf = cout.rdbuf();
    cout.rdbuf(out_file.rdbuf());
    cout << "Enter file contents" << endl;

    // restore prev buffer cout
    cout.rdbuf(buf);

    // flush() method writes all buffer containment
    cout.flush();

    // streambuf has protected interface for inheritance
    // and user-defined buffers
}

void cpp4::stream_tie()
{

    // By default, the standard narrow streams cin and cerr are tied to cout, and their
    // wide character counterparts(wcin and wcerr) to wcout.Library implementations 
    // may also tie clog and wclog
    std::ostream* prevstr;
    std::ofstream ofs;
    ofs.open("test.txt");

    std::cout << "tie example:\n";

    *std::cin.tie() << "This is inserted into cout";
    prevstr = std::cin.tie(&ofs);

    *std::cin.tie() << "This is inserted into the file";
    std::cin.tie(prevstr);

    ofs.close();
}


void cpp4::output_string_streams()
{

    char c = 'a';
    long l = 10l;
    double d = 100.5;
    ostringstream os;

    // std::string is allocated inside stringstream using new()
    os << "Formatted string "
        << "Char:" << c
        << " Long:" << l
        << " Double:" << d;
    cout << os.str() << endl;
    cout << "Total symbols: " << os.str().length() << endl;
}

void cpp4::bidirect_string_streams()
{

    // istringstream receives data from some type from string
    // and often useless itself

    // it's make sense to use bidirectional stringstream
    // that combines two previous streams

    string name;
    int age = 0;

    stringstream ss;
    cout << "Enter name age (through whitespace): ";

    ss << "atatat"
        << ' '
        << 32;

    // before reading set position to beginning of the stream
    ss.seekg(0);

    ss >> name >> age;

    cout << "Entered name: " << name << ", and age: " << ' ' << age << endl;

    string out(ss.str());
    cout << out << endl;
}

