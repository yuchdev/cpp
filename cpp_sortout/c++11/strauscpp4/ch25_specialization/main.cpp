#include <iostream>
#include <vector>

using namespace std;

/*

New features:
* Local classes as template arguments (25.2.1)
* Default template arguments for function templates (25.2.5.1)
* More explicit control of template instantiation: extern templates (26.2.2)

Examples:
1. Local classes as template arguments
2. Default template arguments for function templates
3. More explicit control of template instantiation: extern templates
*/


// 1. Local classes as template arguments
// 2. Default template arguments for function templates
namespace cpp4
{

template <typename T>
struct S
{
    T member;
};

// functions also may have default tmpl args
template <typename T = int, typename Cont = std::vector<T>>
void container()
{
    Cont v { 1,2,3,4,5 };
    std::cout << *v.begin() << std::endl;
}

} // namespace cpp4 

void show_local_and_defaults()
{
    // now local classes are available for template params
    class my_local
    {
        int i = 0;
    };

    cpp4::S<my_local> l;

    // if all function tmpl args are defaulted
    // it is called like a plain function
    cpp4::container();
}

// 3. More explicit control of template instantiation: extern templates
// http://stackoverflow.com/questions/8130602/using-extern-template-c0x
// You should only use extern template to force the compiler to not instantiate a template 
// when you know that it will be instantiated somewhere else, its used to reduce compile time and reduce object file size
// It could be useful for really big template functions

int main()
{
    show_local_and_defaults();
    return 0;
}
