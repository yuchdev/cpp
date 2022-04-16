#include <cmath>
#include <string>
#include <iostream>
// объявление функций  для данных с плавающей точкой
#include <cfloat>
#include <cfenv>
#include <vector>
#include <stdexcept>
#include "ex.h"
#include "controlled.h"
#include "finished.h"
#include "sehhandler.h"
#include "autoptr.h"
#include "zero_divide.h"
#include "structured_exception.h"

using std::vector;
using std::cout;
using std::endl;

// Бросание исключний базовых типов
void show_base_type_ex(int b){
	
	// При бросании исключений базовых типов
	// накакое неявное приведение не происходит 
	// (double -> int, unsigned -> signed, etc.)
	if(b)
		throw int(1);
	else
		throw double(0.);
	// Лучше не кидать исключения базовых типов
	// и не выделять память из кучи
	//throw new int(1);
}

void show_base_type(){
	try	{
		//show_base_type_ex(1);
		show_base_type_ex(0);
	}
	catch (const int e)	{
		cout << "integer " << e << endl;
	}
	catch (const double e) {
		cout << "double " << e << endl;
	}
	// эта конструкция ловит вообще все исключения по указателю!
	catch (const void* p){
        // what to do here except of analyzing stack is unclear
        // could not delete p, type is unknown
	}

}

int main()
{
	show_base_type();
	show_simple_ex();
	return 0;
}

