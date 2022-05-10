#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>

using namespace std;

void show_string()
{

    // string is actually a basic_string template
    basic_string<char, char_traits<char> > str;

    // char_traits is a property template that provides the following information:
    // * symbol types
    // * properties of conversion to integral types
    // * properties of low-level operations with symbol arrays
    // * properties of input/output

    //Initialization
    string s1;
    string s2(s1);
    string s3("str");
    // cannot be initialized by a symbol, but can be assigned
    s1 = 'a';

    // passing a null pointer causes an exception
        // In the debug version assert
#if 0
    try {
        const char* c = nullptr;
        std::string s(c);
        size_t sz = s.size();
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
#endif

    // there's overload to assign c-string without calling an additional constructor
    s2 = "atatat";

    // standard algorithms are less useful for strings than for other containers
    // for substring search, merge, etc, it is better to use built-in operations

    // conversion to C-string (with \0) and to array (without \0)
    const char* c1 = s3.c_str();
    const char* c2 = s3.data();

    // it is possible to compare c-string without calling an additional constructor
    bool b = (s2 == "atatat");

    // the search returns the index of the beginning of the substring or `npos`
    string::size_type s = s2.find("at");
    if (s != string::npos) {

        cout << s2 << endl;
    }

    // substring
    string s4 = s3.substr(0, 2);

    // functions for working with ASCII
    // can be used in algorithms, including modifying
    // isalpha, isdigit, ispunct, tolower, ...
    std::transform(s3.begin(), s3.end(), s3.begin(), ::toupper);
}

void str_rev(char* p)
{
    char* q = p;
    while (q && *q) ++q;
    for (--q; p < q; ++p, --q) {
        *p = *p ^ *q;
        *q = *p ^ *q;
        *p = *p ^ *q;
    }
}

#define SWP(x,y) (x^=y, y^=x, x^=y)

void str_rev_utf8(char* p)
{
    char* q = p;
    str_rev(p); /* call base case */

    /* Ok, now fix bass-ackwards UTF chars. */
    while (q && *q) ++q; /* find eos */
    while (p < --q)
        switch ((*q & 0xF0) >> 4) {
        case 0xF: /* U+010000-U+10FFFF: four bytes. */
            SWP(*(q - 0), *(q - 3));
            SWP(*(q - 1), *(q - 2));
            q -= 3;
            break;
        case 0xE: /* U+000800-U+00FFFF: three bytes. */
            SWP(*(q - 0), *(q - 2));
            q -= 2;
            break;
        case 0xC: /* fall-through */
        case 0xD: /* U+000080-U+0007FF: two bytes. */
            SWP(*(q - 0), *(q - 1));
            q--;
            break;
        }
}

void show_c_string_rev()
{

    char str[] = "atatat";
    printf("%s ", str);
    str_rev(str);
    str_rev_utf8(str);
    printf("%s\n", str);

    // C++ way
    //std::reverse(str.begin(), str.end());
}

int main()
{

    show_string();
    show_c_string_rev();
    return 0;
}
