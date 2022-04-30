#pragma once
#include <new>
#include <cstdio>
#include <iostream>

#include "user_allooc.h"

using std::new_handler;
using std::bad_alloc;

// In this file we will try to create a user-defined allocator

// Meyers 1 Chapter 7

// Можно замещать стандартный обработчик по нехватке памяти
// Стандартно set_new_handler() вызывается,
// пока память не будет освобождена или программа не завершена
// Пользовательский обработчик может:
// * сделать доступным необходимое количество памяти
// * установить другой обработчик new
// * установить нулевой обработчик (т.е. исключение bad_alloc) или бросить bad_alloc вручную
// * завершить программу по abort()/exit()

// Можно для каждого конкретного класса реализовать свой обработчик set_new_handler()
// Можно создать класс-примесь (mixture) чтобы наследовать от него оператор и обработчик new
// или параметризовать класс им
// Используем шаблоны и CRTP, чтобы получить гарантию, что каждый базовый класс уникален

// Mixtury class defenition that replaces standard
// new_handler

// Мейерс 2 Правило 8
// Различайте оператор new - он выделяет память и вызывает конструктор. Его переопределять нельзя
// и функцию operator new() - она изменяет способ выделения памяти, и переопределять ее можно
// Аналогично с оператором delete и функцией operator delete()


// Мейерс 1 Правило 7
// переопределения для класса operator new() гарантирует,
// что для его экземпляров будет вызван именно он
// Мейерс 1 Правило 8
// operator new() наследуется производными классами
// поэтому operator new() в базовом классе нужно проектировать для производных,
// либо просто вызывать для них стандартный operator new()
// Также реализуйте родственный operator new[]()
// Мейерс 1 Правило 9
// В оператор new может быть добавлено сколько угодно параметров
// Если перегружена какая-либо другая форма new, кроме нормальной
// (замещающая, с указателем на обработчик),
// то нормальная обязательно должна быть перегружена
// Иначе перегруженные операторы сокроют глобальную new()
// Размещающий оператор operator new и размещающий
// оператор operator delete должны появляться только парами
// для каждой размещающей функции распределения памяти под объект
// void operator new(size_t, P2, P3, ..., Pn);
// должна существовать совпадающая функция освобождения памяти
// void *operator delete(void *, P2, P3, ..., Pn);
// При этом размещающий оператор delete вызывается только для
// удаления частично созданного объекта (throw из конструктора).
// Полностью сконструированный объект удаляется с помощью выражения delete —
// и эти выражения вызывают неразмещающий operator delete


// Мейерс 1 Правило 8
// Различайте функцию и оператор new
// Функция просто выделяет память, оператор вызывает конструктор
// Функция delete() и оператор delete() соотносятся примерно так же

// Мейерс 1 Правило 10
// operator new() и operator delete() перегружаются обычно для повышения эффективности
// это особенно верно для приложений, размещающих много маленьких объектов

void test_placement_new_handler()
{
    std::cout << "No more memory!\n";
    throw bad_alloc();
}

void show_user_alloc(){

    // different versions of new() and delete()

    // simple new overload
    {
        std::cout << "Test simple new overload" << '\n';
        user_alloc* x = new user_alloc();
        x->test();
        delete x;
    }

    // new for array overload
    {
    std::cout << "Test array new overload" << '\n';
    user_alloc* x = new user_alloc[10];
    x[1].test();
    delete[] x;
}

    // placement new overload
    {
        std::cout << "Test placement new overload" << '\n';
        user_alloc* x = new user_alloc();
        x->test();
        x->~user_alloc();

        user_alloc* y = new(x)user_alloc();
        // could be directly called in functional form only!
        // will be implicitly called in case of exception
        user_alloc::operator delete(y, x);
        delete x;
    }

    // new_handler replacement new overload
    {
        std::cout << "Test placement new overload" << '\n';
        user_alloc* x = new (test_placement_new_handler)user_alloc();
        x->test();
        delete x;
    }
}

void show_new_handler(){

    // Showcase of memory pool, mixture-class replacing new_handler
    // Let's try a very large object
    try{
        LargeObject::set_new_handler(NewHandlerSupport<LargeObject>::no_more_memory);
        LargeObject* x = new LargeObject();
        delete x;
    }
    catch (const std::bad_alloc& e){
        std::cerr << "Lack of memory: " << e.what() << '\n';
    }

    // Let's try regular object
    try{
        SmallObject::set_new_handler(NewHandlerSupport<SmallObject>::no_more_memory);
        SmallObject* x = new SmallObject();
        delete x;
    }
    catch (const std::bad_alloc& e){
        std::cerr << "Lack of memory: " << e.what() << '\n';
    }
}


int main()
{
    show_user_alloc();
    return 0;
}

