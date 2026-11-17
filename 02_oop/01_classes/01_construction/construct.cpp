#include <iostream>
#include "construct.h"

// Class A defined in *.cpp, and forward-declared in header
// However, we can use A in the header for passing and returning by value
class A {};

// SimpleStruct
SimpleStruct::SimpleStruct() : a()
{
    std::cout << "SimpleStruct() default constructor" << std::endl;
}

SimpleStruct::SimpleStruct(int i) : a(i)
{
    std::cout << "SimpleStruct(int) constructor" << std::endl;
}

// SimpleStruct
SimpleStruct::~SimpleStruct()
{
    std::cout << "SimpleStruct destructor" << std::endl;
}


// SimpleClass
// Initialize static arrays first
const char SimpleClass::ch_array[] = "qwertyuiop";
const int SimpleClass::int_array[] = { 0,1,2,3 };
int SimpleClass::_counter = 0;

SimpleClass::SimpleClass() : arr()
{
    std::cout << "SimpleClass() default constructor; object count = " << ++_counter << std::endl;
}

// Call SimpleStruct() constructor first
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
    // Only static method may access static members
    // You can call it for specific class instance, or independently, like:
    // SimpleClass::out_static_array() and c.SimpleClass::out_static_array();
    std::cout << ch_array << std::endl;
}

// Passed-by-value objects could be declared through the forward declaration
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

