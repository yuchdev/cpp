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

// Демонстрация простого множественного наследования
void show_simple(){

    size_t sz = 0;

    {
        control_t control;
        // Здесь и далее считаем размер класса
        // control_t = 8 (2x int)
        sz = sizeof(control);
        control.draw();
        control.hide();
        control.show();
    }

    {
        button_t button;
        // в объект входит control_t + button_t = 12
        sz = sizeof(button);

        // Переопределенные в button_t методы
        // работает перекрытие
        button.push();
        button.draw();

        // Не переропределенные методы
        // вызываются из родительского класса
        button.hide();
        button.show();
    }

    {
        slider_t slider;
        // в объект входит control_t + slider_t = 20 (5x int)
        sz = sizeof(slider);

        // вызывается свой метод
        slider.move(0);

        // вызываются методы базового класса
        slider.draw();
        slider.hide();
        slider.show();
    }

    {
        // здесь дважды вызывается конструктор control_t()
        // последовательность такая:
        // control_t->slider_t->control_t->button_t->window_draw
        // Т.е. конструкторы вызываются в порядке, прописаном при наследовании
        // Конструктор control_t вызывавется дважды
        // (см. simple.h)
        window_draw drawer;

        // в объект входит control_t x2 + button_t + slider_t + window_draw = 36
        // Т.е. класс control_t включен 2 раза
        sz = sizeof(drawer);

        // вызываются переопределенные методы
        drawer.draw();
        drawer.hide();
        drawer.show();

        // вызываются методы родителей
        // (каджый из них имеет только 1 копию, поэтому работает нормально)
        drawer.move(0);
        drawer.push();
    }
}

// Демонстрация виртуального множественного наследования
void show_vitrual(){

    size_t sz = 0;
    // Объект содержит 3 указателя на базовый класс, сам базовый класс и производный класс
    code_formatter formatter(1);

    formatter.generate(cpp_lang);
    sz = sizeof(formatter);
    // 36 = ( base(int+vptr) = 8; generators(3 * (vfptr+vtbl)) = 24 ; last_class(vprt) = 4 )

    cout << "code_formatter size = " << sz << endl;
    sz = sizeof(code_generator_base);	// 8 (int + vtbl)
    cout << "code_generator_base size = " << sz << endl;
    sz = sizeof(cpp_code_generator);	// 20 (4 + 2*(vtbl+vfptr) )
    cout << "cpp_code_generator size = " << sz << endl;


    // Наследование ромбовидное, но один из классов наследован невиртуально
    Final f;
    sz = sizeof(Base);		// 4 (vtbl)
    sz = sizeof(Derived1);	// 4 (vtbl)
    sz = sizeof(Derived2);	// 12 (2*vtbl + vfptr)
    sz = sizeof(f);			// 16
}

void show_sizes(){

    size_t sz = 0;

    {
        // 4 пустых класса связаны ромбовидным наследованием
        class A{};
        class B1 : public A{};
        class B2 : public A{};
        class C : public B1, public B2{};

        // каждый класс размером 1
        sz = sizeof(A);
        sz = sizeof(B1);
        sz = sizeof(B2);
        sz = sizeof(C);
    }


    {
        // добавим поле 2 байта в базовый класс
        class A{ short a; };
        class B1 : public A{};
        class B2 : public A{};
        class C : public B1, public B2{};

        sz = sizeof(A);	// 2
        sz = sizeof(B1);// 2 (от базового)
        sz = sizeof(B2);// 2 (от базового)
        sz = sizeof(C);	// 4 (2 копии базового)
    }


    {
        // сделаем одну из веток наследования виртуальной
        class A{ short a; };
        class B1 : virtual public A{};
        class B2 : public A{};
        class C : public B1, public B2{};

        // теперь в нижнем классе по прежнему 2 копии базового,
        // но появился еще указатель на виртуальное наследование
        sz = sizeof(A);	// 2
        sz = sizeof(B1);// 6 (2 от базового + указатель)
        sz = sizeof(B2);// 2 (от базового)
        sz = sizeof(C);	// 8 (2 от базового по невиртуальной ветке, 6 по виртуальной)
    }


    {
        // сделаем обе ветки наследования виртуальными
        class A{ short a; };
        class B1 : virtual public A{};
        class B2 : virtual public A{};
        class C : public B1, public B2{};

        // теперь в нижнем классе 1 копия базового и 2 указателя vtbl
        sz = sizeof(A);	// 2
        sz = sizeof(B1);// 6 (2 от базового + указатель)
        sz = sizeof(B2);// 6 (2 от базового + указатель)
        sz = sizeof(C);	// 10 (2 от базового + 2 указателя)
    }

    {
        // добавим еще одну ветку виртуального наследования
        class A{ short a; };
        class B1 : virtual public A{};
        class B2 : virtual public A{};
        class B3 : virtual public A{};
        class C : public B1, public B2, public B3{};

        // теперь в результирующем классе 3 указателя
        sz = sizeof(A);		// 2
        sz = sizeof(B1);	// 6 (2 от базового + указатель)
        sz = sizeof(B2);	// 6 (2 от базового + указатель)
        sz = sizeof(B3);	// 6 (2 от базового + указатель)
        sz = sizeof(C);		// 14 (2 от базового + 3 указателя)
    }

    {
        // добавим в базовый класс виртуальный деструктор
        class A{
        public:
            short a;
            virtual ~A(){}
        };
        class B1 : virtual public A{};
        class B2 : virtual public A{};
        class B3 : virtual public A{};
        class C : public B1, public B2, public B3{};

        // теперь размер базового класса на 4 болье - еще один vtbl
        sz = sizeof(A);	// 6 (2 - данные + 4 - vtbl)
        sz = sizeof(B1);// 10 (6 от базового + 4 - vtbl)
        sz = sizeof(B2);// 10 (6 от базового + 4 - vtbl)
        sz = sizeof(B3);// 10 (6 от базового + 4 - vtbl)
        sz = sizeof(C);	// 18 (6 от базового + 3*4 - vtbl)
    }
}



int main()
{

    show_simple();
    show_vitrual();
    show_sizes();
    return 0;
}
