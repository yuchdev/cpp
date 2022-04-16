/** @brief
Глава 13 Страуструпа
Простые шаблоны. Определение, инстанцирование, параметры, эквивалентность, проверка типов
Шаблоны функций.
Шаблоны по выбору алгоритма.
Специализация. Наследование и шаблоны. Преобразования шаблонов.
*/

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

using namespace std;


// ----------------------------------
// Абстрактный класс шаблона -
// может использоваться как обычный класс
// для создания общего интерфейса
// Шаблон может наследовать также от обычного класса.
template<typename Symbols>
class compare_str{
public:
    virtual bool compare(const basic_string<Symbols>& str1, const basic_string<Symbols>& str2) = 0;
};

// ----------------------------------
// Производные шаблонные классы
// с виртуальными методами
template<typename Symbols>
class lexigraphical : public compare_str<Symbols>{
public:
    virtual bool compare(const basic_string<Symbols>& str1, const basic_string<Symbols>& str2);
};

template<typename Symbols>
class no_case : public compare_str<Symbols>{
public:
    virtual bool compare(const basic_string<Symbols>& str1, const basic_string<Symbols>& str2);
};

// --------------- реализация методов ---------------
template<typename Symbols>
bool lexigraphical<Symbols>::compare(const basic_string<Symbols>& str1, const basic_string<Symbols>& str2){
    return str1 > str2;
}

template< typename Symbols >
bool no_case<Symbols>::compare(const basic_string<Symbols> &str1, const basic_string<Symbols>& str2){
    std::basic_string<Symbols> nocase1, nocase2;
    std::transform(str1.begin(), str1.end(), std::back_inserter(nocase1), ::tolower);
    std::transform(str2.begin(), str2.end(), std::back_inserter(nocase2), ::tolower);
    return nocase1 > nocase2;
}
// ----------------------------------


// ----------------------------------
// Шаблонный класс, в который передается 2 параметра
// тип символов и тип сравнения
// Последний имеет значение по умолчанию
template<typename Symbols, typename Comparer = lexigraphical<Symbols> >
class container{
    basic_string<Symbols> _str;
    Comparer _c;
public:
    container(basic_string<Symbols>& str);
    void compare(basic_string<Symbols>& str);
};

template<typename Symbols, typename Comparer>
container<Symbols, Comparer>::container(basic_string<Symbols>& str) :
_str(str)
{}

// Используем переданный тип класса для сравнения строк
template<typename Symbols, typename Comparer>
void container<Symbols, Comparer>::compare(basic_string<Symbols>& str){
    _c.compare(str, _str);
}

// Пример - частичной специализации -
// Стек для хранения только указателей
// наследует интерфейс от my_stack<void*>
// Чтобы определить тип элементов как указатель
// явно укажем параметр шаблона my_stack<T*>
// Специализация - это "перегрузка" для шаблонных классов

// Наследование частичной специализации от полной, чтобы сократить дублирование кода
// - это паттерн шаблонного программирования
// Без него специализированные классы не связаны никакими отношениями, тем более наследованием
template <typename T>
class my_stack{
public:
    my_stack(){
        cout << "general stack<T>" << endl;
    }
    // Здесь методы
private:
    std::vector<T> _stack1;
};


template <>
class my_stack<void*>{
public:
    my_stack(){
        cout << "Full specialization <void*>" << endl;
    }
    // Здесь методы
protected:
    std::vector<void*> _stack1;
};


template <typename T>
class my_stack<T*> : public my_stack<void*>{
public:
    typedef my_stack<void*> base;
    my_stack() : base(){
        cout << "Pointers stack<T*>" << endl;
    }
    // Здесь методы
};
