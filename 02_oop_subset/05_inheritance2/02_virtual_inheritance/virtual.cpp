#include "virtual.h"
#include <iostream>
using std::cout;
using std::endl;

#pragma pack(1)

//--------------------------------------
code_generator_base::code_generator_base(int i) : _i(i)
{
    cout << "code_generator_base() construction" << endl;
}

code_generator_base::~code_generator_base()
{
    cout << "~code_generator_base() destruction" << endl << endl;
}

void code_generator_base::generate(lang l)
{
    cout << "code_generator_base::generate()" << endl;
}
//--------------------------------------
cpp_code_generator::cpp_code_generator(int i) : code_generator_base(i)
{
    cout << "cpp_code_generator() construction" << endl;
}

cpp_code_generator::~cpp_code_generator()
{
    cout << "~cpp_code_generator() destruction" << endl << endl;
}

void cpp_code_generator::print()
{
    cout << "cpp_code_generator()::print()" << endl;
}

void cpp_code_generator::f1()
{
    cout << "cpp_code_generator()::f1()" << endl;
}

//--------------------------------------
java_code_generator::java_code_generator(int i) : code_generator_base(i)
{
    cout << "java_code_generator() construction" << endl;
}

java_code_generator::~java_code_generator()
{
    cout << "~java_code_generator() destruction" << endl << endl;
}

void java_code_generator::f1()
{
    cout << "java_code_generator()::f1()" << endl;
}

//--------------------------------------
php_code_generator::php_code_generator(int i) : code_generator_base(i)
{
    cout << "php_code_generator() construction" << endl;
}

php_code_generator::~php_code_generator()
{
    cout << "~php_code_generator() destruction" << endl << endl;
}

void php_code_generator::f1()
{
    cout << "php_code_generator()::f1()" << endl;
}

//--------------------------------------
// In virtual inheritance, the constructor of the base class 
// is being called by the last derived class in the hierarchy
code_formatter::code_formatter(int i) :
    cpp_code_generator(i)
    , java_code_generator(i)
    , php_code_generator(i)
    , code_generator_base(i)
{
    cout << "code_formatter() construction" << endl;
}

code_formatter::~code_formatter()
{
    cout << "~code_formatter() destruction" << endl << endl;
}

void code_formatter::generate(lang l)
{
    code_generator_base::generate(l);
    cout << "code_formatter::generate()" << endl;
}

//--------------------------------------
void Final::f_base()
{
    cout << "Final::f_base()" << endl;
}

void Final::f1()
{
    cout << "Final::f1()" << endl;
}

void Final::f2()
{
    cout << "Final::f2()" << endl;
}
