#pragma once
#include <string>

// В данном хедере рассматривается структура типового класса
// с обычными, константными и статическими членами
// константными и обычными методами
// Методы снятия констанности, спецификатор mutable


class A{

    // friend declarations of non-existent classes are ignored
    friend class do_not_exists;

public:

    // Т.к. класс содержит константные члены
    // Конструктор по умолчанию не может быть вызван
    //A(); (то же для ссылок)

    A() : a1(10), a5(0), a8(0), cp1(0){}

    // Для инициализации нулем может быть применена форма записи ()
    A(int a) : a1(a), a5(), a8(), cp1(){}
    A(int, int, int);

    int get_int1() const;
    int get_int6() const;

    void set_a5(int);

    void set_a6_mutable(int) const;

    void set_static(int) const;

    // Имеет смысл помещать приватные данные в конце,
    // чтобы сделать акцент на открытых данных интерфейса
    // в этом случае инлайновые методы стоит размещать
    // за пределами объявления класса в том же хедере
private:
    const int a1; // константное поле, д.б. инициализировано в конструкторе
    static const int a2 = 10; // константное статическое поле м.б. иниц. в объявлении
    static const int a3;       // д.б. иниц. в cpp
    static const double a4;    // д.б. иниц. в cpp
    static int a_stat;


    int a5;
    // Неконстантные статические члены
    static int a6;
    static double a7;

    // !!! В качестве констант также удобно использовать
    // члены неименованных перечислений
    enum{ c1 = 5, c2 = 7, c3 = 12 };

    // Если член объявлен mutable, то он изменим всегда
    // даже являясь членом статического объекта
    // Использовать их надо с осторожностью, когда дизайн этого действительно требует
    // Например, кэш к константном объекте может быть неконстантным
    // так называемая "концептуальная константность" (Мейерс 1 Правило 21)
    mutable int a8;

    // возможна обратная ситуация - константный метод модифицирует память,
    // на которую указывает указатель
    // такой метод константен фактически, но не концептуально

    // константность указателей
    char *const cp1; // константный указатель
    char const* cp2; // указатель на константу
    const char* cp3; // указатель на константу

    // Можно также читать объявление "справа налево" -
    // char *const cp1 - константный указатель на char
    // char const* cp2 - указатель на константный char
    // const char* cp3 - указатель на char константный

    // если класс содержит ссылки, то конструктор копирования
    // не может быть создан!
};


// Константный метод - ничего не меняет в классе
// Может быть вызван как для константного, так и для неконстантного экземпляра.
inline int A::get_int1() const	{
    return a1;
}

inline int A::get_int6() const{
    return a6;
}


// Неконстантный метод - может быть вызван только
// для неконстантного экземпляра класса
inline void A::set_a5(int i){
    a5 = i;
}

// Константный метод, изменяющий mutable-член
// может быть вызван для константного экземпляра
inline void A::set_a6_mutable(int i)const {
    a8 = i;
}

inline void A::set_static(int i) const{
    // Static members COULD be changed in const members
    a_stat = i;
}

// Классы имеет смысл замещать структурами,
// если они содержат немного данных, или просто композицию данных,
// а не представление реального объекта
struct AA{
    int a;
    int b;
};

// Близкие по смыслу классы имеет смысл помещать в пространство имен

// Мейерс 1 Правило 18. Интерфейс класса.
// - должен быть одновременно полным и минимальным
// - должен содержать по возможности небольшое число функций
// - среди функций не должно быть дубликатов
// - методы по умолчанию константны, а неконстантны по требованию
// - класс с большим количеством функций трудно поддерживать

// Мейерс 1 Правило 20. Аргументы за сокрытие данных
// - доступ rw
// - функциональная абстракция - заменить член класса вычислением или вообще вынести его в другое место
// - проверка, логгинг
// - если член класса открыт в методе, мы ожидаем, что это член этого класса



// Good structure of a typical class
class good_weekday_t{

    // Better start with open interface
public:

    // start from enums if exist
    enum Wday{ mon = 1, tus, wed, thr, frd, sat, sun };

    // Then internal classes and exception classes
    class bad_weekday_t{};

    // constructors
    good_weekday_t(int day = mon) : _wday(day){}
    good_weekday_t(std::string& sday){/*здесь преобразование строки в день недели*/ }

    // const methods-accessors
    Wday get_weekday() const;
    std::string get_weekday_str() const;

    // operators if exist
    bool operator==(const good_weekday_t& w);
    bool operator!=(const good_weekday_t& w);

    // Helper functions
    bool is_weekend() const;
    int diff(const good_weekday_t& w) const;

    // non-const mutators
    inline void set_weekday(int day);
    // inline (just like register) is not a directive
    // it is canceled by cycles, recursions
    // of course, virtual cancel inline (it's in runtime)
    // pointer to such function cancel inline (it should have address!)

    // when using inline, compiled binary size
    // could be smaller or bigger, it depends
    // If the library or API includes inline function,
    // it is being built-in into the executable

    // constructors can't be inlined, as this object 
    // may be included into the class hierarchy
    // TODO: find better explanations

private:
    int _wday;
    std::string _string_day;
};
