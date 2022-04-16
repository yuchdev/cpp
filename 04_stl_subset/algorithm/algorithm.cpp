#include "algorithm.h"
#include "predicates.h"
#include <set>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <cstdlib>
#include <cctype>       // std::tolower
#include <cstring>

using namespace std;
typedef vector<int> v_int;
typedef vector<int>::iterator v_iit;

template<typename T, template <typename ELEM, typename = std::allocator<ELEM> > class CONT >
void print_container(const CONT<T>& c){
    typename CONT<T>::const_iterator it = c.begin();
    while (it != c.end()){
        cout << (*it) << ' ';
        ++it;
    }
    cout << endl;
}


/// Pair values extraction functor
/// Extracts 1-st value (key)
template <typename Pair> struct select1st {
    typedef Pair argument_type ;
    typedef typename Pair::first_type result_type ;
    const result_type& operator()(const argument_type& p) const {
        return p.first ;
    }
};

/// Pair values extraction functor
/// Extracts 2-nd value
template <typename Pair> struct select2nd {
    typedef Pair argument_type ;
    typedef typename Pair::second_type result_type ;
    const result_type & operator()(const argument_type& p) const {
        return p.second ;
    }
} ;

// --------------- Немодифицирующие алгоритмы ---------------
// for_each
// find
// count
// равенство и несовпадение
// поиск
void show_non_modif() {
    int arr[] =  { 0,4,1,9,3,7,3,7,3,8 };
    int arr1[] = { 7,3,7,4,5,2,8,8,0,4 };
    vector<int> vi( arr, arr + sizeof(arr)/sizeof(int) );
    vector<int> vi1( arr1, arr1 + sizeof(arr1)/sizeof(int) );

    // --- find, find_if ---
    v_iit it = find(vi.begin(), vi.end(), 7 );

    // Предикатом может быть все, к чему применяется операция ()
    it = find_if(vi.begin(), vi.end(), my_less_predivate(7) );

    // Суффикс _if добавлен из-за сложности перегрузки шаблона алгоритма
    // Например, если на вход поступает контейнер bool

    // --- for_each ---
    // Использование for_each() в качесстве аккумулятора с соответствующим функтором
    // for_each() - немодифицирующий алгоритм!
    sum_accum<int> s;
    s = for_each(vi.begin(), vi.end(), std::ref(s) );
    int sum = s.res;

    // --- find_first_of ---
    // Находит в последовательности первый элемент,
    // который имеется во второй последовательности
    it = find_first_of(vi.begin(), vi.end(), vi1.begin(), vi1.end() );

    // --- adjacent_find ---
    // Ищет дубли и возвращает итератор на первый элемент
    it = adjacent_find( vi1.begin(), vi1.end() /*, pred*/ );

    // --- count, count_if ---
    long long c = count(vi.begin(), vi.end(), 7 );
    c = count_if(vi.begin(), vi.end(), my_less_predivate(7) );

    // --- mismatch ---
    // Сравнивает 2 последовательности и находит первое несовпадение
    // Конец второй последовательности не определен, считается, что она не меньше первой
    pair<v_iit, v_iit> p1 = mismatch(vi.begin(), vi.end(), vi1.begin());

    // --- equal ---
    // просто сравнивает 2 последовательности, не указывая место несовпадения
    bool b = equal( vi.begin(), vi.end(), vi1.begin() );

    // --- search, find_end, search_n ---
    int arr2[] = {9,3,7};
    size_t arr2_sz = sizeof(arr2)/sizeof(int);

    // search - находит подпоследовательность в последовательности
    // и возвращает итератор на ПЕРВЫЙ совпадающий элемент
    // Например: Находит в векторе vi массив arr2
    v_iit it2 = search(vi.begin(), vi.end(), arr2, arr2 + arr2_sz);

    // find_end - находит подпоследовательность в последовательности
    // и возвращает итератор на первый совпадающий элемент
    // Например: Находит в векторе vi массив arr2, но возвращает итератор на конец
    it2 = find_end(vi.begin(), vi.end(), arr2, arr2 + arr2_sz);

    // Находит в vi1 последовательность из 2 восьмерок
    it2 = search_n(vi1.begin(), vi1.end(), 2, 8);
}

