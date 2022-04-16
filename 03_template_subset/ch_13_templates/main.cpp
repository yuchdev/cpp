#include "tmpl.h"
#include "class_tmpl.h"
#include "func_tmpl.h"
#include "alg_tmpl.h"
#include "bind_tmpl.h"

template <typename T>
void show_fetch_type_info(){

    // info about passed types (traits, strategies) could be fetched from compiler info
    typedef T::value_type vector_value_type;
    std::cout << typeid(vector_value_type).name() << std::endl;
}

// Демонстрация специализации шаблонов
// и частичной специализации стандартных контейнеров
int show_special()
{
    basic_string<char> ch_str("Char string");
    basic_string<char> ch_str2("Another char string");

    basic_string<wchar_t> wch_str(L"Wide char string");
    basic_string<wchar_t> wch_str2(L"Another wide char string");

    // Специализация (подставление значений)
    // Инстанцирование - генерация специализированного кода
    container< char, lexigraphical<char> > c(ch_str);
    c.compare(ch_str2);

    container< wchar_t, no_case<wchar_t> > wc(wch_str);
    wc.compare(wch_str2);

    // использование частичной специализации для указателей -
    // стандартного контейнера
    my_stack<int> mystack;
    my_stack<int*> my_pointers_stack;

    return 0;
}

void show_internals(){
    no_templ nt1;
    no_templ nt2(5);

    templ<int> t1(5);
}

void show_inherit(){
    Xclass a, b, c;
    Yclass e, f;
    unsigned long l1 = Xclass::count;
    unsigned long l2 = Yclass::count;
}

void show_algirithms_pointer(){

    using namespace alg_func_pointer;

    int a[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0 };
    int b[10] = {};
    size_t sz = sizeof(a) / sizeof(int);

    copy_if_greater(a, a + sz, b, 5);
}

void show_algirithms_pointer_predicate(){
    using namespace alg_func_pointer;

    int a[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0 };
    int b[10] = {};
    size_t sz = sizeof(a) / sizeof(int);

    copy_if_greater(a, a + sz, b, greater_func<int>, 7);
}

void show_algirithms_fuctor(){

    using namespace alg_functor;

    int a[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0 };
    int b[10] = {};
    size_t sz = sizeof(a) / sizeof(int);

    // Скопировать только четные
    alg_functor::copy_if(a, a + sz, b, is_odd());

    // Скопировать больше 6
    alg_functor::copy_if(a, a + sz, b, greater_then(6));

    // Скопировать меньше 8
    alg_functor::copy_if(a, a + sz, b, less_then<int>(8));
}

void show_algirithms_binders(){

    using namespace alg_adapter;

    int a[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0 };
    int b[10] = {};
    size_t sz = sizeof(a) / sizeof(int);

    // Непосредственное использование адаптера
    alg_functor::copy_if(a, a + sz, b, bind_second< greater_f<int>, int >(greater_f<int>(), 4));

    // Использование с функцией-оберткой
    alg_functor::copy_if(a, a + sz, b, bind_right(greater_f<int>(), 5));
}

void show_algorithms_func_binders(){

    using namespace alg_adapter_func;

    int a[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0 };
    int b[10] = {};
    size_t sz = sizeof(a) / sizeof(int);

    // Использование алгоритма с адаптером для указателя на функцию и унарной функции
    alg_functor::copy_if(a, a + sz, b, ptrfun(is_odd_func));

    // Использование алгоритма с адаптером для указателя на функцию и бинарной функции
    alg_functor::copy_if(a, a + sz, b, alg_adapter::bind_right(ptrfun(greater_func), 4));
}

void show_algorithms_methods_binders(){

    using namespace alg_adapter_method;

    int a[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0 };
    int b[10] = {};
    size_t sz = sizeof(a) / sizeof(int);

    // Использование алгоритма с адаптером для указателя на метод класса и унарной функции
    alg_functor::copy_if(a, a + sz, b, memfun(&constant::is_odd));

    // Использование алгоритма с адаптером для указателя на метод класса и бинарной функции
    alg_functor::copy_if(a, a + sz, b, alg_adapter::bind_right(memfun(&constant::greater), 5));
}

int main(){


    show_fetch_type_info<std::vector<int>>();

    show_special();
    show_internals();
    show_inherit();
    show_func_pointers();

    show_algirithms_pointer();
    show_algirithms_pointer_predicate();
    show_algirithms_fuctor();
    show_algirithms_binders();
    show_algorithms_func_binders();
    show_algorithms_methods_binders();

    return 0;
}
