#include <memory>
#include "name_params.h"
#include "crtp.h"
#include "virtual.h"
using std::auto_ptr;

template<typename Worker>
void polymorhic_work(const Worker& w)
{
    w.work();
};

void show_crtp()
{
    count1 b11, b12, b13;
    count2 b21, b22;
    cout << "live objects count1 " << count1::live() << endl;
    cout << "live objects count2 " << count2::live() << endl;

    SpecificWorker1 sp1;
    SpecificWorker2 sp2;
    polymorhic_work(sp1);
    polymorhic_work(sp2);
}

void show_tmpl_virtuality()
{
    Base<non_virtual>* p1 = new Derived<non_virtual>();
    Base<my_virtual>* p2 = new Derived<my_virtual>();
    p1->foo();
    p2->foo();
    delete p1;
    delete p2;
}

int main()
{
    show_name_params();
    show_crtp();
    show_tmpl_virtuality();
    return 0;
}