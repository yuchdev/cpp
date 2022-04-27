#include "construct.h"
// Демонстрация примера из Саттера1 гл.4
#include "class.h"

#include <iostream>

// intern
intern::intern() : a()
{
    std::cout << "intern() default constructor" << std::endl;
}

intern::intern(int i) : a(i)
{
    std::cout << "intern(int) constructor" << std::endl;
}

// intern
intern::~intern()
{
    std::cout << "intern destructor" << std::endl;
}


// construct
// Initialize static arrays first
const char construct::ch_array[] = "qwertyuiop";
const int construct::int_array[] = { 0,1,2,3 };
int construct::_counter = 0;

construct::construct() : arr()
{
    std::cout << "construct() default constructor; object count = " << ++_counter << std::endl;
}

// Call intern() constructor first
construct::construct(int i) : intrn(i), arr()
{
    std::cout << "construct(int) constructor; object count = " << ++_counter << std::endl;
}

construct::construct(const construct& c) : intrn(c.intrn.a), arr()
{
    std::cout << "construct copy constructor; object count = " << ++_counter << std::endl;
}

construct::~construct(void)
{
    std::cout << "construct destructor; object count = " << --_counter << std::endl;
}

void construct::out_static_array()
{
    // Only static method may access static members
    // You can call it for specific class enstance, or independingly, like:
    // construct::out_static_array() and c.construct::out_static_array();
    std::cout << ch_array << std::endl;
}

// All passed-by-value objects declared through the forward declaration in the construct.h header
A construct::get_a(A a)
{
    A a1(a);
    a1.set_a5(5);
    return a1;
}

void create_static(int i)
{
    static construct stat_c1;
    if (i) {
        static construct stat_c2;
    }
}

int& return_static()
{
    static int res = 0;
    return res;
}

