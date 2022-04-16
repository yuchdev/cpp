#include <iostream>
#include <map>
#include <set>

// ------------ map ------------ 
// Удобно определять typedef для объявлений map
typedef map<int, string> is_map;
typedef map<int, string>::iterator is_map_it;

typedef map<string, int, no_case<string> > si_nocase_map;
typedef map<string, int, no_case<string> >::iterator si_nocase_map_it;

template <typename TMap, typename TKey, typename TVal>
void _insert_to_map(TMap& m1, const TKey& k, const TVal& s){

	// Ассоциативный массив требует, чтобы для ключа 
	// существовала операция "меньше"
	
	// Возвращаемая пара содержит итератор на вставленную строку
	// и логический флаг, означающий успешную вставку
	pair< typename TMap::iterator, bool > p = m1.insert( make_pair(k,s) );
	if(p.second == true){
		cout << "Insert success" << endl;
	}
	else{
		cout << "Such key is already defined" << endl;
	}
}

void show_map()
{
	string s1("onotole");
	string s2("chake");
	is_map m1;

	// map требует наличия у элемента операции <
	// unordered(hash) map требует наличия  !=

	// ключи map (и элементы set) напрямую менять нельзя
	// нужно удалить старый и вставить новый

	_insert_to_map<is_map, int, string>(m1,1,s1);
	_insert_to_map<is_map, int, string>(m1,1,s1);

	// Мап с предикатом no_case
	si_nocase_map ncm1;
	_insert_to_map<si_nocase_map, string, int>(ncm1,s1,1);
	_insert_to_map<si_nocase_map, string, int>(ncm1,"Onotole",2);
	_insert_to_map<si_nocase_map, string, int>(ncm1,"chake",3);
	_insert_to_map<si_nocase_map, string, int>(ncm1,"Chake",4);

	pair<is_map_it, bool> p = m1.insert( make_pair(2,"chake") );
	p = m1.insert( make_pair(2,"chake2!") );

	// Предикат сортировки может быть передан параметром конструктора
	// Это позволяет задавать предикат в runtime
	si_nocase_map pred;
	map<string, int, si_nocase_map> mm(pred);

	// При вставке элемента можно указать, после какого элемента его лучше вставить
	// При больших объемах данных это может быть полезно
	m1.insert( p.first, make_pair(3,"leonide") );

	// Итерация производится по порядку сортировки
	print_map(m1);

	// тип map'а - всегда pair<>
	// в инициализаторе pair возможны преобразования
	pair<char, int> p1('a', 1);
	pair<char, double> p2(p1);

	// обращение по индексу
	string s = m1[2];

	// Когда ключ по операции [] не находится,
	// создается значение по умолчанию
	// создать новый индекс с пустым значением
	s = m1[3];
	
	// создать новый индекс со значением
	m1[4] = "leonide";

	// Если у элемента пары нет конструктора по умолчанию,
	// его всегда нужно инициализировать явно

	// поиск итератора по значению кюча
	map<int, string>::iterator it = m1.find(2);
	
	// первый элемент с ключом больше 1/меньше= 2
	it = m1.upper_bound(1);
	it = m1.lower_bound(2);
	
	// проверка если ничего не найдено
	if(it == m1.end()){
		m1[2] = string("onotole");
	}

	// Элементы можно удалять
	// - по одному
	m1.erase( m1.find(1) );

	// - диапазонами [)
	m1.erase( m1.find(2), m1.find(3) );
	// после вызова erase() итераторы, указывавшие на них,
	// инвалидируются

	size_t sz = m1.size();
	size_t msz = m1.max_size();
	// map и set предоставляют сильную гарантию исключений,
	// кроме вставок нескольких элементов (какое-то количество может вставиться)
}

// ------------ set ------------ 
void show_set(){
	// Контейнер с уникальными элементами
	// Можно представить как map только с ключами
	// Можно использовать как аналог unique()
	
	// Вставим из вектора с повторяющимися элементами
	
	set<string> st;

}

// ------------ multiset, multimap ------------ 
void show_multimap(){
	typedef multimap<int, string> is_mmap;
	typedef multimap<int, string>::iterator is_mmap_it;

	is_mmap m2;

	m2.insert( make_pair(1,"onotole") );
	m2.insert( make_pair(1,"chake") );
	m2.insert( make_pair(2,"") );
	m2.insert( make_pair(2,"leonide") );

	// Первый элемент с ключом 2
	is_mmap_it p1 = m2.lower_bound(2);

	// Следующий за последним элементом с ключом 1
	p1 = m2.upper_bound(1);

	// пара итераторов ограничивающих все элементы с ключом 2
	pair<is_mmap_it, is_mmap_it> pm = m2.equal_range(2);

	// количество элементов с ключом 2
	size_t sz = m2.count(2);
}

void show_multiset(){
	// Множество повторяющихся элементов. Такая куча.

	// todo : сделать с предикатом!
	multiset<string> ms;

}

