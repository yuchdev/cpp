#include <memory>
#include <vector>
#include <algorithm>
#include <functional>

// TODO: sort out the examples

void show_simple_inheritance()
{
    Employee e1("Jason Statham");
    Manager m1("Chuck Norris", 1, 2000);

    // A derived class can be assigned to a base class 
    // By pointer or reference without explicit conversion
    // The opposite is incorrect
    Employee* pe1 = &m1;
    Employee& re1 = m1;

    // Methods of the base class are available by the pointer or reference
    // The opposite is incorrect
    pe1->show_name();

    // For accessing such methods, explicit cast to a derived class is possible
    static_cast<Manager*>(pe1)->show_level();


    Manager m2("Vin Diesel", 2, 4000);

    m2.show_level();

    // show_name() is also defined in Manager, it replaces the method of the base class
    m2.show_name();

    // Such method of the base class could be called explicitly
    m2.Employee::show_name();

    // Here we see so-called "slicing effect", which usually generates a compiler warning
    Employee e2 = m2;
}

// Show operator overloading and inheritance
void show_operators()
{
    Currency c1 = 1.0;
    Currency c2 = 20;

    Usd r1(1.);
    Usd r2 = r1;

    // Test the base class operators
    Currency c3 = c1;
    c3 = c1 + c2;
    c3 += 2.;
    c3 = c2 + 1;

    // Test the derived class operators
    r2 = r1;
    r1 += r2;
    Usd r3 = r1 + r2;

    // Test the operators between base abd derived
    r3 = r1 + c1;

    // Addition doesn't work this way anymore
    // r1 and r2 are silently converted as a result of the _increasing_ transformation
    // to a base class and sent by parameters to operator+()
    // And it returns Currency by value

    // To resolve such a situation, let's introduce in Currency class
    // overloaded upcasting `operator=()`, as well as a similar constructor

    // This call in functional form: `r1.operator =( c1 )`
    r1 = c1;

    // This call in functional form: `cr3( r1 + r2 )`
    const Currency cr3 = r1 + r2;

    // This call in functional form: `r3( cr3 )`
    Usd r4 = cr3;

    // dynamic_cast is safer tool for downcasting
    if (Usd* r = dynamic_cast<Usd*>(&c3)) {
        r->print();
    }
}

// Show abstraction and virtual functions
void show_abstract()
{

    // These classes are polymorphic
    // and operate a pointer to a table of virtual functions
    circle c;
    rectangle r;
    sphere s;
    cube cb;

    vector<shape*> vs;

    vs.push_back(&c);
    vs.push_back(&r);
    vs.push_back(&s);
    vs.push_back(&cb);

    // Draw all the shapes from the container using the virtual function and algorithm for_each
    for_each(vs.begin(), vs.end(), mem_fun(&shape::draw));
}

// Show the virtual functions use in the constructor and destructor
void show_virtual_construct()
{

    // In the example, you can see that the class constructor&destructor
    // always "class own" virtual function is called

    {
        A_virt a;

        // "virtualize" external function
        cout << a;
    }

    {
        B_virt b;
        cout << b;
    }
}

void show_linkage()
{

    // Inheritance without virtual functions
    Base b;
    Derived d;

    // Only early binding is performed
    // In both cases from an open method of a base class
    // protected method of the base class is called
    int ret = b.call_protected_function(5);	// 11
    ret = d.call_protected_function(5);		// 11

    // Inheritance with virtual functions
    Base_virt bv;
    Derived_virt dv;

    // Late (or dynamic) binding is in effect
    // In the second case, from the open method of the base class
    // a protected method of a derived class is called
    ret = bv.call_protected_function(5);	// 11
    ret = dv.call_protected_function(5);	// 26
}

// Show of co-variant return types
// and default values in virtual functions
void show_virtual_def_values()
{
    Base1 b;
    Derived1 d;
    Base1* b1 = &b;
    Base1* d1 = &d;

    // Function prototype Base1::g(i = 10) 
    b1->f(1);
    b1->f(1.0);
    b1->g();
    A_virt* av1 = b1->pf();

    // Function prototype Derived1::g(i = 10)
    // however, the default parameter is inherited
    d1->g();
    // virual function of the derived class is able to return 
    // a different type of pointer or reference than base,
    // if it could be casted to the base
    A_virt* av2 = d1->pf();
    delete av1;
    delete av2;
}

void show_adapter()
{

    // Base class
    deque_t d;
    d.push_front(1);
    d.push_back(2);
    d.push_back(3);
    int a = d.pop_back();

    // Derived class, connected with base using private inheritance
    adapter_stack_t s;
    s.push(1);
    s.push(2);

    a = s.pop();

    write_only_adapter_stack_t ws;
    ws.push(1);
    ws.push(2);

    another_adapter_stack_t as;
    // only push_back() and pop_back() methods are available
    as.push_back(a);
    as.push_back(1);
#if 0
    // method is not available from the derived class
    as.push_front(2);
#endif
}

void show_abstract_destructor()
{

    // We cannot create an abstract class
    // even though a destructor and one method are explicitly defined
    //v_base vb;

    // But we can create derived, even its vertual desructor is not defined
    v_derived vd;

    // Calling a pure virtual function of the base class
    vd.pure();
}

void show_open_hide()
{
    Base1 b;
    Derived1 d;
    Base1* b1 = &b;
    Base1* d1 = &d;

    b.hide_me();
    b1->hide_me();

    // protected function is called as it opened in base class!
    d1->hide_me();
    d.open_me();
}


// Some tricky problems on inheritance
struct A
{
    A()
    {
        cout << "A ";
    }
    A(const A& a)
    {
        cout << "copy-A ";
    }
    A& operator=(const A& b)
    {
        cout << "=A ";
        return *this;
    }

    // the destructor is non-virtual!
    ~A()
    {
        cout << "~A ";
    }
};

struct B : public A
{

    B()
    {
        cout << "B";
    }
    B(const B& b)
    {
        cout << "copy-B ";
    }
    B& operator=(const B& b)
    {
        cout << "=B ";
        return *this;
    }
    ~B()
    {
        cout << "~B";
    }
};

void fff(A a)
{
}

void ff1()
{
    // The problem is matter of attention to detail
    // The smart pointer passes object B by pointer A
    // But the A class has a *non-virtual* destructor, 
    // so only the base A class destructor woiuld be called
    std::auto_ptr<A> o(new B);
}

void ff2()
{
    // AB~A~A~B~A
    // by adding copy ctor, we get the expected effect with slicing
    // by passing the parameter by value, copy ctor is called
    // for the A-part of class B and creates a temporary object
    // A B copy-A ~A ~B~A

    B b;
    fff(b);
}


int main()
{

    show_simple_inheritance();
    show_abstract();
    show_operators();
    show_virtual_construct();
    show_linkage();
    show_virtual_def_values();
    show_adapter();
    show_abstract_destructor();
    show_open_hide();

    ff1();
    cout << endl;
    ff2();

    return 0;
}
