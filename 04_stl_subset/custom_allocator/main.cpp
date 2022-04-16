#include <iostream>
#include <list>
#include <set>
#include <vector>
#include <cstdlib>
#include <iterator>
#include <memory>
#include "checked_iterator.h"
#include "custom_allocator.h"

using namespace std;

// В C++ запрещена модификация временного объекта
// Поэтому некоторые итераторы с автоматическим временем жизни модифицировать нельзя
// Например, sort(++a, b) - если a - указатель, но скомпилируется, если объект - наоборот


void show_iterator_traits(){
	// Итератор - это абстракция (то, что ведет себя как итератор - это итератор)
	// Чаще всего это указатель или объект

	// Виды итераторов:
	// Output
	// Input
	// Forward
	// Bidirectional
	// Random-access
	int a[] = {0,1,9,3,5,4,2,7,6,5};

	// вектор имеет Random Access итераторы
	vector<int> v1(a, a + sizeof(a)/sizeof(int));

	// а список имеет Bidirectional
	list<int> l1(a, a + sizeof(a)/sizeof(int));

	// Расстояние между итераторами
	// Для вектора
    typedef vector<int>::iterator::difference_type vect_int_dist;
	vect_int_dist vdist = distance(v1.begin(), v1.end());

	// обмен значений по итератору (первый с последним)
	vector<int>::iterator lst = v1.end() - 1;
	iter_swap(v1.begin(), lst);

	// Для списка
    typedef list<int>::iterator::difference_type list_int_dist;
	list_int_dist ldist = distance(l1.begin(), l1.end());
    cout << "Distance = " << ldist << endl;

	// в подобных алгоритмах также неявно передается объект "категории итератора"
	// он позволяет выбрать перегрузку, соответствующую типу итератора
	// Например, для RA это просто разность,
	// а для BI это пробежка от одного итератора до другого

	// Аналогично с операцией advance (аналог +=)
	// += 5 (то же что и operator[])
    const vect_int_dist run_v = 5;
    vector<int>::iterator v_b = v1.begin();
    advance(v_b, run_v);

	// пробежка по списку (аналог operator[] для std::list)
    const list_int_dist run_l = 5;
    list<int>::iterator l_b = l1.begin();
    advance(l_b, run_l);

	// выделяют также операторы-вставки
	// и обратные итераторы (рассмотрены в 18)
	// А также итераторы потоков (рассмотрены в 03)
	// С их помощью можно, например, удваивать каждую строку, читать каждую третью строку и тп
}

void show_checked_iterator(){

	// Конетйнеры с проверкой 
	Checked_container< vector<int> > v1;
	Checked_container< vector<int> > v2(10);

	Checked_container< list<int> > l1;
	Checked_container< list<int> > l2(10);

	// Итераторы с проверкой 
	try{
		// инициализация v2.begin()
		Checked_container< vector<int> >::iterator it(v2);
		while(true) it++;
	}
	catch(const std::exception& e){
		cout << "exception: " << e.what() << endl;
	}

}

void show_set_iterator(){
	set<int> s;

	// неудобный способ
	associative_insert_iterator< set<int> > aint(s);
	
	*aint = 1;
	++aint;
	*aint = 2;
	++aint;
	*aint = 3;
	++aint;

	// удобный способ
	associative_inserter(s) = 4;
	associative_inserter(s) = 5;

	int vals[] = {6,7,8,9};
	copy(vals, vals + sizeof(vals) / sizeof(int), associative_inserter(s));
}


void show_custom_allocator(){
	// объекты с разными распределителями в общем случае не должны смешиваться
	// например, list::splice с разными распределителями будет работать
	// но не слиянием списков, а поэлементной вставкой

	typedef std::vector<int, std::custom_allocator<int> > custom_vector;
	custom_vector v;
	v.push_back(1);
	v.push_back(2);
	v.pop_back();

	// в STL входят функции для низкоуровневой работы с 
	// пользовательскими контейнерами и алгоритмами
	// они предлагают сильную гарантию исключений

	// инициализация памяти значением
	int arr[10] = {};
	uninitialized_fill(arr, arr + sizeof(arr) / sizeof(int), 1);

	// инициализация n элементов значением
	uninitialized_fill_n(arr, 5, 2);

	// копирование интервала в память
	int arr2[10] = {};
	uninitialized_copy(arr, arr + sizeof(arr) / sizeof(int), arr2);

	// существует интератор для перебора неинициализированной памяти
	// raw_storage_iterator и ее инициализации T()

	// для краткосрочного использования памяти применяются:
	pair<int*, ptrdiff_t>p = get_temporary_buffer<int>(10);
	// возвращает пару - указатель на начало и размер

	// у деаллокации нет проверки на 0
	if (p.first != 0)
		return_temporary_buffer(p.first);
}

void show_c_memory(){

	// функции C для управления "сырой" памятью

	// выделим 100 байт
	void* m = malloc(100);

	// выделяет и обнуляет память для 100 int
	void* c = calloc(100, sizeof(int));

	// изменить размер до 200
	void* m1 = realloc(m, 200);
	if (m1 != NULL) {
		m = m1;
	}
	else {
		free(m);
		puts("Error (re)allocating memory");
	}
	free(m);
	free(c);
	// также есть 
	// memcpy() - копирование без перекрытия
	// memmove() - перемещение с перекрытием
	// memchr() - поиск бита по значению
	// memcmp(), memset()
}

int main(){

	show_iterator_traits();
	show_checked_iterator();
	show_set_iterator();
	show_custom_allocator();
	show_c_memory();

	return 0;
}
