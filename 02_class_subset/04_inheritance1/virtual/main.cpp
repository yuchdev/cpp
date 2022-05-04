#include "abstract.h"
#include "classes.h"
#include "currency.h"
#include "adapter.h"
#include "virtual.h"

#include <memory>
#include <vector>
#include <algorithm>
#include <functional>

// TODO: virtual specifier
// https://en.cppreference.com/w/cpp/language/virtual

// Dynamic&Static Binding
// https://tekslate.com/static-dynamic-binding-virtual-function-c-language


void show_simple_inheritance(){
    Employee e1("Helmut Komatoza");
    Manager m1("German Skotobaza", 1, 2000);

    // �� ��������� ��� ������ ����������� ����� ����� ����
    // �������� �������� ��� ������ ����������
    Employee* pe1 = &m1;
    Employee& re1 = m1;

    // �������� ������� � ������� ������!
    //Output: cannot convert from 'Employee *__w64 ' to 'Manager *'
    //Manager* pm2 = &e1;

    // ������ �������� ������ �������� �� ���������
    pe1->show_name();
    // � ������������ - ����������
    //pe1->show_level(); error C2039: 'show_level' : is not a member of 'Employee'

    // � ���� ������ ��������� ����� ���������� � ������������ ������
    static_cast<Manager*>(pe1)->show_level();

    // ------------------------------
    Manager m2("Vatslav Melioratsya", 2, 4000);

    m2.show_level();
    // show_name() ����� ��������� � Manager, �� �������� ����� �������� ������
    m2.show_name();
    // ���� ����� ���� ������� ����� �������� �������
    m2.Employee::show_name();

    // �������� - ��� ������ ������. ���������� ���� �� �������������.
    //Employee e2 = m2;
}

// --- ������������ ���������� ���������� � ������������ ---
void show_operators(){
    currency_t c1 = 1.0;
    currency_t c2 = 20;

    roubles_t r1(1.);
    roubles_t r2 = r1;

    // --- �������� ��������� �������� ������ ---
    currency_t c3 = c1;
    c3 = c1 + c2;
    c3 += 2.;
    c3 = c2 + 1;

    // --- �������� ��������� ������������ ������ ---
    r2 = r1;
    r1 += r2;
    roubles_t r3 = r1 + r2;

    // --- �������� ��������� � ����� ������� ---
    r3 = r1 + c1;

    // � �������� ��� �� ��������
    // error C2440: 'initializing' : cannot convert from 'currency_t' to 'roubles_t'
    //roubles_t r3 = r1 + r2;

    // r1 � r2 � ���������� _�����������_ �������������� ����� ���������
    // � �������� ������ � ���������� ����������� � operator+()
    // � �� ���������� currency_t �� ��������

    // ����� ����� �� ��������,	������ � roubles_t
    // ������������� ������������� operator=() - ��. h-����
    // � ����� ����������� �����������!
    r1 = c1; // �.�. � �������������� ����� r1.operator =( c1 );
    const currency_t cr3 = r1 + r2; // �.�. ����� ������������ cr3( r1 + r2 );
    roubles_t r4 = cr3;		// r3( cr3 )

    // downcasting ���������� ������ ��� ������ dynamic_cast
    if (roubles_t* r = dynamic_cast<roubles_t*>(&c3)){
        r->print();
    }
}

// --- ������������ ���������� � ����������� ������� ---
void show_abstract() {

    // ������ ������ �������� ������������
    // � ������ ������ ������� ����������� �������
    circle c;
    rectangle r;
    sphere s;
    cube cb;

    vector<shape*> vs;

    vs.push_back(&c);
    vs.push_back(&r);
    vs.push_back(&s);
    vs.push_back(&cb);

    // ������ ��� ������ �� ���������� � ������� ����������� ������� �
    // ��������� for_each()
    for_each(vs.begin(), vs.end(), mem_fun(&shape::draw));
}

