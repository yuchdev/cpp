#include "codegen.h"

#include <cassert>
#include <iostream>
#include <typeinfo>

using std::cout;
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
    assert(cpp_gen != nullptr);

    // Crosscasting: requires dynamic_cast and a polymorphic (virtual-base)
    // hierarchy; a static_cast could not do this.
    php_code_generator* php_gen = dynamic_cast<php_code_generator*>(cpp_gen);
    assert(php_gen != nullptr);

    // Upcasting again, to the shared virtual base.
    code_generator_base* base = dynamic_cast<code_generator_base*>(php_gen);
    assert(base != nullptr);

    // Downcasting: back to the most-derived type.
    code_formatter* cf = dynamic_cast<code_formatter*>(base);
    assert(cf == &formatter); // same complete object, recovered via 3 casts

    // dynamic_cast<void*> finds the address of the complete (most-derived)
    // object, regardless of which sub-object pointer you start from.
    void* cf_void = dynamic_cast<void*>(cf);
    assert(cf_void == static_cast<void*>(&formatter));

    // finalizer is not part of this hierarchy at all: the cast correctly
    // fails and returns nullptr for a pointer target.
    finalizer* fin = dynamic_cast<finalizer*>(cf);
    assert(fin == nullptr);

    // The same failure, cast to a REFERENCE instead of a pointer, throws
    // std::bad_cast instead of returning anything -- references cannot be
    // null, so failure has no "empty" value to return.
    bool threw_bad_cast = false;
    try {
        finalizer& fin_ref = dynamic_cast<finalizer&>(formatter);
        (void)fin_ref;
    }
    catch (const std::bad_cast&) {
        threw_bad_cast = true;
    }
    assert(threw_bad_cast);
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

    // as_base's STATIC type is code_generator_base&, but typeid reports
    // its DYNAMIC type -- proven by comparing against typeid() of each
    // candidate type directly, not by reading printed (and
    // implementation-defined) name() strings.
    assert(ti_dynamic_type == typeid(code_formatter));
    assert(ti_dynamic_type != typeid(code_generator_base));

    // typeid on a dereferenced null polymorphic pointer has nothing to
    // report: it throws std::bad_typeid instead of crashing.
    bool threw_bad_typeid = false;
    try {
        code_generator_base* null_ptr = nullptr;
        const type_info& ti = typeid(*null_ptr);
        (void)ti;
    }
    catch (const std::bad_typeid&) {
        threw_bad_typeid = true;
    }
    assert(threw_bad_typeid);

    // type_info::before() gives an unspecified-but-consistent ordering,
    // useful for storing type_info/type_index values in ordered
    // containers; it carries no meaning beyond "some total order". The
    // only thing actually checkable about it from outside is that it's
    // a strict, consistent ordering: exactly one of a<b, b<a, a==b holds.
    bool a_before_b = ti_static_type.before(ti_dynamic_type);
    bool b_before_a = ti_dynamic_type.before(ti_static_type);
    bool equal = (ti_static_type == ti_dynamic_type);
    assert((a_before_b + b_before_a + equal) == 1);
}

int main()
{
    show_casts();
    show_typeid();
    return 0;
}
