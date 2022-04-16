#include "classes.h"


// --- ������������ ������� ��� ������� ������������ --- 

Employee::Employee(const char* n) : _name(n){

}

Employee::~Employee(){}

// ���� � ������� ������ ��� ������������ �� ���������,
// � ����������� �� ������ ���� ������ ����

Manager::Manager(const char* n, short l , int s) 
	// � ����������� ������ ������������� ������ ���������������� ����� ������� �������...
	: Employee(n), _level(l)/*, _salary(s)*/{	

	// ... �� ����� � ���� ������������! - ���� ��� protected
	Employee::_salary = s;

	// �.�. ���� _salary ���������� ��� ��, ��� � � ������� - 
	// ��� �������� ��������������������
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
	// ����� ����� ������ �������� ������
	Employee::show_name();
}