// --- ������������ ������������� ����������� ������� � ������������-����������� ---
void show_virtual_construct() {
    // � ������� �����, ��� � ������������-����������� ������
    // ���������� ������ "����" ����������� �������
    {
        A_virt a;

        // ����� "�������������" ������� ������� (��������� << )
        cout << a;
    }

    {
    B_virt b;
    cout << b;
}
}

void show_linkage(){

    // ������, ��������� ������������� ��� ����������� �������
    Base b;
    Derived d;

    // ����������� ������ ������ ���������� - � ����� �������
    // �� ��������� ������ �������� ������
    // ���������� ���������� ����� �������� ������
    int ret = b.call_protected_function(5);	// 11
    ret = d.call_protected_function(5);		// 11

    // ������, ��������� ������������� � ������������ ���������
    Base_virt bv;
    Derived_virt dv;

    // ����������� ������� (������������) ���������� -
    // �� ������ ������ �� ��������� ������ �������� ������
    // ���������� ���������� ����� ������������ ������
    ret = bv.call_protected_function(5);	// 11
    ret = dv.call_protected_function(5);	// 26
}

// ������������ ��������� ������������ ��������
// � ���������� ����������� ������� � �������� �� ��������� � �� ����������
void show_virtual_def_values(){
    Base1 b;
    Derived1 d;
    Base1* b1 = &b;
    Base1* d1 = &d;

    // � ������� Base1::g(i = 10)
    b1->f(1);
    b1->f(1.0);
    b1->g();
    A_virt* av1 = b1->pf();

    // � ������� Derived1::g(i = 10)
    // ������ �������� �� ��������� �����������
    d1->g();
    // ���������� ������� � ����������� ������
    // ����� ���������� ��������� ��� ��������� ��� ������,
    // ���� ��� ���������� � ��������
    A_virt* av2 = d1->pf();
    delete av1;
    delete av2;
}

void show_adapter(){

    // ������� �����
    deque_t d;
    d.push_front(1);
    d.push_back(2);
    d.push_back(3);
    int a = d.pop_back();

    // ����������� �����, ��������� � ������� �������� �������������
    adapter_stack_t s;
    s.push(1);
    s.push(2);

    a = s.pop();

    write_only_adapter_stack_t ws;
    ws.push(1);
    ws.push(2);

    another_adapter_stack_t as;
    // ������� ������ ������ push_back() � pop_back()
    as.push_back(a);
    as.push_back(1);
    //as.push_front(2); - ����� ������ � ����������� ������!
}

void show_abstract_destructor(){

    // �� ����� �������������� ����������� �����
    //(���� ���������� � ���� ����� ���������� ����!)
    //v_base vb;

    // � ����������� �����, ���� ��� ���������� � �� ���������!
    v_derived vd;

    // ���������� ����� ����������� ������� �������� ������!
    vd.pure();
}

void show_open_hide(){
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


// ������� �� ����� ����������
struct A{
    A(){
        cout << "A ";
    }
    A(const A& a){
        cout << "copy-A ";
    }
    A& operator=(const A& b){
        cout << "=A ";
        return *this;
    }
    ~A(){// ���������� �������������!
        cout << "~A ";
    }
};

struct B : public A{

    B(){
        cout << "B";
    }
    B(const B& b){
        cout << "copy-B ";
    }
    B& operator=(const B& b){
        cout << "=B ";
        return *this;
    }
    ~B(){
        cout << "~B";
    }
};

void fff(A a){
}

void ff1(){
    // ������ �� �������� � �������
    // � ����� ��������� ���������� ������ B �� ��������� A
    // �� � ������ A *�������������* ����������, ��� ��� ����� ������ *������*
    // ���������� �������� ������
    std::auto_ptr<A> o(new B);
}

void ff2(){
    // AB~A~A~B~A
    // ������� copy ctor, �������� ��������� ������ �� �������:
    // ��� �������� ��������� �� �������� ���������� copy ctor
    // ��� A-����� ������ B � ��������� ��������� ������.
    // A B copy-A ~A ~B~A
    B b;
    fff(b);
}


int main(){

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
