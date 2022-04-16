#pragma once
#include <cstring>
// ------------------- Отличия от шаблона класса -------------------
// - возможна перегрузка с обычными функциями
// - возможен вывод аргументов
// - невозможны параметры по умолчанию
// - невозможна частичная специализация

// ------------------- Перегрузка, специализация -------------------
// обычная перегруженная функция (нужна для примера, 
// должен быть определен перед шаблоном для корректной перегрузки)
inline const int max(const int a, const int b){
	return (a > b) ? a : b;
}

// При пергрузке предпочтение отдается функциям

// typename можно заменить на class, но так лучше не делать
// Если параметр шаблона - тип, то используйте typename 
// typename T - общепринятая форма
// T - параметр шаблона, a, b - параметры вызова

template <typename T>
inline const T& max(const T& a, const T& b){
	return (a > b) ? a : b;
}

// Специализация для строк (лексиграфический предикат)
template <>
inline const char* const& max(const char* const& a, const char* const& b ){
	return ( strcmp(a, b) < 0 ) ? a : b;
}


// Шаблонные функции м.б. встраиваимыми (inline)

// Одна специализация считается более специализированной чем другая,
// если каждый список аргументов соответствует первой специализации
template <typename T> class Vector;
template <typename T> class Vector<T*>;
template <> class Vector<void*>;


// ------------------- Указатели на функции и методы -------------------

void first_func(int i){
	cout << "First func: " << i << endl;
}

void second_func(int i){
	cout << "Second func: " << i << endl;
}

// введем указатель на функцию void void_func_p(int);
typedef void (*void_func_p)(int);

// И указатель на методы, статический и обычный
struct holder{
	void method(int i){
		cout << "Method: " << i << endl;
	};
	static void stat_method(int i){
		cout << "Static method: " << i << endl;
	}
};

typedef void (holder::* method_p)(int);

void show_func_pointers(){

	// Указатель на обычную функцию
	void_func_p f = &first_func;
	f(5);
	f = &second_func;
	
	// Вызов функции по указателю
	f(6);
	(*f)(7);

	cout << "sizeof f: "<< sizeof(f) << endl;

	// Указатель на метод
	method_p m = &holder::method;
	holder h;

	cout << "sizeof m: "<< sizeof(m) << endl;
	
	// Вызов метода по указателю - операция .* или ->*
	(h.*m)(8);

	// Возможен только при наличии объекта holder!

	// Вызов метода по указателю для динамического объекта
	holder* ph = new holder();
	(ph->*m)(9);
	delete ph;

	// Указатель на обычный метод нельзя присвоить указателю на функцию
	//m = f; (раскомментировать для ошибки)
	//f = &holder::method;

	// А на статический - можно

	// Указатель на статический метод
	void_func_p ms = &holder::stat_method;
	ms(10);

	// Также нельзя указатель на статический присвоить обычному
	//method_p m1 = ms;  (раскомментировать для ошибки)
}
