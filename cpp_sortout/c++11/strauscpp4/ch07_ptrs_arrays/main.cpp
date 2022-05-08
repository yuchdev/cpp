#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

/*

New features:
* Null pointer keyword (7.2.2)
* Rvalue references (enabling move semantics)(7.7.2)
* Unicode characters (6.2.3.2, 7.3.2.2)
* Raw string literals (7.3.2.1)
* Scoped and strongly typed enums: enum class (2.3.3, 8.4.1)
* Generalized POD (8.2.6)
* Generalized unions (8.3.1)
* A fixed-sized continuous sequence container: array (8.2.4, 34.2.1)
* Generalized and guaranteed constant expressions: constexpr (2.2.3, 10.4, 12.1.6)
* Range-for-statement (2.2.5, 9.5.1)

Examples:
1. nullptr!= NULL; Rvalue references, move.
2. Unicode and raw strings, backslash, LR
3. Union tag
4. Explicit enum class base, plane and unnamed enums
5. Literal type (constexpr constructor)

*/

namespace cpp4 {

class test_me
{
public:
    void point_to_me() {}
};

void point_to_me_static() {}

// A string literal is statically allocated so that it is safe to return one from a function
constexpr const char* error_message(int i) 
{
    return "range error";
}

// Basically, constexpr's role is to enable and ensure compile-time evaluation, 
// whereas const's primary role is to specify immutability in interfaces

// "perfect swap" (almost) 
template<class T>
void swap(T& a, T& b)
{ 
    // Since move(x) does not move x (it simply produces an rvalue reference to x), 
    // it would have been better if move() had been called rval(), 
    // but by now move() has been used for years

    // move from a  
    T tmp{ std::move(a) };

    //move from b 
    a = std::move(b);

    //move from tmp
    b = std::move(tmp);
}

}

void show_ptrs_refs()
{
    // A pointer to any type of object can be assigned to a variable of type void*, 
    // but a pointer to to member(20.6) cannot

    // pointer to function
    void(*static_func_ptr)() = &cpp4::point_to_me_static;

    void(cpp4::test_me::* method_ptr)() = &cpp4::test_me::point_to_me;

    // ? better not to do, ensure in Standard
    void* v1 = static_func_ptr;

    //void* v2 = method_ptr;

    // assign nullptr - OK
    static_func_ptr = nullptr;
    method_ptr = nullptr;

    // assign 0 - OK
    static_func_ptr = 0;
    method_ptr = 0;

    // An rvalue reference refers to a temporary object
    std::vector<int> vv1{ 1,2,3 };
    std::vector<int> vv2{ 4,5,6 };
    cpp4::swap(vv1, vv2);

    // The compiler uses v and v+N as begin(v) and end(v) for a built-in array T v[N]
    int arr[] = { 7,2,5,9,5,3,5,8,0,3 };
    std::sort(std::begin(arr), std::end(arr));
}

void show_string_literals()
{

    // Raw string literals use the R"(ccc)" notation for a sequence of characters ccc
    const char* qs = R"(quoted string)"; // the string is "quoted string"
    const char* complicated = R"("('(?:[^\\\\']|\\\\.)*'|\"(?:[^\\\\\"]|\\\\.)*\")|")";
    const char* with_returns = R"(atatat
tatatat)";

    std::cout << qs << std::endl;
    std::cout << complicated << std::endl;
    std::cout << with_returns << std::endl;

    // Unless you work with regular expressions, raw string literals are probably just a curiosity

    // Similarly, a string with the prefix LR, such as LR"(angst)", is a raw string
    const wchar_t* long_complicated = LR"("('(?:[^\\\\']|\\\\.)*'|\"(?:[^\\\\\"]|\\\\.)*\")|")";

    std::wcout << long_complicated << std::endl;
}

namespace cpp4 {

class tagged_union
{
public:
    
    struct bad_tag {};

    void set_integer(int i) 
    {
        tag_ = tag::integer;
        i_ = i;
    }

    void set_pointer(int* pi) 
    {
        tag_ = tag::pointer;
        pi_ = pi;
    }

    int integer() const 
    {
        if (tag_ != tag::integer) {
            throw bad_tag();
        }
        return i_;
    }

    int* pointer() const 
    {
        if (tag_ != tag::pointer) {
            throw bad_tag();
        }
        return pi_;
    }

private:
    enum class tag { integer, pointer };
    tag tag_;
    union 
    {
        int i_;
        int* pi_;
    };
};

}

void show_union_tags() {
    cpp4::tagged_union tu;
    int i = 0;
    tu.set_integer(1);
    tu.set_pointer(&i);
}

void show_enum_classes() {

    // The underlying type must be one of the signed or unsigned integer types (6.2.4)
    // the default is int. We could be explicit about that:

    enum class warning1 : long {green, yellow, red};

    enum class warning2 : unsigned char { green, yellow, red };

    long l = static_cast<long>(warning1::green);
    unsigned char c = static_cast<unsigned char>(warning2::green);

    // 
    enum plain : long { plain1, plain2, plain3 };
    plain p = plain1;

    // Todo plain and class name resolution

    // An enum is a user-defined type, so we can define the | and & operators

    // A plain enum can be unnamed, e.g.:
    enum { arrow_up = 1, arrow_down, arrow_sideways };
}

namespace cpp4 {

// A class with a constexpr constructor is called a literal type
// To be simple enough to be constexpr, a constructor must have an empty body 
// and all members must be initialized by potentially constant expressions. For example:
struct point
{
    constexpr point(double x, double y, double z) :x_{x}, y_{y}, z_{z} {}
private:
    double x_, y_, z_;
};

}

void show_literal_types()
{
    // A constant expression is an expression that a compiler can evaluate
    constexpr cpp4::point p{ 1.,2.,3. };
    constexpr cpp4::point parr[] = { { 1.,2.,3. } ,{ 1.,2.,3. } ,{ 1.,2.,3. } };
}

int main() 
{
    show_ptrs_refs();
    show_string_literals();
    show_union_tags();
    show_enum_classes();
    show_literal_types();

    return 0;
}