// --------------- Модифицирующие алгоритмы ---------------
// copy, copy_if, copy_backward
// transform
// replace, replace_if, replace_copy, replace_copy_if
// remove,remove_copy,remove_copy_if
// fill,fill_n
// generate,generate_n
// unique,unique_copy
// reverse,reverse_copy
// rotate,rotate_copy
// random_shuffle
// swap,iter_swap,swap_ranges
void show_modif(){

    int arr[] =  { 0,4,1,9,3,7,3,7,3,8 };
    int arr1[] = { 7,3,7,4,5,2,8,8,0,4 };
    vector<int> vi( arr, arr + sizeof(arr)/sizeof(int) );
    vector<int> vi1( arr1, arr1 + sizeof(arr1)/sizeof(int) );

    // Модифицирующие алгоритмы необязательно пишут изменения
    // во входную последовательность

    // Вместо вставки и изменения, элементы переписываются в другое место контейнера,
    // а потом удаляются/изменяются вручную

    // Сделано так потому, что алгоритм ничего не знает об изменяемом контейнере,
    // и не сможет, например, изменить его параметр размера

    // --- copy, copy_backward ---
    // Копирует куда угодно, не обязательно в конейнер. Например, в поток.
    copy(vi.begin(), vi.end(), ostream_iterator<int>(cout));
    cout << endl;

    // Использование вставок не позволяет выйти за пределы контейнера
    // и позволяет удобно записывать в конец
    vector<int> v_copy;
    copy(vi.begin(), vi.end(), back_inserter(v_copy));

    // Входная и выходная последовательности могут перекрываться
    // Если начало ВЫХОДНОЙ последовательности находится ВНУТРИ ВХОДНОЙ
    // необходимо использовать copy_backward - копирование с конца
    // Например, скопировать все элементы в тот же контейнер со сдвигом на 2
    // (???)
    //copy_backward( v_copy.begin(), v_copy.end(), v_copy.begin() + 2 );

    // copy_if в стандартной поставке STL отсутствует
    //copy_if(vi.begin(), vi.end(),back_inserter(v_copy), less<int>(7) );

    // --- transform ---
    // Создает выходную последовательность, применяя операцию ко входной
    // (или к 2-м входным)

    // С его помощью можно очистить полиморфный контейнер, изменить case_sens и пр.

    vector<int> vcolors;
    vector<circle> vshape1;
    vshape1.push_back(circle(10));
    vshape1.push_back(circle(20));
    vshape1.push_back(circle(30));

    // Сформируем контейнер цветов из контейнера фигур
    transform( vshape1.begin(), vshape1.end(), back_inserter(vcolors), mem_fun_ref( &shape::color ) );

    // Есть версия, прининающая 2 последоватльности
    // Демонстрация умножения двух векторов и записи в третий
    vector<int> res;
    transform( vi.begin(), vi.end(), vi1.begin(),
        back_inserter(res), multiplies<int>() );

    // Более 2 последовательностей не принимает

    // --- unique,unique_copy ---
    // unique - переносит все уникальные элементы в конец (или начало???) последовательности,
    // и возвращает итератор на конец последовательности дубликатов
    vi.clear();
    int arr2[] =  { 0,4,1,1,1,7,3,3,3,8,6,2,9,4,4,7 };
    vector<int> vi2( arr2, arr2 + sizeof(arr2)/sizeof(int) );
    sort(vi2.begin(), vi2.end());
    v_iit vit1 = unique( vi2.begin(), vi2.end() );

    // удалять их придется вручную
    vi2.assign( arr2, arr2 + sizeof(arr2)/sizeof(int) );
    sort(vi2.begin(), vi2.end());

    // unique_copy - формирует новую последовательность (старую не трогает)
    unique_copy( vi2.begin(), vi2.end(), back_inserter(vi) );

    // --- replace, replace_if, replace_copy, replace_copy_if ---
    // Семейство этих алгоритмов заменяет в последовательности указанные значения

    // Заменить 7 на 8
    replace( vi.begin(), vi.end(), 7, 8 );

    // Заменить в vi 4 на 6 и вставить в вектор vi2 (старую не трогает)
    vi2.clear();
    replace_copy( vi.begin(), vi.end(), back_inserter(vi2), 4, 6 );

    // Заменить все <= 2 на 3
    replace_if( vi.begin(), vi.end(), bind2nd(less_equal<int>(), 2) , 3 );

    // Заменить в vi1 все <= 2 на 3 и вставтить в вектор vi  (старую не трогает)
    replace_copy_if( vi1.begin(), vi1.end()
        , back_inserter(vi)
        , bind2nd(less_equal<int>(), 2)
        , 3 );

    // --- remove,remove_copy,remove_copy_if ---
    // Семейство remove перемещает элементы (в последовательности или вне ее),
    // основываясь на значении или предикате
    // Простой remove просто записывает в начало (или конец???) последовательности (удалять вручную)
    print_container(vi);
    v_iit it = remove( vi.begin(), vi.end(), 5 );
    print_container(vi);
    // todo : print vector

    // remove_if - по предикату
    it = remove_if( vi.begin(), vi.end(), bind2nd( less<int>(), 4 ) );
    print_container(vi);
    // todo : print vector

    // remove_copy_if - элемент помещается на выход,
    // если он не удовлетворяет условию по предикату
    vector<int> vi3;
    remove_copy_if( vi1.begin(), vi1.end(), back_inserter(vi3), bind2nd( less<int>(), 2 ) );
    print_container(vi3);

    // --- fill,fill_n ---
    // Семейство функций заполняет значениями контейнер

    // Первым 2 элементам присвоить 5
    fill(vi3.begin(), vi3.begin() + 2, 5 );

    // То же самое с применением fill_n
    fill_n(vi3.begin(), 2, 6 );

    // --- generate,generate_n ---
    // Семейство функций, генерирующих значения контейнера из функции
    // Генерация случайных чисел
    generate(vi3.begin(), vi3.end(), rand );

    // Вывод 20 случайных чисел
    generate_n( ostream_iterator<int>(cout), 20, rand );

    // --- reverse,reverse_copy ---
    // reverse меняет порядок последовательности на обратный
    reverse( vi.begin(), vi.end() );

    // reverse_copy производит новую последовательность в обратном порядке
    vector<int> vi5;
    reverse_copy( vi.begin(), vi.end(), back_inserter(vi5) );

    // --- rotate,rotate_copy ---
    // rotate циклически сдвигает элементы, пока второй аргумент функции
    // не окажется первым аргументом последовательности
    int arr3[] = {7,2,89,3,8,6,2,6};
    size_t sz3 = sizeof(arr3)/sizeof(int);

    // Например циклический сдвиг массива на 3 влево
    rotate(arr3, arr3 + 3, arr3 + sz3);

    // rotate_copy
    // Копирование сдвинутого на 5 массива в vi5
    vi5.clear();
    rotate_copy(arr3, arr3 + 5, arr3 + sz3, back_inserter(vi5));

    // --- random_shuffle ---
    // Перемешивание контейнера
    random_shuffle(arr3, arr3 + sz3/* , rand_gen()*/ );

    // --- swap,iter_swap,swap_ranges ---
    // swap - переставляет 2 объекта местами
    swap(vi, vi1);

    // iter_swap
    // swap - переставляет 2 элемента местами (необязательно в одном контейнере) (??? в одном)
    swap(*vi.begin(), *vi1.begin());

    // swap_ranges
    // Меняет местами 2 диапазона равного размера, напрмер
    // первые 2 элемента vi1 и vi2
    swap_ranges(vi1.begin(), vi1.begin() + 2, vi2.begin());
}

