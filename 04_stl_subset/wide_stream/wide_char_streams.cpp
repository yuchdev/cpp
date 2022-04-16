#include <iostream>
#include <locale>
#include <sstream>
#include "streams.h"

using namespace std;

void wide_symbols(){
	
	// символы, литералы и строки
	wchar_t wc = L'A';
	wchar_t str[] = L"Wide char literal";
	wstring wstr(L"Wide char string");

	wcout << str << endl;
	wcout << wstr << endl;
	
    // widen() method
	wcout << wcout.widen('W') << endl;

    // reverse operation narrow()
    // second param is returned if unable to narrow
    cout << cout.narrow(L'W', 'W') << endl;

	// при чтении методом read, подсчет в символах, а не байтах
	wchar_t wc1 = '\0';
	wcin.read(&wc1, 1);
	wcout << L"You entered: " << wc1 << endl;
}

void wide_locales(){
	// Чтобы выводить юникодные символы, 
	// нужно установить локаль для wcin и wcout
	locale current_loc_cin = wcin.getloc();
	locale current_loc_cout = wcout.getloc();

    locale current_loc;

    cout << "Locale name: " << current_loc.name() << '\n';

	// в консоли Windows codepage всегда 866
	locale russian_loc("rus_rus.866");
	wcin.imbue(russian_loc);
	wcout.imbue(russian_loc);

	cout << "Locale name: " << russian_loc.name() << '\n';

	wstring cyrrilic_str;
	wcin >> cyrrilic_str;
	wcout << L"Введена строка: " << cyrrilic_str << endl;

	//wcin.clear(/*ios_base::goodbit*/);
	//wcin.seekg(0);
	if (wcin.fail()){
		wcin.clear(/*ios_base::goodbit*/);
	}

	// \n should be removed after wcin usage
	wcin.ignore();

	// все остальные методы так же работают с юникодом
	wstring cyrrilic_str1;
	getline(wcin, cyrrilic_str1);
	wcout << L"Введена строка: " << cyrrilic_str1 << endl;

	// вернем системные локали обратно
	wcin.imbue(current_loc_cin);
	wcout.imbue(current_loc_cout);
}

void wide_strings(){
	// по аналогии с string_streams.cpp
	// пример с двунаправленным широким потоком
	wstring cyrillic_name;
	int age = 0;

	wstringstream ss;
	
	locale current_loc_cout = wcout.getloc();
	locale russian_loc("rus_rus.866");

	// should be set without first usage!
	ss.imbue(russian_loc);
	wcout.imbue(russian_loc);

	wcout << L"Введите имя на кириллице и возраст (через пробел): " << endl;

	ss << L"ататат"
		<< L' '
		<< 32;

	// перед чтением надо установить укуазатель в начало потока
	ss.seekg(0);

	ss >> cyrillic_name >> age;
	
	wcout << L"Введены имя и возраст: " << cyrillic_name << wcout.widen(' ') << age << endl;


	wstring out(ss.str());
	wcout << out << endl;

	wcout.imbue(current_loc_cout);
	
	// аналогично для файловых потоков
}


void show_wide_char_locales(){
	// does not work =((
    //wide_symbols();
	//wide_locales();
	wide_strings();
}
