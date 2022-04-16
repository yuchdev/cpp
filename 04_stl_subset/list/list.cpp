#include <list>

template<typename T, template <typename ELEM, typename = std::allocator<ELEM> > class CONT >
void print_container(const CONT<T>& c){
    typename CONT<T>::const_iterator it = c.begin();
	while( it != c.end() ){
		cout << (*it) << ' ';
		++it;
	}
	cout << endl;
}

template<typename TKey, typename TVal>
void print_map(const map<TKey, TVal>& m	) {
    typename map<TKey, TVal>::const_iterator it = m.begin();
	while( it != m.end() ) {
        cout << (*it).first << "=/t" << (*it).second;
		++it;
	}
	cout << endl;
}

// list не обладает рандомными итераторами, и функциями reserve() и capacity()
void show_list(){
	int arr[10] = {1,6,3,5,7,6,4,2,4};
	vector<int> vi1(10, 1);	

	list<int> l;
	list<int> l1;
	l.assign(arr, arr + sizeof(arr)/sizeof(int));
	l1.assign(vi1.begin(), vi1.end());
	print_container(l);
	print_container(l1);

	// Используйте встроенную сортировку списка вместо алгоритма
	// т.к. алгоритм требует рандомные итераторы
	l.sort();
	print_container(l);

	list<string> ls;
	list<string> ug;
	ls.push_back("onotole");
	ls.push_back("chake");
	ls.push_back("leonide");
	ug.push_back("ermakove");
	print_container(ls);

	// Операция "удалить и вставить"
	// Может принимать данные из списков
	// Дешевая при постоянной сложности, не выполняется копирование элементов

	// куда вставить, какой список вставить
	ls.splice(ls.begin(), ug);
	print_container(ls);

	// куда вставить, из какого списка, какой элемент
	ug.splice(ug.begin(), ls, ls.begin() );
	print_container(ug);

	// куда вставить, из какого списка, с какого по какой элемент 
	// (найдем первый элемент, у которого первая буква 'c' и вставим с начала по этот элемент)
	list<string>::iterator p = find_if(ls.begin(), ls.end(), initial('c'));
	ug.splice(ug.begin(), ls, ls.begin(), p );
	print_container(ug);

	// Так - ошибка, потому что не рандомный итератор
	//ug.splice(ug.begin(), ls, ls.begin(), ls.begin()+2 );

	// Вообще, с функцией splice надо быть осторожнее
	// при передаче итераторов - они должны указывать на один список и
	// быть валидны

	// операция с начальным элементом
	ug.push_front("gotyachke"); 
	// если планируется взаимозамена контейнеров,
	// то лучше предпочитать операции с конца - 
	// тогда список можно заменить, например, на вектор

	// обмен списков - также дешевая операция, не происходит копирования
	ug.swap(ls);

	// слияние отсортированных списков с сохранением порядка:
	l.merge(l1);

	// Можно сливать неотсортированные списки, но тогда порядок не гарантирован
	l1.assign(10,2);
	list<int> l3(arr, arr + sizeof(arr)/sizeof(int));
	// l1.merge(l3);  в VC вообще выдает ошибку

	// Используйте встроенное удаление
	l.remove(1);
	
	// в том числе с предикатом - например удалить нечетные
	l.remove_if(is_odd<int>());

	ls.push_back("onotole");
	ls.push_back("Onotole");
	ls.push_back("ermakove");
	
	// Сортировка с предикатом no_case
	ls.sort( no_case<string>() );

	// удалить последовательные повторы,
	// удовлетворяющие предикату
	ls.unique( initial2('o') );

	ls.unique(initial2('e'));

	// удалить все последовательные повторы
	l.unique();

	// перевернуть список
	ls.reverse();

	// первый и последний
	string first = ls.front();
	string last = ls.back();
	
	// Списки наиболее надежны в плане исключений
	// Базовую гарантию предоставляет только sort()
	// все остальные операции предоставляют сильную
	// При этом предикаты не должны кидать исключений
	
	// Итераторы списка всегда валидны
}

