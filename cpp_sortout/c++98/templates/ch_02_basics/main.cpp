#include <deque>
#include <cstring>
#include <complex>

#include "func_tmpl.h"
#include "class_tmpl.h"
#include "non_type_params.h"
#include "basics_tmpl.h"

using std::string;

// Demonstration of a simple template function 
void show_func_template()
{

    int i = 10;

    // We use the global namespace form ::max
    // not to be confused with std::max
    ::max(7, i);
    ::max(3.4, 7.6);

    std::string s1("atatat");
    std::string s2("bdbdbdb");
    ::max(s1, s2);

    // In all three of these forms, the type has been found automatically
    // The so-called "template arguments deduction", a link between the parameters of the template and its arguments
    // It is possible to do it explicitly as well
    int a = ::max<int>(i, 5);

    // For correct instantiation, a comparison operation must be defined for T type
    // for example for std::complex will not work
    std::complex<double> c1(0.1, 0.2);
    std::complex<double> c2(.0, 0.5);

    // ::max(c1, c2);
    // error C2784: 'bool std::operator >(const std::basic_string<_Elem,_Traits,_Alloc> &,const _Elem *)'
    // : could not deduce template argument for 'const std::basic_string<_Elem,_Traits,_Alloc> &'
    // from 'const std::complex<double>'

    // In other words it is necessary so that each operation for a template element was avaiulable

    // plain function with implicit conversion
    int b = ::max(1, 3.5);
    // warning C4244: 'argument' : conversion from 'double' to 'const int', possible loss of data

    // Explicit call of template function (<> refers to a template form)
    b = ::max<>(1, 2);

    // The code of the template is compiled 2 times -
    // at syntactic correctness check and at instantiation
}


// Demonstration of asaimple template class
void show_class_tmpl()
{
    stack<int> s;

    s.push(1);
    s.push(2);

    // Instantiation occurs only for used member functions
    int i = s.top();
    s.pop();
    // in other words bool empty() has not been instantiated

    stack<string> s2;
    s2.push("atatat1");
    s2.push("atatat2");
}

struct textme
{
    void method();
    void method(int);
};

// Demonstration of partial specialization
void show_parital_spec()
{
    double_stack<long, int> ds0;	// specialization "second parameter is int"
    double_stack<int, double> ds1;	// no specialization
    double_stack<double, double> ds2;	// specialization "parameter are the same"
    double_stack<double*, int*> ds3;	// specialization "parameter are pointers"
    double_stack<void(*)(), void(*)(int)> ds4; // instantiation by pointer to functions

    // error C2752: 'double_stack<int *,int *>': more than one partial specialization matches
    //double_stack<int*, int*> ds5;	
    // two specializations match "parameter are the same" and "parameter are pointers"

    double_stack<void(textme::*)(), void(textme::*)(int)> ds6; // pointer to method is not a pointer!
}

// Demonstration of default parameters
void show_default_param()
{
    custom_stack<int> st1;
    custom_stack<int, std::deque<int> > st2;

    st1.push(1);
    st2.push(1);
}

// Demonstration of parameters which are not types
// (fixed-size array template class)
void show_non_type_params()
{

    // These are different types!
    fixed_array<int, 20> ar1;
    fixed_array<int, 40> ar2;

    try {
        ar1[10] = 10;
        ar2[20] = 10;

        // Throw a bound-check exception here
        ar2[200] = 10;
    }
    catch (const std::exception& e) {
        cout << e.what() << endl;
    }
}

// Application of keywords typename and template
void show_typename_tmpl()
{
    std::list<int> v;
    v.push_back(1);
    v.push_back(2);

    print_container< std::list<int> >(v);

    test_bitset();
}

// Demonstration of template method of template class
void show_template_member()
{
    copyable_stack<int> is;
    copyable_stack<double> ds;

    is.push(1);
    ds.push(1.0);
    ds.push(2.0);
    is = ds;
}

// Demonstration of template class with template argument
void show_tmpl_param()
{

    // Element type is int, base container is default
    tmpl_stack<int> s1;
    s1.push(1);
    s1.push(2);

    // Element type is doubcle, base container is std::vector
    tmpl_stack<double, std::vector> s2;
    s2.push(1.0);
    s2.push(3.0);

    // Instantiate operator= (the most complex t member)
    s1 = s2;

    // For function templates, no template parameters are allowed!
}


// Demonstration of zero initialization
void show_zero_init()
{

    // All types will be initialized with zero or default constructor
    init_zero<int> iz1;
    init_zero< std::vector<int> > iz2;
    init_zero< std::string > iz3;
}

// Demonstration of string literals and array conversion to pointers
// Conversion of an array to a pointer is performed only if the array name is passed by value
void show_literal()
{

    // Error: the array cannot be cast to a pointer because it is passed by reference
    // error C2440: 'return' : cannot convert from 'const char *' to 'const char (&)[5]'
    // str_max_ref("aaaa", "bbbb");
    show_ref("aaaa");

    // And here you can cast the array to a pointer, since it is passed by value
    str_max_val("aaaa", "bbbb");
    show_val("aaaa");

    //To avoid such situations:
    // - pass by value
    // - Do not use C-string
    // The first method is used in std::make_pair(f, s) - both parameters by value to pass literals
}

int main()
{

    show_func_template();
    show_class_tmpl();
    show_parital_spec();
    show_default_param();
    show_non_type_params();
    show_typename_tmpl();
    show_tmpl_param();
    show_literal();


    return 0;
}
