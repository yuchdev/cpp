#include <list>
#include <string>
#include <algorithm>
#include <functional>

using namespace std;

// ------------------- Bulky debug messages -------------------
void show_long_error()
{
    list<string> coll;

    // An error was made here
    // an int template parameter is passed to the 'greater' predicate
    // when the comparison should be performed by by std::string
    list<string>::iterator pos =
        find_if(coll.begin(), coll.end(),
            bind2nd(greater</*int - error!*/string>(), "A"));
    /*
    error C2440: 'initializing' : cannot convert from 'const char [2]' to 'int'
    There is no context in which this conversion is possible
    d:\projects\study\cpp\tmpl\part_01_debugging\debug_error.cpp(15) :
    see reference to function template instantiation 'std::binder2nd<_Fn2>
    std::bind2nd<std::greater<_Ty>,const char[2]>(const _Fn2 &,const char (&))' being compiled
    with
    [
    _Fn2=std::greater<int>,
    _Ty=int
    ]
    */

    // As you can see, the debug message is quite hard to read
}

// Shallow instantiation
// Consider the hierarchy of template functions we will call later

// This function assumes that T is a pointer!
template<typename T>
void clear(T& p)
{
    *p = 0; // error!

}

// this one just passes in a parameter (not necessarily a pointer!)
template<typename T>
void core(T& p)
{
    clear(p);
}

// This too
template<typename T>
void middle(T& p)
{
    core(p);
}

// The top function in the hierarchy, passes an 'int' type
// Which is "not a pointer
template<typename T>
void up(const T& env)
{
    // something wrong with env
    typename T::index_t p;
    middle(p);
}

class client
{
public:
    typedef int index_t;
};

void test_stack_error()
{
    //client cli;	// - uncomment for error
    //up(cli);		// - uncomment for error
    // Error messages through all the call stack

    /*
    error C2100: illegal indirection
    d:\projects\study\cpp\tmpl\part_01_debugging\debug_error.cpp(45) :
    see reference to function template instantiation 'void clear<T>(T &)' being compiled
    with
    [
    T=client::index_t
    ]
    d:\projects\study\cpp\tmpl\part_01_debugging\debug_error.cpp(51) :
    see reference to function template instantiation 'void core<T>(T &)' being compiled
    with
    [
    T=client::index_t
    ]
    d:\projects\study\cpp\tmpl\part_01_debugging\debug_error.cpp(60) :
    see reference to function template instantiation 'void middle<client::index_t>(T &)' being compiled
    with
    [
    T=client::index_t
    ]
    d:\projects\study\cpp\tmpl\part_01_debugging\debug_error.cpp(70) :
    see reference to function template instantiation 'void up<client>(const T &)' being compiled
    with
    [
    T=client
    ]
    */
}

// The "shallow instantiation" model is used to avoid such situations. 
// at the top level, it checks if the template parameter meets some requirements
// In this case, if parameter is the pointer. Approximately like this:
template <typename T>
class shallow_checks
{
public:
    // Empty function to suppress optimization
    // otherwise, the parameter is unused
    static void ignore(const T& p) { T a = p; }
    static void check_is_pointer(T ptr)
    {
        ignore(*ptr);
    }
};

template<typename T>
void up1(const T& env)
{
    // something wrong with env

    // zero-initialize index_t 
    typename T::index_t p();

    // A pointer check will be triggered:
    // shallow_checks<T::index_t>::check_is_pointer(p); // - uncomment for error 2
    // error C2100: illegal indirection
    // middle(p); // - uncomment for error 2
}

void test_shallow_check()
{
    client cli;
    up1(cli);
}
