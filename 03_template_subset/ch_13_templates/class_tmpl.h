#include <iostream>
using std::cout;
using std::endl;
// Нетиповые параметры шаблона

// ------------------- Шаблоны, друзья -------------------
// Шаблон массива с проверкой границ
// обнуляется на запросу и бросает исключения
// Кстати, можно задать размер по умолчанию,
// но в данном случае это будет bad design
template <typename T, int MAX>
class fixed_array{
public:
    // Полезно улучшить читабельность класса введением следующий typedef
    typedef T			value_type;
    typedef T&			reference;
    typedef const T&	const_reference;

    fixed_array();
    T& operator[](int index);
    const T& operator[](int index) const;

    // Шаблоны могут быть друзьями и у шаблонов могут быть друзья
    // - Приведенный дружественный оператор сам не является шаблоном -
    // он просто зависит от его параметра. Он должен от него зависеть,
    // иначе будет нарушено ODR
    // - Дружественные функции шаблона лучше определять в самом шаблоне
    // (это делается для удобства приведения типов)
    friend std::ostream& operator<< (std::ostream& os, const fixed_array<T,MAX>& f){
        for(int i = 0 ; i < MAX; i++){
            os << f._arr[i];
        }
        return os;
    }

private:
    T _arr[MAX];
};



template <typename T, int MAX>
fixed_array<T,MAX>::fixed_array() : _arr(){}

template <typename T, int MAX>
T& fixed_array<T,MAX>::operator [](int index) {
    if( (index < 0) || (index >= MAX) )
        throw std::out_of_range("Array ranges error");
    return _arr[index];
}

template <typename T, int MAX>
const T& fixed_array<T,MAX>::operator [](int index) const {
    if( (index < 0) || (index >= MAX) )
        throw std::out_of_range("Array ranges error");
    return _arr[index];
}

// Подобные параметры можно использовать и в функциях
// Пример, немного надуманный, но
template <typename T, int VAL>
T add_value(const T& t){
    return t + VAL;
}

// Нетиповые параметры шаблонов могут быть
// - целочисленными константами
// - указателями на объекты с внешним связыванием

// Параметр шаблона может быть использовать для последующего
// определения других параметров
template <typename T, T def_val> class cont;

// ------------------- Вложенные шаблоны -------------------
// Возможны следующие варианты:
// - поле-шаблон в классе (д.б. инстанцировано) или шаблоне
// - метод-шаблон, в т.ч. конструктор в классе или шаблоне
// - вложенный шаблон в классе или шаблоне
// - вложенный класс в классе или шаблоне

// Шаблон не может быть определен внутри функции!
// Также нельзя совместить объявление вложенного шаблонного класса
// с определением - непонятно чем его инстанцировать


// 1.Класс
class no_templ{
public:
    // поле в классе
    fixed_array<int, 100> _array;

    no_templ() : _intern(5){}

    // Шаблонный конструктор в классе
    // Шаблонные конструкторы и операторы копирования
    // никогда не замещают конструкторов о операторов по умолчанию
    template <typename T>
    no_templ(const T& t): _intern(5){}

    // Вложенный в класс шаблонный класс
    template <typename T2>
    struct internal_no_templ{
        internal_no_templ(const T2& t) : _param(t){}
        T2 _param;
    } /*_internal - нельзя определить шаблон в классе -
      непонятно чем инстанцировать */;

    internal_no_templ<int> _intern;
};

// 2.Шаблон
template<typename T>
class templ{
public:
    // поле в шаблоне
    fixed_array<T, 100> _array;

    // Шаблонный конструктор в шаблоне
    templ(const T& t) : _intern(t){}

    // Вложенный в шаблон шаблонный класс
    template <typename T2>
    struct internal_no_templ{
        internal_no_templ(const T2& t) : _param(t){}
        T2 _param;
    } /*_internal - нельзя определить шаблон в классе */;

    internal_no_templ<T> _intern;
};

// ------------------- Наследование -------------------
// Шаблон от класса
// Класс от шаблона
// Шаблон от шаблона

// 1.Шаблон от класса
// Статический счетчик в базовом классе
template<typename T>
struct common_counter{
    common_counter(){++count;}
    ~common_counter(){--count;}
    static unsigned long count;
};

template<typename T>
unsigned long common_counter<T>::count = 0;


class Xclass : public common_counter<Xclass>{
public:
    Xclass(){}
};

class Yclass : public common_counter<Yclass>{
public:
    Yclass(){}
};
// 2. Класс от шаблона - см. CRTP.h в проекте к книге Джосаттиса

// 3.Шаблон от шаблона - см. пример в tmpl.h

// ------------------- Наследование от шаблонов и преобразования -------------------
// С точки зрения С++ два класса, сгенерированные из одного шаблона,
// не связаны никакими отношениями наследования. Напрмер:

class Shape{};

class Circle : public Shape{};

template <typename T>
class Set{};

void get_shape(Set<Shape*>& s){

}

void get_circle(Set<Circle*>& c){
    // ошибка преобразования
    //get_shape(c); - раскомментировать для ошибки
}
