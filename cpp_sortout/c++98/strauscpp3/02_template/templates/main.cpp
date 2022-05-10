#include "tmpl.h"
#include "class_tmpl.h"
#include "func_tmpl.h"
#include "alg_tmpl.h"
#include "bind_tmpl.h"

template <typename T>
void show_fetch_type_info()
{

    // info about passed types (traits, strategies) could be fetched from compiler info
    typedef T::value_type vector_value_type;
    std::cout << typeid(vector_value_type).name() << std::endl;
}

// Demonstration of template specialization
// and partial specialization of standard containers
int show_special()
{
    basic_string<char> ch_str("Char string");
    basic_string<char> ch_str2("Another char string");

    basic_string<wchar_t> wch_str(L"Wide char string");
    basic_string<wchar_t> wch_str2(L"Another wide char string");

    // Specialization is an explicit type substitution
    // Instantiation is a generation of specialized code
    container< char, lexigraphical<char> > c(ch_str);
    c.compare(ch_str2);

    container< wchar_t, no_case<wchar_t> > wc(wch_str);
    wc.compare(wch_str2);

    // using partial specialization for pointers
    my_stack<int> mystack;
    my_stack<int*> my_pointers_stack;

    return 0;
}

void show_internals()
{
    no_templ nt1;
    no_templ nt2(5);

    templ<int> t1(5);
}

void show_inherit()
{
    Xclass a, b, c;
    Yclass e, f;
    unsigned long l1 = Xclass::count;
    unsigned long l2 = Yclass::count;
}

int main()
{
    show_fetch_type_info<std::vector<int>>();
    return 0;
}
