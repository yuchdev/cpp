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

// Demonstration of inheritance finalization in pre C++11
void show_final()
{

    class more : public finalizer
    {
        int i;
    };
    // The following instructions do not compile:
    //more m;
    //more* mp = new more();
    //delete mp;

    // You cannot create a derived class neither statically nor dynamically
    //error C2248: 'finalizer::finalizer' : cannot access private member declared in class 'finalizer
}

// Multimethod demonstration
void show_multimethod()
{

    // method is addressed depending on the caller class and the type of argument passed
    // Works both inside and outside the hierarchy
    derived_a da;
    derived_b db;

    da.multiop(da);
    da.multiop(db);

    db.multiop(da);
    db.multiop(db);

    db.multiop(dynamic_cast<base&>(da));
    db.multiop(dynamic_cast<base&>(db));
}

// Demonstration of pointers to members
void show_member_pointer(member_pointer* p)
{

    // 1. Pointer to a non-static function-class member is a special case
    // It cannot be assigned to a regular pointer or to a function pointer.
    // Usually it is inpmeneted as a struct, because, besides the function pointer itself, 
    // it can contain offset of `this` pointer (especially in case of multiple inheritance)

    // 2. In addition, if the function is virtual, instead of the address it stores nullptr, 
    // but stores the index of the virtual function in VTBL instead

    // 3. Member pointer could be treated os an OFFSET from the class beginning
    // (or its polymorph part)

    // 4. Thus, a structure is used to store a member pointer (not defined by the standard, but used in practice)
    // * function address or NULL if method is virtual
    // * offset of the `this` pointer
    // * index, if the function is virtual

    // A pointer to a member can be obtained by applying 
    // the dereferencing (&) operation to a fully qualified class name
    p_mem s = &member_pointer::start;
    p_mem ss = &member_pointer::stop;

    // direct call
    p->start();

    // indirect call through the method pointer
    member_derived* d = static_cast<member_derived*>(p);
    (d->*s)();
    (p->*ss)();

    member_derived md;
    (md.*s)();

    static_member_pointer sp = &member_derived::static_init;

    // A pointer to a function can be used when the function name is unknown
    // A pointer to a member of a base class can be assigned to a pointer to a derived member.
    // Pointers to a method can be virtual and non-virtual.
    // In the virtual case, they define the method offset in the vtbl

    // The pointer to a static member in fact is a pointer to a plain function,
    // because it is not connected to `this`
}

void show_new_delete()
{
    //member_derived* m = new member_derived();
    //delete m;
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
