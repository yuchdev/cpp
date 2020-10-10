#include <iostream>
#include <cmath>
#include <bitset>
#include <ctime>
#include <cstddef>

/*
Размеры типов:
1 = sizeof(char) <= sizeof(short) <= sizeof(int) <= sizeof(long)
1 <= sizeof(bool) <= sizeof(long)
sizeof(char) <= sizeof(wchar_t) <= sizeof(long)
sizeof(float) <= sizeof(double) <= sizeof(long double)
sizeof(N) <= sizeof(signed N) <= sizeof(unsigned N)
*/

using std::bitset;
using std::cout;
using std::endl;

/* Перечисления и их размеры */
enum enum1{ dark, light };		// диапазон 0:1
enum enum2{ a = 3, b = 9 };		// диапазон 0:15
enum enum3{ c = 3, d = 900 };	// диапазон 0:1024
enum enum4{ e = 3, f = 5000000000 };	// диапазон 0:2^32 - не входит!

void show_enumerations(){

    // преобразование в enum1
    enum1 e1 = enum1(0);
    enum1 e2 = enum1(1);

    enum1 e3 = enum1(20); // - компилируется, но неверно -
    // 20 не принадлежит перечислению enum1

    // Размер перечисления архитектурозависим
    size_t sz = sizeof(enum1);
    sz = sizeof(enum2);
    sz = sizeof(enum3);

    // Здесть значение enum будет обрезано!
    enum4 e4 = enum4::f;
    sz = sizeof(e4);
    long long l = e4;
}

static int g_global = 5;

void show_name_convensions(int a){
    // Давайте переменным с большой областью видимости
    // большие, понятные имена
    static int test_counter = a;

    // А с малой - короткие однобуквенные
    for (int i = 0; i < test_counter; i++){
        ::g_global += test_counter;
        cout << ::g_global << endl;
    }

    // обращение к глобальной
    ::g_global = 0;

    // сокрытие глобальной
    int g_global = 0; // (автоматический объект)

    // Не существует способа обратиться к
    // сокрытой локальной переменной!
}

// Сравнение указателей
template<typename T>
bool less_ptr(const T* p1, const T* p2) {
    return p1 < p2;
}

//Пункт стандарта не скажу по памяти, но акцентируется разница между сравнением на == или != и сравнением на < >.
//Сравнение на == != разрешено в любом случае, а на больше / меньше только в одном массиве,
//в противном случае — unspecified behavior(не undefined).

// Так что, если хочется корректности, то нужно пользоваться теми средствами, для которых гарантии даны — т.е.писать
template<class T> bool less_ptr(T* a, T* b) {
    return std::less<T*>()(a, b);
}

//А заодно —(вымышленный) пример реализации, которая может этим зазором воспользоваться на вред юзеру.
//Сегментная модель памяти, указатель представляет пару(селектор, смещение).
//Ну хотя бы large модель для 16 - битного режима x86.
//Делается допущение, что каждый объект и каждый массив расположены в пределах одного сегмента,
//поэтому для сравнения указателей(на неравенство) достаточно сравнить смещения.

