#include "adapter.h"
#include <iostream>
using std::cout;
using std::endl;

deque_t::deque_t(){}

deque_t::deque_t( const deque_t& c ) 
	: _deque(c._deque){}

deque_t& deque_t::operator=( const deque_t& c ){ 
	_deque = c._deque;
	return *this;
}

deque_t::~deque_t(){}

void deque_t::push_back(int val){
	_deque.push_back( val );
}

void deque_t::push_front(int val){
	_deque.push_front( val );
}

int deque_t::pop_back(){
	int ret = _deque.back();
	_deque.pop_back();
	return ret;
}

int deque_t::pop_front(){
	int ret = _deque.front();
	_deque.pop_front();
	return ret;
}

bool deque_t::is_empty(){
	return _deque.empty();
}

void deque_t::clear(){
	_deque.clear();
}


// ------------------------------ 
adapter_stack_t::adapter_stack_t(){}

adapter_stack_t::adapter_stack_t(const deque_t& d)
	: deque_t(d){}

adapter_stack_t::adapter_stack_t(const adapter_stack_t& s)
	: deque_t(s){}

adapter_stack_t::~adapter_stack_t(){}

adapter_stack_t& adapter_stack_t::operator=( const adapter_stack_t& c ){
	deque_t::operator =(c);
	return *this;
}

adapter_stack_t& adapter_stack_t::operator=( const deque_t& c ){
	deque_t::operator =(c);
	return *this;
}

void adapter_stack_t::push(int val){
	deque_t::push_back(val);
}

int adapter_stack_t::pop(){
	return deque_t::pop_back();
}

bool adapter_stack_t::is_empty(){
	return deque_t::is_empty();
}

void adapter_stack_t::test_pr(){
    cout << "test protected adapter_stack_t::test_pr()" << endl;
}

// ---  ---
void write_only_adapter_stack_t::push(int val){
	// Достучаться до методов deque_t нельзя - наследование private
	// все protected/public методы стали private
	// Если его поменять на protected - будет можно
	// (методы станут protected)
	//deque_t::push_back(val);

    adapter_stack_t::push(val);
}
