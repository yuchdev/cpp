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

// Demonstration of virtual multiple inheritance
void show_vitrual()
{

    size_t sz = 0;

    // The object contains 3 pointers to the base class, the base class itself and the derived class
    code_formatter formatter(1);

    formatter.generate(cpp_lang);
    sz = sizeof(formatter);
    // 36 = ( base(int+vptr) = 8; generators(3 * (vfptr+vtbl)) = 24 ; last_class(vprt) = 4 )

    cout << "code_formatter size = " << sz << endl;
    sz = sizeof(code_generator_base);	// 8 (int + vtbl)
    cout << "code_generator_base size = " << sz << endl;
    sz = sizeof(cpp_code_generator);	// 20 (4 + 2*(vtbl+vfptr) )
    cout << "cpp_code_generator size = " << sz << endl;


    // Inheritance is diamond-shaped, but one of the classes is inherited non-virtually
    Final f;
    sz = sizeof(Base);		// 4 (vtbl)
    sz = sizeof(Derived1);	// 4 (vtbl)
    sz = sizeof(Derived2);	// 12 (2*vtbl + vfptr)
    sz = sizeof(f);			// 16
}

void show_sizes()
{

    size_t sz = 0;

    {
        // 4 empty classes are linked by diamond inheritance
        class A {};
        class B1 : public A {};
        class B2 : public A {};
        class C : public B1, public B2 {};

        // each class size == 1
        sz = sizeof(A);
        sz = sizeof(B1);
        sz = sizeof(B2);
        sz = sizeof(C);
    }


    {
        // add a 2 byte field to the base class
        class A { short a; };
        class B1 : public A {};
        class B2 : public A {};
        class C : public B1, public B2 {};

        sz = sizeof(A);	// 2
        sz = sizeof(B1);// 2 (from the base)
        sz = sizeof(B2);// 2 (from the base)
        sz = sizeof(C);	// 4 (2 copies of base)
    }


    {
        // make one of the inheritance branches virtual
        class A { short a; };
        class B1 : virtual public A {};
        class B2 : public A {};
        class C : public B1, public B2 {};

        // now the lower class still has 2 copies of the base class,
        // but a pointer to virtual inheritance was added
        sz = sizeof(A);	// 2
        sz = sizeof(B1);// 6 (2 from the base + pointer)
        sz = sizeof(B2);// 2 (from the base)
        sz = sizeof(C);	// 8 (2 from the base of non-virtual branch, 6 of virtual one)
    }


    {
        // make both branches of inheritance virtual
        class A { short a; };
        class B1 : virtual public A {};
        class B2 : virtual public A {};
        class C : public B1, public B2 {};

        // now the lower class has 1 copy of the base class and 2 vtbl pointers
        sz = sizeof(A);	// 2
        sz = sizeof(B1);// 6 (2 from the base + pointer)
        sz = sizeof(B2);// 6 (2 from the base + pointer)
        sz = sizeof(C);	// 10 (2 from the base + 2 pointers)
    }

    {
        // add another branch of virtual inheritance
        class A { short a; };
        class B1 : virtual public A {};
        class B2 : virtual public A {};
        class B3 : virtual public A {};
        class C : public B1, public B2, public B3 {};

        // now there are 3 pointers in the resulting class
        sz = sizeof(A);		// 2
        sz = sizeof(B1);	// 6 (2 from the base + pointer)
        sz = sizeof(B2);	// 6 (2 from the base + pointer)
        sz = sizeof(B3);	// 6 (2 from the base + pointer)
        sz = sizeof(C);		// 14 (2 from the base + 3 pointers)
    }

    {
        // add a virtual destructor to the base class
        class A
        {
        public:
            short a;
            virtual ~A() {}
        };
        class B1 : virtual public A {};
        class B2 : virtual public A {};
        class B3 : virtual public A {};
        class C : public B1, public B2, public B3 {};

        // now the size of the base class is 4 more because of another vtbl
        sz = sizeof(A);	// 6 (2 data  + 4 of vtbl)
        sz = sizeof(B1);// 10 (6 from the base + 4 of vtbl)
        sz = sizeof(B2);// 10 (6 from the base + 4 of vtbl)
        sz = sizeof(B3);// 10 (6 from the base + 4 of vtbl)
        sz = sizeof(C);	// 18 (6 from the base + 3*4 of vtbl)
    }
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