void show_pointer(){

    // как работает typedef для массива
    typedef int MyNumbers[10];
    MyNumbers numbers = { 1, 2, 3 };

    // Гарантируется, что нет объектов с нулевым адресом.

    const int* m;
    const int*& o = m;

    // строковый литерал - это указатель
    size_t sz = sizeof("aaa");

    // строковый литерал по умолчанию константен
    const char* p = "literal";

    // для формирования неконстантной строки используйте массив
    char p_arr[] = "array";
    p_arr[0] = 'A';
    cout << sizeof(p_arr) << endl;

    // Non-const pointer is assigned to const (not otherwise!)
    int* ip = nullptr;
    m = ip;


    // Арифметика указателей:
    // Вычитание числа - перемещение по массиву назад
    // Прибавление числа - перемещение по массиву вперед
    // Вычитание указателей - расстояние между ними в элементах
    // Сложение указателей - не имеет смысла и запрещено

    // Результатом sizeof является беззнаковый int = size_t (cstddef)
    // Типом разности указателей - int = ptrdiff_t (cstddef)

    // Interesting notation of arrays
    int x = 2;
    cout << x["ABCDEF"] << endl;
    cout << 2["ABCDEF"] << endl;

    // error C2166: l-value specifies const object
    //x["ABCDEF"] = 'Z';

    // 1[array] == array[1], грубо говоря
    // 1 + *pointer == *pointer + 1
    // correct!
    x[p_arr] = 'Z';

    // Ещё со времён C, для массивов 1[x] тождественно равно *(1 + x)
    // ISO / IEC 9899:TC2 : 6.5.2.1 Array subscripting

    // Thus, &*E is equivalent to E (even if E is a null pointer), and &(E1[E2]) to ((E1)+(E2))

    // ...a pointer to a nonarray object behaves the same as a pointer to the
    // first element of an array of length one with the type of the object as its element type
    int* p1 = new int;
    int* p2 = new int;
    bool p1_less_p2 = less_ptr(p1, p2);
    delete p1;
    delete p2;
}

