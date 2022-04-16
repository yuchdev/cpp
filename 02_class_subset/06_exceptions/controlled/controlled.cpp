#include "controlled.h"
#include <iostream>
#include <stdexcept>

using std::cout;
using std::endl;
using std::vector;
using std::exception;

// --------------------------------------
// Контролируемый конструктор - перехватывает все исключения
// даже в списке инициализации ( и передает их далее )
controlled::controlled() try : _vect(1000){
	for( int i = 0 ; i < 1000 ; i++ )
		_vect.push_back(1);	

    throw std::runtime_error("constructor exception");
}
catch(const exception& e) {
	cout << e.what() << endl;
}
catch(...){
	cout << "Unknown exception" << endl;
}

// Стандарт: Если выражение return встречается внутри обработчика function-try-block конструктора, то 
// программа считается плохо сформированной(ill - formed).

// --------------------------------------
// Деструкторы не должны кидать исключения
// По крайней мере они не должны выходить за его пределы
controlled::~controlled() throw() {}

// --------------------------------------
// Контролируемый метод
void controlled::controlled_method() const try{
    throw std::runtime_error("controlled method exception");
}
catch( std::exception& e ){
	cout << e.what() << endl;
}
