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

    // По указателю или ссылке производный класс может быть
    // присвоен базовому без явного приведения
    Employee* pe1 = &m1;
    Employee& re1 = m1;

    // Обратное неверно и вызовет ошибку!
    //Output: cannot convert from 'Employee *__w64 ' to 'Manager *'
    //Manager* pm2 = &e1;

    // Методы базового класса доступны по указателю
    pe1->show_name();
    // А производного - недоступны
    //pe1->show_level(); error C2039: 'show_level' : is not a member of 'Employee'

    // В этом случае допустимо явное приведение к производному классу
    static_cast<Manager*>(pe1)->show_level();

    // ------------------------------
    Manager m2("Vatslav Melioratsya", 2, 4000);

    m2.show_level();
    // show_name() также определен в Manager, он замещает метод базового класса
    m2.show_name();
    // Хотя можно явно вызвать метод базового снаружи
    m2.Employee::show_name();

    // Внимание - это эффект срезки. Компилятор даже не предупреждает.
    //Employee e2 = m2;
}

// --- Демонстрация перегрузки операторов и наследования ---
void show_operators(){
    currency_t c1 = 1.0;
    currency_t c2 = 20;

    roubles_t r1(1.);
    roubles_t r2 = r1;

    // --- Проверим операторы базового класса ---
    currency_t c3 = c1;
    c3 = c1 + c2;
    c3 += 2.;
    c3 = c2 + 1;

    // --- Проверим операторы производного класса ---
    r2 = r1;
    r1 += r2;
    roubles_t r3 = r1 + r2;

    // --- Проверим операторы в обоих классах ---
    r3 = r1 + c1;

    // А сложение уже не работает
    // error C2440: 'initializing' : cannot convert from 'currency_t' to 'roubles_t'
    //roubles_t r3 = r1 + r2;

    // r1 и r2 в результате _повышающего_ преобразования молча приведены
    // к базовому классу и отправлены параметрами в operator+()
    // А он возвращает currency_t по значению

    // Чтобы выйти из ситуации,	введем в roubles_t
    // перегруженный преобразующий operator=() - см. h-файл
    // а также аналогичный конструктор!
    r1 = c1; // т.е. в функциональной форме r1.operator =( c1 );
    const currency_t cr3 = r1 + r2; // т.е. вызов конструктора cr3( r1 + r2 );
    roubles_t r4 = cr3;		// r3( cr3 )

    // downcasting безопаснее делать при помощи dynamic_cast
    if (roubles_t* r = dynamic_cast<roubles_t*>(&c3)){
        r->print();
    }
}

// --- Демонстрация абстракции и виртуальных функций ---
void show_abstract() {

    // Данные классы являются полиморфными
    // и внутри хранят таблицу виртуальных функций
    circle c;
    rectangle r;
    sphere s;
    cube cb;

    vector<shape*> vs;

    vs.push_back(&c);
    vs.push_back(&r);
    vs.push_back(&s);
    vs.push_back(&cb);

    // Рисуем все фигуры из контейнера с помощью виртуальной функции и
    // алгоритма for_each()
    for_each(vs.begin(), vs.end(), mem_fun(&shape::draw));
}

// --- Демонстрация использования виртуальных функций в конструкторе-деструкторе ---
void show_virtual_construct() {
    // В примере видно, что в конструкторе-деструкторе класса
    // вызывается всегда "своя" виртуальная функция
    {
        A_virt a;

        // прием "виртуализации" внешней функции (оператора << )
        cout << a;
    }

    {
    B_virt b;
    cout << b;
}
}

void show_linkage(){

    // Классы, связанные наследованием без виртуальных функций
    Base b;
    Derived d;

    // Выполняется только раннее связывание - в обоих случаях
    // из открытого метода базового класса
    // вызывается защищенный метод базового класса
    int ret = b.call_protected_function(5);	// 11
    ret = d.call_protected_function(5);		// 11

    // Классы, связанные наследованием с виртуальными функциями
    Base_virt bv;
    Derived_virt dv;

    // Выполняется позднее (динамическое) связывание -
    // во втором случае из открытого метода базового класса
    // вызывается защищенный метод производного класса
    ret = bv.call_protected_function(5);	// 11
    ret = dv.call_protected_function(5);	// 26
}

// демонстрация различных возвращаемых значений
// в прототипах виртуальных функций и значений по умолчанию в их параметрах
void show_virtual_def_values(){
    Base1 b;
    Derived1 d;
    Base1* b1 = &b;
    Base1* d1 = &d;

    // в функции Base1::g(i = 10)
    b1->f(1);
    b1->f(1.0);
    b1->g();
    A_virt* av1 = b1->pf();

    // в функции Derived1::g(i = 10)
    // однако параметр по умолчанию наследуется
    d1->g();
    // вируальная функция в наследующем классе
    // может возвращать различный тип указателя или ссылки,
    // если она приводится к базовому
    A_virt* av2 = d1->pf();
    delete av1;
    delete av2;
}

void show_adapter(){

    // Базовый класс
    deque_t d;
    d.push_front(1);
    d.push_back(2);
    d.push_back(3);
    int a = d.pop_back();

    // Производный класс, связанный с базовым закрытым наследованием
    adapter_stack_t s;
    s.push(1);
    s.push(2);

    a = s.pop();

    write_only_adapter_stack_t ws;
    ws.push(1);
    ws.push(2);

    another_adapter_stack_t as;
    // открыты только методы push_back() и pop_back()
    as.push_back(a);
    as.push_back(1);
    //as.push_front(2); - метод закрыт в производном классе!
}

void show_abstract_destructor(){

    // Не можем инстанцировать абстрактный класс
    //(хотя деструктор и один метод определены явно!)
    //v_base vb;

    // А производный можем, хотя его деструктор и не определен!
    v_derived vd;

    // Вызывается чисто виртуальная функция базового класса!
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


// Задачки из теста Дойчебанка
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
    ~A(){// деструктор невиртуальный!
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
    // Вопрос на внимание к деталям
    // В умный указатель передается объект B по указателю A
    // Но в классе A *невиртуальный* деструктор, так что будет вызван *только*
    // деструктор базового класса
    std::auto_ptr<A> o(new B);
}

void ff2(){
    // AB~A~A~B~A
    // добавив copy ctor, получаем ожидаемый эффект со срезкой:
    // При передаче параметра по значению вызывается copy ctor
    // для A-части класса B и создается временный объект.
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
