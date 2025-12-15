#include <iostream>
#include <typeinfo>

#include "virtual.h"
#include "simple.h"
#include "final.h"
#include "multimethod.h"
#include "member_pointer.h"

#pragma pack(1)

using std::cout;
using std::endl;
using std::type_info;

// Demonstration of dynamic type and RTTI
void show_typeid_rtti()
{
    code_formatter formatter(1);
    formatter.generate(cpp_lang);

    // for upcasting or crosscasting using dynamic_cast<> polymorphism required

    // upcasting
    cpp_code_generator* cpp_gen = dynamic_cast<cpp_code_generator*>(&formatter);

    // crosscasting
    // after crosscasting the resulting type does not have to be polymorphic
    php_code_generator* php_gen = dynamic_cast<php_code_generator*>(cpp_gen);

    // upcasting
    code_generator_base* bs = dynamic_cast<code_generator_base*>(php_gen);

    // downcasting
    code_formatter* cf = dynamic_cast<code_formatter*>(bs);
    // if the dynamic transformation is ambiguous
    // (in case of several copies of the base class)
    // it returns `nullptr` or `bad_cast` is thrown

    // a void* conversion with dynamic_cast is used to find 
    // the starting address of polymorphic type (in a very low-level code!)
    void* cfvoid = dynamic_cast<void*>(cf);

    // If conversion is forbidden, a `nullptr` is returned
    finalizer* fin = dynamic_cast<finalizer*>(cf);
    if (fin == 0)
        cout << "Wrong dynamic cast" << endl;

    // If it fails while casting to a reference, a bad_cast exception is thrown
    try {
        finalizer& fin1 = dynamic_cast<finalizer&>(formatter);
    }
    catch (const std::bad_cast& e) {
        cout << e.what() << endl;
    }

    // Type information:

    // By type name
    const type_info& ti1 = typeid(finalizer);
    cout << ti1.name() << endl;

    // By object name
    const type_info& ti2 = typeid(formatter);
    cout << ti2.name() << endl;

    // For the basic type
    const type_info& ti3 = typeid(int);
    cout << ti3.name() << endl;

    // The symbolic representation of type information is implementation-defined
    // A bad_typeid exception is thrown when trying to get a null polymorphic pointer type
    try {
        code_generator_base* bs = 0;
        const type_info& ti = typeid(bs);
        cout << ti.name() << endl;
    }
    catch (const std::bad_typeid& e) {
        cout << e.what() << endl;
    }

    // the `before()` method is required for sorting
    int b = ti1.before(ti2);
    b = ti1.before(ti3);
}

int main()
{
    show_simple();
    show_vitrual();
    show_sizes();
    show_final();
    show_typeid_rtti();
    show_multimethod();

    member_derived md;
    show_member_pointer(&md);
    show_new_delete();

    return 0;
}