void show_pointer_arithmetic(){

    int ia[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int* ip = ia;

    // ++ moves to the pointee size!
    cout << "ip = " << ip << "; *ip = " << *ip << endl;
    ++ip;
    cout << "++ip = " << ip << "; *ip = " << *ip << endl;

    void* vp = reinterpret_cast<void*>(ip);
    cout << "vp = " << vp << endl;
    // ++vp; unable to ++ void* : error C2036: 'void *' : unknown size
}


//http://rsdn.ru/forum/cpp/5458937.flat#5458937
// Широко распространенное заблуждение.
// Почему-то людям кажется, что если массив неявно преобразуется к указателю,
// то адрес массива и сам массив — это одно и тоже
// Нет. Выражения &array и array имеют разные типы, не подлежащие даже сравнению.


// Для иллюстрации примера в show_array_pointers()
int stack_address_increment(int i)
{
    // значение на стеке по нулевому смещению от i (0)
    int a = 0[&i];

    // значение на стеке по смещению 1 от i (0xcccccccc в msvc)
    int b = 1[&i];

    // Memory dump 0[&i]       1[&i] ...
    // 0x002CF9E8  00 00 00 00 cc cc cc cc cc cc cc cc 00 00 00 00 00 fa 2c 00 c8 56 26 01 50
    // 0x002CFA01  fa 2c 00 89 64 26 01 01 00 00 00 28 81 49 00 70 6e 49 00 8c 37 2a 52 00 00

    // возвращаем данные за стековым значением,
    // переданным в функцию

    // выражения *(1 + &i) и 1[&i] ЭКВИВАЛЕНТНЫ
    return 1[&i];
    //return *(&i + 1);
}

// Размер массива
// Для иллюстрации примера в show_array_pointers()
template <typename T, std::size_t N>
inline std::size_t countof(T(&arr)[N]) {
    return N;
}


void show_array_pointers(){

    // 1.
    int i = 0;
    // найдем содержимое по смещению + 1 на стеке
    int ii1 = stack_address_increment(i);
    int ii2 = stack_address_increment(1);

    // 2.
    // Массив и указатель на массив - разные типы, их нельзя даже сравнивать
    // они молча приводятся друг к другу, но сравнивать их нельзя
    int array[42] = {};
    for (size_t s = 0; s < 42; ++s){
        array[s] = s;
    }

    int* ii3 = array;

    // Размер массива вшит в его тип!
    // cannot convert from 'int (*)[42]' to 'int *'
    // int* ii31 = &array;
    int(*ii4)[42] = &array;

    // Похоже, что это один и тот же указатель
    ptrdiff_t diff1 = reinterpret_cast<int*>(ii4)-ii3;

    std::cout << std::hex << ii4 << '-' << ii3 << '=' << std::dec << diff1 << '\n';

    // 3.
    // но если попробовать прибавить 1, получим интересный эффект
    int* ii5 = 1 + array;

    // cannot convert from 'int (*)[42]' to 'int *'
    // int* ii31 = 1 + &array;
    int(*ii6)[42] = 1 + &array;

    // diff2 равен 41. Т.е. (&array + 1) указывает на последний элемент + смещение 1
    // То есть указатель на массив указывает на "весь" массив, и +1 перемещает указатель за его пределы
    ptrdiff_t diff2 = reinterpret_cast<int*>(ii6)-ii5;

    std::cout << std::hex << ii6 << '-' << ii5 << '=' << std::dec << diff2 << '\n';

    // 4.
    // Модифицируем два предыдущих примера, чтобы получить размер массива
    // &array+1 будет указывать на элемент, следующий за массивом
    // &array[0] - на первый элемент
    ptrdiff_t diff3 = (1[&array] - &array[0]);

    // компилятор расставит приоритеты следующим образом:
    // 1( [&array] ) - &( array[0] ) == ( 1 + &array ) - &(0 + array)
    std::cout << std::hex << 1[&array] << '-' << &array[0] << '=' << std::dec << diff3 << '\n';

    // 5.
    // Но лучше использовать compile-time вычисление, тем более размер зашит в тип
    std::cout << countof(array) << '\n';

    // 6. multi-dimensional pointer
    int m_arr[10][20] = {};
    int(*m_dim_ptr)[10][20] = &m_arr;
}


void show_references(){

    int ii = 10;

    // ссылку нужно инициализировать
    // перенаправить ее уже нельзя
    int& rr = ii;

    // ссылки на константы более предпочтительны
    // т.к. неконстантный объект более подвержен ошибкам
    const int ic = 0;
    const int& rc = ic;

    // ссылки можно возвращать - тогда возвращающая функция будет lvalue

    //Когда встречается конструкция T& ref = fun()
    //- ожидается, что fun() возвращает ссылку, живущую достаточно долго(например, some_container.front())
    //- ad - hoc - полиморфизм(т.е.мы хотим создать автоматический объект, но не можем явно указать его тип)

    // Const ref makes rvalue object live the same time as ref
    const long& lr = 12;
}

// This is correct.  The else actually matches with the second if (C++ Standard 6.4.1/1).
// show_ifs(-1) ret 0
// (clang warning)
int show_ifs(int x){
    if (x > 0)
    if (x > 100)
        return 1;
    else
        return 2;
    return 0;
}

int my_negate(int i){
    return ~(i - 1);
}

int my_abs(int i){
    // x32 specific!
    return (i & 0x10000000) ? my_negate(i) : i;
}

// для положительных чисел битовый сдвиг числа вправо на n равносилен целочисленному делению на 2^n
// Аналогично, битовый сдвиг влево на n бит равносилен(для положительных чисел) умножению на 2^n
int divide_2(int i){
    return i >> 1;
}

bool raised_to2(int x){

    const int intsize = sizeof(int)* 8;
    cout << "x = \t\t" << bitset<intsize>(x) << '\n';
    cout << "(x - 1) = \t" << bitset<intsize>(x - 1) << '\n';
    cout << "(x - 1) & x = \t" << bitset<intsize>((x - 1) & x) << '\n';

    if (!((x - 1) & x))
        return true;
    return false;
}

int multiply_2(int i){
    return i << 1;
}

void show_integers(){

    // !!! x86 of today use little - endian storage for all types of data(integer, floating point, BCD)

    const int intsize = sizeof(int)* 8;
    // Битовое представление 1
    int i = 1;
    cout << bitset<intsize>(i) << '\n';

    // Битовое представление 2^8
    i = pow(2, 8);
    cout << bitset<intsize>(i) << '\n';

    // Битовое представление 2^16
    i = pow(2, 16);
    cout << bitset<intsize>(i) << '\n';

    // Битовое представление -1
    i = -1;
    cout << bitset<intsize>(i) << '\n';

    // Битовое представление -2
    i = -2;
    cout << bitset<intsize>(i) << '\n';

    // Проверим ручной negate
    i = my_negate(1);
    i = my_negate(-1);
    i = my_negate(-2);

    // Проверим ручной abs
    i = my_abs(1);
    i = my_abs(-1);
    i = my_abs(-2);

    // Проверим ручное деление-умножение на 2
    i = divide_2(10);
    i = divide_2(64);
    i = multiply_2(10);
    i = multiply_2(64);

    bool r = raised_to2(2);
    r = raised_to2(4);
    r = raised_to2(8);
    r = raised_to2(16);
    r = raised_to2(24);

    unsigned int i1 = 1;
    int j1 = -1;
    // Секция Unreachable не достигается, т.к. uint -1 -> maxint
    if (j1 < i1){
        cout << "Unreachable" << endl;
    }
}

void show_close_enough(){

    // nextafter() returns the next representable value after x in the direction of y
    double d = 1.0;
    double e = nextafter(d, 1000.0);
    if (close_enough(d, e))
        cout << d << " == " << e << endl;


    d = 0.00000000000000000001;
    e = nextafter(d, 1.0);
    if (close_enough(d, e))
        cout << d << " == " << e << endl;
}

void show_fast_float2int(){
    float f = 1.0;
    int i = fast_float2int(f);
    int i1 = fast_float2int_debug(f);
    cout << f << " -> " << i << endl;


    f = 1.1;
    i = fast_float2int(f);
    i1 = fast_float2int_debug(f);
    cout << f << " -> " << i << endl;

    f = 1.5;
    i = fast_float2int(f);
    i1 = fast_float2int_debug(f);
    cout << f << " -> " << i << endl;

    f = 2.0;
    i = fast_float2int(f);
    i1 = fast_float2int_debug(f);
    cout << f << " -> " << i << endl;

    f = 10.0;
    i = fast_float2int(f);
    cout << f << " -> " << i << endl;

    double d = 1.0;
    i = fast_double2int(d);
    cout << d << " -> " << i << endl;
}

void show_fast_sqrt(){

    float f = 4.0;
    float i = quick_rsqrt(f);
    cout << "sqrt " << f << " = " << 1 / i << endl;

    f = 16.0;
    i = quick_rsqrt(f);
    cout << "sqrt " << f << " = " << 1 / i << endl;

    f = 2.0;
    i = quick_rsqrt(f);
    cout << "sqrt " << f << " = " << 1 / i << endl;

    f = -1.0;
    i = quick_rsqrt(f);
    cout << "sqrt " << f << " = " << 1 / i << endl;
}

void show_shift(){
    long long l = 0;
    for (size_t i = 0; i < sizeof(long long)* 8; ++i){
        // маска обязательно должна соответствовать типу
        l = 1075LL << i;
        cout << " magic =  " << bitset<sizeof(l)* 8>(l) << endl;
    }
}

//Почему rand() - очень медленная функция ?
//Я имею в виду вызов Enter / LeavCriticalSection или что - то такого же,
//что блокирует шину данных, когда зовём rand при линковке с многопоточной библиотекой.
//Если перенести этот код на многопроцессорную машину, где лок шины данных будет настоящий,
//а не "понарошку" - то я не знаю сколько будет.Вот код для тестирования:
void show_rand(){
    volatile int x;
    int n = 10000000;
    clock_t t = clock();
    for (int i = 0; i < n; ++i) x = rand();
    std::cout << int(double(n) * CLOCKS_PER_SEC / (clock() - t));
}



int main(){

    show_pointer_arithmetic();

    show_bits();
    show_integers();
    show_shift();

    show_floating_point();

    show_universal_fast_float2int();

    show_cmath_fpoint_operations();

    // does not work as expected
    show_fp_coltrol_noexcept();
    show_fp_coltrol();

    show_close_enough();
    show_fast_float2int();
    show_fast_sqrt();

    // example from rsdn
    show_array_pointers();

    show_enumerations();
    show_name_convensions(5);
    show_pointer();
    show_references();
    show_ifs(-1);
    show_rand();
    show_hamming_weight();
    show_MXCSR_registry();
    return 0;
}