// --------------- Сортировка ---------------
//  sort,stable_sort
//  parital_sort,parital_stable_sort
//  nth_element, binary_search
//  merge,inplace_merge
//  lower_bound, upper_bound
//  partition,stable_partition
void show_sort(){
    int arr[] =  { 0,4,1,9,3,7,3,7,3,8 };
    int arr1[] = { 7,3,7,4,5,2,8,8,0,4 };
    vector<int> vi( arr, arr + sizeof(arr)/sizeof(int) );
    vector<int> vi1( arr1, arr1 + sizeof(arr1)/sizeof(int) );

    // --- sort,stable_sort ---
    // Требуют итераторов с произвольным доступом
    sort(vi.begin(), vi.end());

    // stable_sort сохраняет порядок одинаковых элементов
    stable_sort(vi1.begin(), vi1.end());

    vi.assign( arr, arr + sizeof(arr)/sizeof(int) );
    vi1.assign( arr1, arr1 + sizeof(arr1)/sizeof(int) );

    // --- parital_sort,partial_sort_copy ---
    // Посдедовательность разбивается на 2 части, и сортировка продолжается
    // до тех пор, пока не будет отсортировани первая часть
    partial_sort(vi.begin(), vi.begin() + 3, vi.end());

    // Посдедовательность разбивается на 2 части, копируется в выходную последовательность
    // и сортировка продолжается до тех пор, пока не будет отсортировани первая часть
    partial_sort_copy(vi.begin(), vi.end(),vi1.begin(), vi1.begin() + 3);

    vi.assign( arr, arr + sizeof(arr)/sizeof(int) );
    vi1.assign( arr1, arr1 + sizeof(arr1)/sizeof(int) );
    // --- nth_element ---
    // Сортировка до тех пор, пока n-ый элемент не займет свое место
    // Полезно при вычислении медиан, процентилей и тп
    // Например, третий элемент
    nth_element(vi.begin(), vi.begin() + 3, vi.end());

    // --- 	binary_search, lower_bound, upper_bound ---
    // binary_search - поиск в отсортированной последовательности
    sort(vi.begin(), vi.end());
    bool b = binary_search(vi.begin(), vi.end(),1);

    // lower_bound и upper_bound - возвращают
    // верхнюю и нижнюю границу одинаковых элементов
    // отсортированной последовательности
    v_iit it = lower_bound(vi.begin(), vi.end(),7);
    it = upper_bound(vi.begin(), vi.end(),7);

    // --- merge,inplace_merge ---
    // Семейство функций слияния отсортированных последовательностей

    // merge
    // Слияние двух последовательностей
    sort(vi1.begin(), vi1.end());
    vector<int> res;
    merge( vi.begin(), vi.end(),vi1.begin(), vi1.end(),back_inserter(res) );

    // inplace_merge
    // Слияние двух отсортированных частей одной последовательности
    res.clear();
    res.reserve(100);	// чтоб не перераспределять память

    copy(vi.begin(), vi.end(), back_inserter(res));
    copy(vi1.begin(), vi1.end(), back_inserter(res));
    inplace_merge(res.begin(), res.begin()+vi.size() , res.end());

    // --- partition,stable_partition ---
    // Разделение контейнера - помещение всех элементов,
    // удовлетворяющих предикату, перед теми, которые ему не удовлетворяют
}


