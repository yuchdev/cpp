#include "codegen.h"

#include <iostream>
#include <typeinfo>

using std::cout;
using std::endl;
using std::type_info;

// Upcasting, crosscasting, and downcasting around the virtual diamond.
// Crosscasting (cpp_code_generator* -> php_code_generator*) is the
// interesting case: those two types are siblings, not related to each
// other by inheritance, so there is no compile-time path between them.
// dynamic_cast finds the complete object at run time, via RTTI, and
// returns a pointer to the sibling branch if (and only if) it actually
// exists in that object.
void show_casts()
{
    code_formatter formatter(1);
    formatter.generate(cpp_lang);

    // Upcasting: always safe, could even be a static_cast here, since
    // code_formatter publicly derives from cpp_code_generator.
    cpp_code_generator* cpp_gen = dynamic_cast<cpp_code_generator*>(&formatter);

    // Crosscasting: requires dynamic_cast and a polymorphic (virtual-base)
    // hierarchy; a static_cast could not do this.
    php_code_generator* php_gen = dynamic_cast<php_code_generator*>(cpp_gen);

    // Upcasting again, to the shared virtual base.
    code_generator_base* base = dynamic_cast<code_generator_base*>(php_gen);

    // Downcasting: back to the most-derived type.
    code_formatter* cf = dynamic_cast<code_formatter*>(base);

    // dynamic_cast<void*> finds the address of the complete (most-derived)
    // object, regardless of which sub-object pointer you start from.
    void* cf_void = dynamic_cast<void*>(cf);
    cout << "dynamic_cast<void*> address matches &formatter: "
         << (cf_void == static_cast<void*>(&formatter)) << '\n';

    // finalizer is not part of this hierarchy at all: the cast correctly
    // fails and returns nullptr for a pointer target.
    finalizer* fin = dynamic_cast<finalizer*>(cf);
    if (fin == nullptr)
        cout << "dynamic_cast to an unrelated type correctly yields nullptr\n";

    // The same failure, cast to a REFERENCE instead of a pointer, throws
    // std::bad_cast instead of returning anything -- references cannot be
    // null, so failure has no "empty" value to return.
    try {
        finalizer& fin_ref = dynamic_cast<finalizer&>(formatter);
        (void)fin_ref;
    }
    catch (const std::bad_cast& e) {
        cout << "bad_cast on reference cast: " << e.what() << '\n';
    }
}

// typeid() reports the DYNAMIC type of a polymorphic glvalue, not the
// static type of the expression used to reach it -- that is the whole
// point of RTTI.
void show_typeid()
{
    code_formatter formatter(2);
    code_generator_base& as_base = formatter;

    const type_info& ti_static_type = typeid(finalizer);
    const type_info& ti_dynamic_type = typeid(as_base); // reports code_formatter, not code_generator_base
    const type_info& ti_builtin = typeid(int);

    cout << "typeid(finalizer).name()        = " << ti_static_type.name() << '\n';
    cout << "typeid(as_base).name()          = " << ti_dynamic_type.name() << '\n';
    cout << "typeid(int).name()              = " << ti_builtin.name() << '\n';

    // typeid on a dereferenced null polymorphic pointer has nothing to
    // report: it throws std::bad_typeid instead of crashing.
    try {
        code_generator_base* null_ptr = nullptr;
        const type_info& ti = typeid(*null_ptr);
        (void)ti;
    }
    catch (const std::bad_typeid& e) {
        cout << "bad_typeid on a null polymorphic dereference: " << e.what() << '\n';
    }

    // type_info::before() gives an unspecified-but-consistent ordering,
    // useful for storing type_info/type_index values in ordered
    // containers; it carries no meaning beyond "some total order".
    bool b = ti_static_type.before(ti_dynamic_type);
    (void)b;
}

int main()
{
    show_casts();
    show_typeid();
    return 0;
}
