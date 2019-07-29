#include <iostream>
#include <cmath>
#include <bitset>
#include <ctime>
#include <cstddef>

// TODO: alignment
// https://stackoverflow.com/questions/5435841/memory-alignment-in-c-structs
// https://en.cppreference.com/w/c/language/object

// Important information
// Agreements about types size in C++:
// 1 = sizeof(char) <= sizeof(short) <= sizeof(int) <= sizeof(long)
// 1 <= sizeof(bool) <= sizeof(long)
// sizeof(char) <= sizeof(wchar_t) <= sizeof(long)
// sizeof(float) <= sizeof(double) <= sizeof(long double)
// sizeof(N) <= sizeof(signed N) <= sizeof(unsigned N)

// 1. Facts about signed and unsigned arithmetic
void show_unsigned()
{
    unsigned int i1 = 1;
    int j1 = -1;
    // Unreachable section will never be reached, because uint(-1) being casted to maxint
    if (j1 < i1) {
        std::cout << "Unreachable;";
    }
}

// 1. What is the size of enum
// TODO: find in Standard

enum enum1{ dark, light };		// 0:1
enum enum2{ a = 3, b = 9 };		// 0:15
enum enum3{ c = 3, d = 900 };	// 0:1024
enum enum4{ e = 3, f = 5000000000 };	// out of range 0:2^32

void enum_size()
{
    enum1 e1 = enum1(0);
    enum1 e2 = enum1(1);

    // UB
    enum1 e3 = enum1(20); 

    // enum size is platform-dependent
    size_t sz = sizeof(enum1);
    sz = sizeof(enum2);
    sz = sizeof(enum3);

    // out if range
    enum4 e4 = enum4::f;
    sz = sizeof(e4);
    long long l = e4;
}

static int g_global = 5;

// 2. How to compare pointers
void compare_pointers_1()
{
    int a{ 10 }, b{ 20 };
    int* pa = &a;
    int* pb = &b;
    if (a < b) {
        std::cout << "Really, what do you expect here?\n";
    }
    // technically may be correct, but according to the Standard it's wrong
}

// Detailed explanation: there are two ways to compare pointers in C++:
// * equality operators, which is != and ==
// * more-less operators, < <= > >=

// TODO: Standard chapter
// First operators allowed in any case
// Second make sense ONLY if two pointers belong to the same address space
// Otherwise we meet Unspecified Behavior (Not Undefined!)

// Right way to compare pointers is std::less() function, which fives guarantees for correctness
// https://en.cppreference.com/w/cpp/utility/functional/less
// We can even create template function for pointers only
template<typename T> 
bool less_ptr(T* a, T* b)
{
    return std::less<T*>()(a, b);
}

void compare_pointers_1()
{
    int a{ 10 }, b{ 20 };
    int* pa = &a;
    int* pb = &b;
    if (less_ptr(pa, pb)) {
        std::cout << "Now I feel safe comparing my pointers!\n";
    }
    // Example where this approach may be necessary, is segment memory model
    // It used to be model for 16-bit CPU architectures, where memory was a pair of 
    // [selector + offset]
    // Let's imagine, we have some embedded architecture with the same memory model
    // In this case, plain compare (pa < pb) between different segments make no sense!
}

