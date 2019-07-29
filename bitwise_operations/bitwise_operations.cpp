#include <iostream>
#include "bit_operations.h"

// Bitwise operatons

void show_and() {
	int b = 50;
	if ( b & 0x10 )
		std::cout << "Fourth bit is set" << std::endl;
	else
		std::cout << "Fourth bit is clear" << std::endl;
}


void show_or(){
    int b = 50;
	short c = b | 0x04;
	std::cout << "c = " << c << std::endl;
}



void show_xor(){
    int b = 50;
	std::cout << "b = " << b << std::endl;
	b = b ^ 0x18;
	std::cout << "b = " << b << std::endl;
}

void show_negate(){
	int b = ~0x03;
	std::cout << "b = " << b << std::endl;
}


void shift_output(){

    int b = 2;
	for( int i = 31 ; i >= 0  ; i-- ){
		
		// Сдвиг на инкрементное число
		short d = b >> i;
		
		// выводим последний бит
		short c = d & 0x1;
		std::cout << c;
	}
	std::cout << std::endl;
}

void shift_output2(){
        int b = 8;
	bool leadzero = false;
	for( int i = 15 ; i >= 0  ; i-- ){
		short d = b >> i;
		short c = d & 0x1;

		// Как только закончились незначащие нули,
		// начинаем побитовый вывод
		if( c || leadzero){
			leadzero = true;
			std::cout << c;
		}
	}
	std::cout << std::endl;
}

void shift_collect(){
    int b = 12;	
	short mask = 0;
	short fill1 = 0x1;
	for( int i = 15 ; i >= 0 ; i-- ){
		
		short d = b >> i;
		short c = d & 0x1;
		std::cout << c;
		
		if(c == 0x1){
			mask |= fill1;
			fill1 <<= 1;
		}
	}
	std::cout << std::endl;
	shift_output(mask);
}

// !(!a && !b)
//a || b
//This is correct.  NOT (NOT A  AND  NOT B) equals A OR B.  Conversely, NOT (NOT A  OR  NOT B) equals A AND B.

int main(){

	show_and();
	show_or();
	show_xor();
	show_negate();
	shift_output();
	shift_output2();
	shift_collect();
    return 0;
}