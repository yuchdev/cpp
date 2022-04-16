#include <bitset>
#include <string>
#include <iostream>
#include <stdexcept>
#include "containers2.h"
using namespace std;

void show_bitset(){

	// все 0
	bitset<10> b1;
	bitset<16> b2(0xaaaa);
	bitset<32> b3(0xaaaa);

	bitset<16> b4( string("00100001001") );		// из литерала не конвертирует!
	bitset<16> b5( string("00100001001"), 4);	// от позиции строки 4
	bitset<16> b6( string("00100001001"), 2, 8);// от позиции строки 4 до 2
	
	try {
		bitset<16> b6( string("ytcivy"));// исключение invalid_argument
	}
	catch (const std::invalid_argument& e) {
		cout << e.what() << endl;
	}

	string loong("000000000000111111111111100000000001111111110000111111111111000000");
	try {
		bitset<128> b7(loong);
		long l = b7.to_ulong(); // исключение overflow_error
	}
	catch (const std::overflow_error& e) {
		cout << e.what() << endl;
	}

	// в виде строки
	cout << b2.to_string() << endl;

	// Операции:
	// сдвиги:
	b2 >>= 4;
	b3 <<=2;

	// Установка бит
	b1.set();	// все в 1
	b2.reset();	// все в 0

	b1.set(2, 0);	// второй в 0
	b1.reset(3);	// третий в 0

	b3.flip();	// invert

	// оператор = - только для равных размеров!
	b4 = ~b5;		// инверсия с присвоением
	b5 = b6 >> 5;	// сдвиг с присвоением

	try {
		//b1[16] = 1;	// любое обращение по индексу - с проверкой
		// здесь не exception, а debug assert
	}
	catch (const std::exception& e) {
		cout << e.what() << endl;
	}

	// вывод - пример двойного использования оператора <<
	cout << b2 << endl << b3 << endl;
}
