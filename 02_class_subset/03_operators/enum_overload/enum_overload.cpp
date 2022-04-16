#include <iostream>
#include "complex.h"
#include "smart_ptr.h"
using std::vector;

#pragma pack(8)
using namespace std;



// продемонстрируем перегрузку перечислений
enum Wday{ mon = 1, tus, wed, thr, frd, sat, sun };

Wday& operator++(Wday& d){
	return d = (d == sun) ? mon : Wday(d+1);
}

void show_enum_overload(){
	Wday d = sun;
	++d;
	++d;
}

int main()
{
	show_smart_ptr();
	show_vector_by_val();
	show_dynamic();
	show_overloads();
	show_enum_overload();
	return 0;
}
