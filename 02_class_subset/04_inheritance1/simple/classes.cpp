#include "classes.h"


// --- Демонстрация методов при простом наследовании --- 

Employee::Employee(const char* n) : _name(n){

}

Employee::~Employee(){}

// Если в базовом классе нет конструктора по умолчанию,
// в производном он должен быть вызван явно

Manager::Manager(const char* n, short l , int s) 
	// В производном списке инициализации нельзя инициализировать члены базовых классов...
	: Employee(n), _level(l)/*, _salary(s)*/{	

	// ... но можно в теле конструктора! - если они protected
	Employee::_salary = s;

	// т.к. поле _salary называется так же, как и в базовом - 
	// оно остается неинициализированным
	_salary = s;
}

Manager::~Manager(){}

void Employee::show_name(){
	cout << _name << endl;
}

void Manager::show_level(){
	cout << _level << endl;
}

void Manager::show_name(){
	cout << "Manager: ";
	// Явный вызов метода базового класса
	Employee::show_name();
}