// --------------- Heap ---------------
// make_heap, sort_heap
// push_heap, pop_heap
void show_heap(){

    // на основе кучи строится priority_queue

    vector<int> v({ 0, 2, 5, 1, 5, 3, 8, 3, 6, 5 });
    print_container(v);

    // сделать из вектора кучу
    make_heap(v.begin(), v.end());
    print_container(v);

    // внести элемент в кучу
    // вносится всегда последний элемент
    v.push_back(3);
    push_heap(v.begin(), v.end());
    print_container(v);

    // вынести элемент из кучи
    // выносится тоже последний элемент
    pop_heap(v.begin(), v.end());
    v.pop_back();
    print_container(v);

    // отсортировать кучу (после сортировки последовательногсть перестает быть кучей)
    sort_heap(v.begin(), v.end());
    print_container(v);
}


// --------------- Min|Max ---------------
// min, max, max_element, min_element
// lexigraphical_compare
bool no_case(char c1, char c2){
    return std::tolower(c1) < std::tolower(c2);
}

void show_min_max(){
    // --- min, max, max_element, min_element ---
    double m = max(1.2, 2.5);
    const char* a = "aaa";
    const char* b = "bbb";
    string s( min(a, b) );
    // max/min элементы из пары или из контейнера
    // no comments

    const char* A = "AAA";

    // lexigraphical_compare - сравнение строк
    // true if the first range compares lexicographically less than than the second.
    // false otherwise(including when all the elements of both ranges are equivalent).
    bool diff = lexicographical_compare(a , a + strlen(a), b , b + strlen(b) );
    diff = lexicographical_compare(a, a + strlen(a), A, A + strlen(A), no_case);
}