// 3. Facts about pointers
void pointers_facts(){

    // x86 of today use little - endian storage for all types of data(integer, floating point, BCD)


    // Every object has an address
    // Guaranteed that there are no objects with NULL address
    int a{};
    int* ptr_a = &a;

    // Standard output prints pointers as hexadecimal
    std::cout << "ptr_a = " << ptr_a << '\n';

    // Always take notice about constant semantic of POINTER or OBJECT,
    // which pointer points on

    // pointer is constant, integer does not
    int* const ptr1 = &a;

    // Integer is constant, pointer does not
    const int* ptr2 = &a;
    // .. basically the same
    int const* ptr3 = &a;

    // Both pointer and integer are constant
    const int* const ptr4 = &a;

    // Pointer arithmetic
    int some_array[10] = {0,1,2,3,4,5,6,7,8,9};
    int* ptr_to_first = some_array;


    // 1.pointer + integer: moves forward from the pointer
    int* fifth_element = ptr_to_first + 5;

    // 2.pointer - integer: moves backward from the pointer
    int* second_element = ptr_to_first -3;

    // 3.pointer - pointer: is important and means distance between pointers
    // Pointer distance has a special type, defined in <cstddef> include file
    ptrdiff_t some_distance = fifth_element - second_element;

    // 4.++ or -- moves to the pointee size!
    ++ptr_to_first; // moves 4 bytes forward!
    ptrdiff_t other_distance = ptr_to_first - some_array;
    
    // Exception from this rule is void*
    void* vp = reinterpret_cast<void*>(some_array);
    // ++vp; there's no increment/decrement for void*


    // 4.pointer + pointer: why do we even need this?

    // A pointer to a non-array dynamic object behaves the same as a pointer to the
    // first element of an array of length one
    int* p1 = new int(1);
    int* p2 = new int(2);
    bool p1_less_p2 = less_ptr(p1, p2);
    delete p1;
    delete p2;
}

// 4. Facts about arrays
void array_facts()
{
    // typedef for array
    typedef int Vector3D[10];
    Vector3D numbers = { 1, 2, 3 };

    // string literal is an array
    const char* str = "Some string";
    std::cout << "Size of the string '" << str << "' is " << sizeof(str);

    // string literal is constant by default
    char* const_string = "Other string";
    // p[0] = 'o' - can't touch this!

    // If you want to operate non-const string literal, use array instead
    char non_const_string[] = "this string is non-constant";
    non_const_string[0] = 'T';
    std::cout << non_const_string << '\n';

    // Interesting notation of arrays
    int x = 2;
    std::cout << x["ABCDEF"] << '\n';
    std::cout << 2["ABCDEF"] << '\n';

    // Is this code correct?
    // Yes! // It is correct!

    // Sincs plain ANSI C times, for array x[1] is equal  to 1[x] is equal to *(1 + x)
    // ISO / IEC 9899:TC2 : 6.5.2.1 Array subscripting
    // Thus, &*E is equivalent to E (even if E is a null pointer), and &(E1[E2]) to ((E1)+(E2))

    // Simply says, 1[array] == array[1]
    // 1 + *pointer == *pointer + 1

    // However, don't try to change it, since l-value specifies constant object
    //x["ABCDEF"] = 'Z'; error C2166
}


//http://rsdn.ru/forum/cpp/5458937.flat#5458937
// Широко распространенное заблуждение.
// Почему-то людям кажется, что если массив неявно преобразуется к указателю,
// то адрес массива и сам массив — это одно и тоже
// Нет. Выражения &array и array имеют разные типы, не подлежащие даже сравнению.

// 5. Array and pointer are not the same!
// Some developers think if array could be implicitly casted to pointer,
// it means they array address and array itself basically the same. Right?..
// Wrong! 
// Expressions 'array' and '&array' have different types and could not even be compared


// Helper function 
int stack_address_increment(int i)
{
    // 0 offset from i (i itself)
    int a = 0[&i];

    // 1 offset from i (usually some garbage)
    int b = 1[&i];

    // Memory dump 0[&i]       1[&i] ...
    // 0x002CF9E8  00 00 00 00 cc cc cc cc cc cc cc cc 00 00 00 00 00 fa 2c 00 c8 56 26 01 50
    // 0x002CFA01  fa 2c 00 89 64 26 01 01 00 00 00 28 81 49 00 70 6e 49 00 8c 37 2a 52 00 00

    // returns address where object of type int ends
    return 1[&i]; //which is equal to *(&i + 1);
}

// Array size
template <typename T, std::size_t N>
inline std::size_t arraysize(T(&arr)[N]) {
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

    // Reference must be initialized, and could not be re-initialized
    int& rr = ii;

    // ссылки на константы более предпочтительны
    // т.к. неконстантный объект более подвержен ошибкам
    const int ic = 0;
    const int& rc = ic;

    // When we meet function T& ref = boo(),
    // we expect that object lives long enough not to create "dead" reference
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
