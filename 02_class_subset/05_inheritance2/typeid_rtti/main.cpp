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

// Демонстрация определения типа и RTTI
void show_typeid_rtti(){

    code_formatter formatter(1);
    formatter.generate(cpp_lang);

    // для upcast или crosscast dynamic_cast требует полиморфизма

    // повышающее преобразование (upcast)
    cpp_code_generator* cpp_gen = dynamic_cast<cpp_code_generator*>(&formatter);

    // перекрестное преобразование (crosscast)
    php_code_generator* php_gen = dynamic_cast<php_code_generator*>(cpp_gen);
    // при перекрестном преобразовании результирующий тип не обязан быть полиморфным

    // повышающее преобразование (upcast)
    code_generator_base* bs = dynamic_cast<code_generator_base*>(php_gen);

    // понижающее преобразование (downcast)
    code_formatter* cf = dynamic_cast<code_formatter*>(bs);
    // если динамическое преобразование неоднозначно
    // (в случае нескольких копий базового класса)
    // возвращается 0 или бросается bad_cast

    // Приведение к void* с помощью dynamic_cast применяется для нахождения адреса
    // начала объекта полиморфного типа (в очень низкоуровневом коде!)
    void* cfvoid = dynamic_cast<void*>(cf);

    // Если преобразование запрещено, то возвращается 0-указатель
    finalizer* fin = dynamic_cast<finalizer*>(cf);
    if (fin == 0)
        cout << "Wrong dynamic cast" << endl;

    // Если преобразуется в ссылку, то при неудаче бросается исключение bad_cast
    try	{
        finalizer& fin1 = dynamic_cast<finalizer&>(formatter);
    }
    catch (const std::bad_cast& e)	{
        cout << e.what() << endl;
    }

    // Информация о типе:

    // По имени типа
    const type_info& ti1 = typeid(finalizer);
    cout << ti1.name() << endl;

    // По имени объекта
    const type_info& ti2 = typeid(formatter);
    cout << ti2.name() << endl;

    // Для базового типа
    const type_info& ti3 = typeid(int);
    cout << ti3.name() << endl;

    // Символьное представление зависит от реализации

    // При попытке получить тип нулевого полиморфного указателя бросается исключение bad_typeid (???)
    try	{
        code_generator_base* bs = 0;
        const type_info& ti = typeid(bs);
        cout << ti.name() << endl;
    }
    catch (const std::bad_typeid& e)	{
        cout << e.what() << endl;
    }

    // Метод before нужен для сортировки
    int b = ti1.before(ti2);
    b = ti1.before(ti3);
}


int main()
{

    show_simple();
    show_vitrual();
    show_sizes();
    show_final();
    show_typeid_rtti();
    show_multimethod();

    member_derived md;
    show_member_pointer(&md);
    show_new_delete();

    return 0;
}
