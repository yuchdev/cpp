#include "construct.h"

#include <cassert>
#include <iostream>

// A is defined here, in the .cpp -- forward-declared in the header, where
// it only ever appears by value in a function DECLARATION. That compiles
// with just the forward declaration. Actually calling get_a with a real A
// object needs A's complete definition at the call site; see
// show_forward_declared_by_value() below.
class A
{
public:
    A() = default;
    A(const A&) = default;
    int tag = 7;
};

SimpleStruct::SimpleStruct() : a()
{
    std::cout << "SimpleStruct() default constructor" << std::endl;
}

SimpleStruct::SimpleStruct(int i) : a(i)
{
    std::cout << "SimpleStruct(int) constructor" << std::endl;
}

SimpleStruct::~SimpleStruct()
{
    std::cout << "SimpleStruct destructor" << std::endl;
}

const char SimpleClass::ch_array[] = "qwertyuiop";
const int SimpleClass::int_array[] = { 0, 1, 2, 3 };
int SimpleClass::_counter = 0;

SimpleClass::SimpleClass() : arr()
{
    std::cout << "SimpleClass() default constructor; object count = " << ++_counter << std::endl;
}

// intrnal constructs first, because it is declared first in the class --
// NOT because it is listed first here. Swap the mem-initializer order and
// the construction order stays the same; only -Wreorder would complain.
SimpleClass::SimpleClass(int i) : intrnal(i), arr()
{
    std::cout << "SimpleClass(int) constructor; object count = " << ++_counter << std::endl;
}

SimpleClass::SimpleClass(const SimpleClass& c) : intrnal(c.intrnal.a), arr()
{
    std::cout << "SimpleClass copy constructor; object count = " << ++_counter << std::endl;
}

SimpleClass::~SimpleClass()
{
    std::cout << "SimpleClass destructor; object count = " << --_counter << std::endl;
}

void SimpleClass::out_static_array()
{
    std::cout << ch_array << std::endl;
}

A SimpleClass::get_a(A a)
{
    A a1(a);
    return a1;
}

void create_static(int i)
{
    static SimpleClass stat_c1;
    if (i) {
        static SimpleClass stat_c2;
    }
}

int& return_static()
{
    static int res = 0;
    return res;
}

// The declaration of SimpleClass::get_a compiles with only a forward
// declaration of A in scope. Calling it for real is a different story: the
// caller has to construct an actual A to pass by value, and A's complete
// definition (above, in THIS translation unit) is what makes that
// possible. If this .cpp only had the forward declaration, the call below
// would fail with "invalid use of incomplete type", not at the
// declaration, but right here, at the construction of `arg`.
void show_forward_declared_by_value()
{
    SimpleClass c;
    A arg;
    arg.tag = 99;

    A result = c.get_a(arg);
    assert(result.tag == 99); // passed and returned by value, unchanged
}
