#include <bitset>
#include <string>
#include <iostream>
#include <stdexcept>
#include "containers2.h"
using namespace std;

void show_bitset(){

	// ��� 0
	bitset<10> b1;
	bitset<16> b2(0xaaaa);
	bitset<32> b3(0xaaaa);

	bitset<16> b4( string("00100001001") );		// �� �������� �� ������������!
	bitset<16> b5( string("00100001001"), 4);	// �� ������� ������ 4
	bitset<16> b6( string("00100001001"), 2, 8);// �� ������� ������ 4 �� 2
	
	try {
		bitset<16> b6( string("ytcivy"));// ���������� invalid_argument
	}
	catch (const std::invalid_argument& e) {
		cout << e.what() << endl;
	}

	string loong("000000000000111111111111100000000001111111110000111111111111000000");
	try {
		bitset<128> b7(loong);
		long l = b7.to_ulong(); // ���������� overflow_error
	}
	catch (const std::overflow_error& e) {
		cout << e.what() << endl;
	}

	// � ���� ������
	cout << b2.to_string() << endl;

	// ��������:
	// ������:
	b2 >>= 4;
	b3 <<=2;

	// ��������� ���
	b1.set();	// ��� � 1
	b2.reset();	// ��� � 0

	b1.set(2, 0);	// ������ � 0
	b1.reset(3);	// ������ � 0

	b3.flip();	// invert

	// �������� = - ������ ��� ������ ��������!
	b4 = ~b5;		// �������� � �����������
	b5 = b6 >> 5;	// ����� � �����������

	try {
		//b1[16] = 1;	// ����� ��������� �� ������� - � ���������
		// ����� �� exception, � debug assert
	}
	catch (const std::exception& e) {
		cout << e.what() << endl;
	}

	// ����� - ������ �������� ������������� ��������� <<
	cout << b2 << endl << b3 << endl;
}
