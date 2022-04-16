#include <iostream>
#include <sstream>
#include "streams.h"

using namespace std;

void output_string_streams(){
	
	char c = 'a';
	long l = 10l;
	double d = 100.5;
	ostringstream os;
	// объект string выделяется внутри потока по new
	os << "Formatted string "
		<< "Char:" << c
		<< " Long:" << l
		<< " Double:" << d;
	cout << os.str() << endl;
	cout << "Total symbols: "<< os.str().length() << endl;
}

void bidirect_string_streams(){
	// поток ввода получает данные из потока в строку или другую переменную
	// сам по себе довольно бесполезен
	// имеет смысл использовать двунаправленный поток
	// комбинирует два предыдущих потока
	string name;
	int age = 0;

	stringstream ss;
	cout << "Enter name age (through whitespace): ";

	ss << "atatat"
		<< ' '
		<< 32;

	// перед чтением надо установить укуазатель в начало потока
	ss.seekg(0);

	ss >> name >> age;

	cout << "Entered name: " << name << ", and age: "<< ' ' << age << endl;

	string out(ss.str());
	cout << out << endl;
}

void show_string_streams(){
	output_string_streams();
	bidirect_string_streams();
}