// ------------------- Set -------------------

// --- includes ---
// --- set_union, set_intersection ---
// --- set_difference, set_symmetric_difference ---
void show_set(){
    // Операции с множествами имеют смысл только для отсортированных последовательностей
    // особенно эффективны с контейнерами set и multiset
    int arr[] =  { 0,4,1,9,3,7,3,7,3,8 };
    int arr1[] = { 7,3,7,4,5,2,8,8,0,4 };

    set<int> s( arr, arr + sizeof(arr)/sizeof(int) );
    multiset<int> ms( arr1, arr1 + sizeof(arr1)/sizeof(int) );
    set<int> s1(ms.begin(), ms.end());

    // --- includes ---
    // Проверяет, являются ли все члены первой последовательности
    // также членами второй
    bool b = includes(s.begin(), s.end(),ms.begin(), ms.end());

    // --- set_union, set_intersection ---
    // Объединение и пересечение соответственно
    vector<int> union_out;
    vector<int> intersection_out;
    set_union(s.begin(), s.end(), s1.begin(), s1.end(), back_inserter(union_out)); //(??? wtf)
    set_intersection(s.begin(), s.end(), ms.begin(), ms.end(), back_inserter(intersection_out));

    // --- set_difference, set_symmetric_difference ---
    vector<int> diff_out;
    vector<int> symm_out;

    vector<int> v1(arr, arr + sizeof(arr) / sizeof(int));
    vector<int> v2(arr1, arr + sizeof(arr1) / sizeof(int));
    sort(v1.begin(), v1.end());
    sort(v2.begin(), v2.end());

    // set_difference - последовательность входящих в первую, но не входящих во вторую элементов
    set_difference(v1.begin(), v1.end(), v2.begin(), v2.end(), back_inserter(diff_out));

    // set_symmetric_difference - последовательность входящих в только одну из последовательностей элементов
    set_symmetric_difference(v1.begin(), v1.end(), v2.begin(), v2.end(), back_inserter(symm_out));
}

// --------------- Permutations ---------------
// next_permutation, prev_permutation
void show_permutations(){

    // все возможные перестановки последовательности
    char v[] = "abcd";
    cout << endl << "start permutations:" << endl << v << '\t';
    while ( next_permutation(v, v + 4) ){
        cout << v << '\t';
    }
}

// --------------- C algorithms ---------------
// qsort, bsearch
int c_compare( const void* arg1, const void* arg2 ){
    /* Compare by subtracting (ret>0, ret==0, ret<0)*/
    return (*static_cast<const int*>(arg1)) - (*static_cast<const int*>(arg2));
}

void c_algorithms(){
    int arr[] =  { 0,4,1,9,3,7,3,7,3,8 };
    size_t sz = sizeof(arr)/sizeof(int);
    qsort( arr, sz, sizeof(int), c_compare );
    int key = 1;
    int* i = (int*)bsearch( (const void*)&key, (int*)arr, sz, sizeof(int), c_compare );
}
