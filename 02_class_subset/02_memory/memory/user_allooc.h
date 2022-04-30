#pragma once
#include <new>
#include <cstdio>

using std::new_handler;
using std::bad_alloc;

// В этом хедере демонстрируется создание объекта
// при помощи пользовательского аллокатора

// Мейерс 1 Правило 7
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
template <typename T>
class NewHandlerSupport{
public:
    static new_handler set_new_handler(new_handler p);
    static void* operator new(size_t size);
    static void operator delete(void* p);
    static void no_more_memory();
private:
    static new_handler _current_handler;
};

// Mixtury class defenition that replaces standard
// new_handler
template <typename T>
new_handler NewHandlerSupport<T>::_current_handler = &NewHandlerSupport<T>::no_more_memory;


template <typename T>
new_handler NewHandlerSupport<T>::set_new_handler(new_handler p)
{
    // ведет себя как стандартный set_new_handler
    new_handler old_handler = _current_handler;
    _current_handler = p;
    return old_handler;
}


template <typename T>
void NewHandlerSupport<T>::no_more_memory(){
    // в нашем кастомном обработчике просто выведем сообщение
    // а дальше поступим как стандартный new_handler
    printf("No more memory!\n");
    throw bad_alloc();
}

// Мейерс 2 Правило 8
// Различайте оператор new - он выделяет память и вызывает конструктор. Его переопределять нельзя
// и функцию operator new() - она изменяет способ выделения памяти, и переопределять ее можно
// Аналогично с оператором delete и функцией operator delete()


template <typename T>
void* NewHandlerSupport<T>::operator new(size_t sz)
{
    // получить стандартный обработчик (0 если throw bad_alloc)
    new_handler global_handler = std::set_new_handler(_current_handler);
    void* mem = nullptr;

    try{
        // выделить память глобальным оператором
        mem = ::operator new(sz);
        // если выделить нельзя - вызовется _current_handler()
        // если он = 0, бросится bad_alloc
    }
    catch (const std::bad_alloc&){
        // вернем на место старый глобальный new_handler
        std::set_new_handler(global_handler);
        throw;
    }
    set_new_handler(global_handler);
    return mem;
}


template <typename T>
void NewHandlerSupport<T>::operator delete(void* p){
    ::operator delete(p);
}


// Наследуем mixture-class
class LargeObject : public NewHandlerSupport<LargeObject> {
public:
    LargeObject(){}
    ~LargeObject(){}
private:
    // very large array, will throw out of memory
    char large_array[2147483646];
};

// Еще раз наследуем mixture-class
class SmallObject : public NewHandlerSupport<SmallObject> {
public:
    SmallObject(){}
    ~SmallObject(){}
private:
    // everything ok should be here
    char large_array[42];
};


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

class user_alloc
{
public:
    user_alloc();

    // Без виртуальных деструкторов функция operator delete() может работать ненадлежащим образом
    virtual ~user_alloc();

    // 1. Simple new/delete overload
    static void* operator new(size_t s);
    static void operator delete(void* p);

    // 2. Array new/delete overload

    static void* operator new[](size_t s);
    static void operator delete[](void* p);


    // 3. Placement new/delete overload

    static void* operator new(size_t s, void* place);
    static void operator delete(void *p, void *place);

    // operator delete is unique in being a non-member or static member function that is dynamically dispatched
    // A type with a virtual destructor performs the call to its own delete from the most derived destructor

    // 4. New replacing new_handler
    static void* operator new(size_t s, new_handler p);
    static void operator delete(void *p, new_handler h);

    void test();
protected:

    // здесь для простоты применена одна защищенная функция
    // для new и new[]. Это действительно подходит, т.к. в перегруженном
    // operator new (или new[]) мы просто выделяем память, и ничего не делаем
    // со сконструированными объектами.
    // На практике могут быть различия, например, в new() мы может проверить
    // s == sizeof(Type), в new[] - уже нет (размер массива неизвестен)
    static void* universal_allocate(size_t s, new_handler handler = 0);
    static void universal_free(void* p);
private:
    int _test;
};

// Мейерс 1 Правило 8
// Различайте функцию и оператор new
// Функция просто выделяет память, оператор вызывает конструктор
// Функция delete() и оператор delete() соотносятся примерно так же

// Мейерс 1 Правило 10
// operator new() и operator delete() перегружаются обычно для повышения эффективности
// это особенно верно для приложений, размещающих много маленьких объектов




