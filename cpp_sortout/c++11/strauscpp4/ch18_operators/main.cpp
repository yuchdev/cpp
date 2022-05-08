#include <iostream>

using namespace std;

/*

New features:
* Explicit conversion operators (18.4.2)
* User-defined literals (19.2.6)

Examples:
1. Delete standard operators (18.2.2)
2. Complex & literals (18.3.4)
3. Explicit conversion type (18.4.2)
4. User-defined Literals (19.2.6)
5. Template literal operator (19.2.6)
6. Template friend (19.4)
7. Friend lookup (19.4.1)


*/

//1. Delete standard operators(18.2.2)
// The operators = (assignment), &(address - of), and, (sequencing) have predefined meanings
// when applied to class objects. These predefined meanings can be eliminated(deleted)
namespace cpp4 {

class some
{
public:
    // delete some standard operations
    void operator=(const some&) = delete; 
    void operator&() = delete;
    void operator,(const some&) = delete;
};

} // namespace cpp4 

void show_deleted_operations()
{
    cpp4::some a;
    cpp4::some b;

    // all these operations not possible
    //a = b;
    //some* pa = &a;
    //a,b;
}


//2. Complex & literals(18.3.4)
// It is possible to go further and introduce a user-defined literal in support of our type
namespace cpp4 {

class complex
{
public:
    constexpr complex(long double re, long double im) :re_{re}, im_{im} {}
private:
    long double re_, im_;
};

// imaginary literal (must start from underscore, all others are reserved)
constexpr complex operator"" _i(long double d)
{
    // complex is a literal type }
    return{ 0, d };
}

/*
The declaration of a literal operator shall have a parameter-declaration-clause equivalent to one of the following:
(Per paragraph 13.5.8./3 of the C++11 Standard)

const char*
unsigned long long int
long double
char
wchar_t
char16_t
char32_t
const char*, std::size_t
const wchar_t*, std::size_t
const char16_t*, std::size_t
const char32_t*, std::size_t
*/

} // namespace cpp4 

void show_operator_suffix()
{
    using namespace cpp4;
    complex c1{ 1.0, 2.0 };
    complex c2 = 1.0_i;
}

//3. Explicit conversion type(18.4.2)
// It is possible to declare a conversion operator explicit and have it apply only for direct initialization
namespace cpp4 {

template <typename T>
class smart_ptr
{
public:
    smart_ptr(T* p) :p_{ p } {}

    ~smart_ptr() 
    {
        delete p_;
    }

    // non-copyable
    smart_ptr(const smart_ptr&) = delete;
    smart_ptr& operator=(const smart_ptr&) = delete;

    // non-movable, for simplicity
    smart_ptr(const smart_ptr&&) = delete;
    smart_ptr& operator=(const smart_ptr&&) = delete;

    // used only for the direct init
    explicit operator bool() const noexcept
    {
        return p_ ? true : false;
    }

private:
    T* p_;
};

} // namespace cpp4 

void show_explicit_convert_operator()
{
    cpp4::smart_ptr<int> p1(new int{ 1 });
    
    // // OK: we want this use
    if (p1) {
        std::cout << "explicit operator bool()\n";
    }

    // error ; suspicious use
    // bool b = p1;

    // // error; we definitly don't want this
    // int x = 1 + p1;
}

//4. User-defined Literals(19.2.6)
//5. Template literal operator (19.2.6)
// A template literal operator is a literal operator that takes its argument 
// as a template parameter pack, rather than as a function argument

#if 0
// The variadic template techniques can be disconcerting, but it is the only way 
// of assigning nonstandard meanings to digits at compile time
// Ex: ternary digits
namespace cpp4 {

// x to the nth power for n>=0 
constexpr int ipow(int x, int n)
{
    return (n > 0) ? x*ipow(x, n-1) : 1;
}

// handle the single ternary digit case  
template<char c> 
constexpr int b3_helper()
{
    static_assert(c < '3', "not a ternary digit");
    return c; 
}

// peel off one ternary digit 

template<char c, char... tail> 
constexpr int b3_helper()
{
    static_assert(c < '3', "not a ternary digit");
    return ipow(3, sizeof...(tail))*(c-'0') + b3_helper(tail...); 
}

// base 3, i.e., ternary
template<char... chars>
constexpr int operator""_b3()
{
    return b3_helper(chars...);
}

} // namespace cpp4


void show_ternary_digits()
{
    using namespace cpp4;

    // changing meaning
    constexpr int i1 = 201_b3;
}
#endif

//6. Template friend (19.4)
namespace cpp4 {

template<typename T> 
class X
{
    friend T;
    // friend class T; redundant 'class'
    int x = 0;
};

class your_fiend
{
public:
    your_fiend(X<your_fiend>& x1) {
        // access to private
        std::cout << x1.x << std::endl;
    }
};

} // namespace cpp4

void show_template_friend()
{
    cpp4::X<cpp4::your_fiend> x;
    cpp4::your_fiend f(x);
}

//7. Friend lookup(19.4.1)
// A friend must be previously declared in an enclosing scope or defined in the non-class scope 
// immediately enclosing the class that is declaring it to be a friend. 
// Scopes outside the innermost enclosing namespace scope are not considered for a name 
// first declared as a friend (iso.7.3.1.2). Consider a technical example

// Thus, a friend function should be explicitly declared in an enclosing scope
// or take an argument of its class or a class derived from that

class c1 {};
void f1()
{
    // we are friend but the class defined below
    // error C2039: 'find_friends': is not a member of 'cpp4'
    //cpp4::find_friends f;
    //f.x = 1;
}

namespace cpp4 {

class c2 {};
void f2() 
{
    // we are friend but the class defined below
    // error C2039: 'find_friends': is not a member of 'cpp4'

    //cpp4::find_friends f;
    //f.x = 1;
}

class find_friends 
{
public:

    // OK: declared previously in global namespace
    friend class c1;
    friend void f1();

    // OK: declared previously in the same namespace
    friend class c2;
    friend void f2();

    // OK: declared just in the same namespace
    friend class c3;
    friend void f3();

    // Wrong: declared but will never be found, as they are defined below 
    // and not in the same namespace
    friend class c4;
    friend void f4();

private:
    int x = 0;
};
// also could be found if a class a param of a friend function

class c3 {};
void f3() 
{
    // ok
    cpp4::find_friends f;
    f.x = 1;
}

} // namespace cpp4 

class c4 {};
void f4()
{
    cpp4::find_friends f;

    // not a friend as expected to be in the same namespace
    // 'cpp4::find_friends::x': cannot access private member declared in class 'cpp4::find_friends'
    //f.x = 1;
}

void show_find_friends()
{
    f1();
    cpp4::f2();
    cpp4::f3();
}


int main() 
{
    show_deleted_operations();
    show_operator_suffix();
    show_explicit_convert_operator();
    // TODO:
    //show_ternary_digits();
    show_template_friend();
    show_find_friends();
    return 0;
}
