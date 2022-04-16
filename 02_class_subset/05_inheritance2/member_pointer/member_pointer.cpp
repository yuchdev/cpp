#include "member_pointer.h"
#include <iostream>
using std::cout;
using std::endl;

member_pointer::~member_pointer(){

}

void member_derived::start(){
	cout << "member_derived::start" << endl;
}

void member_derived::stop(){
	cout << "member_derived::stop" << endl;
}

void member_derived::suspend(){
	cout << "member_derived::suspend" << endl;
}

void member_derived::resume(){
	cout << "member_derived::resume" << endl;
}

void member_derived::static_init(){

}

//void* member_derived::operator new(size_t sz){
//
//	cout << "member_derived::operator new(size_t sz)" << endl;
//	void* m = malloc(sz);
//	memset(m, sizeof(member_derived), sz);
//	return m;
//}

//void* member_derived::operator new[](size_t sz){
//	void* m = calloc(,sz);
//	memset(m, sizeof(member_derived), sz);
//	return m;
//}

//void member_derived::operator delete(void* mem, size_t sz){
//
//	cout << "member_derived::operator delete(void* mem, size_t sz)" << endl;
//	if(mem){
//		free(mem);
//		memset(mem, sizeof(member_derived), sz);
//	}
//}

//void member_derived::operator delete[](void* mem, size_t sz){
//
//}

