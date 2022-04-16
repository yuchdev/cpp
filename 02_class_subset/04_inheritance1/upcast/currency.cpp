#include "currency.h"

#include <iostream>

currency_t::currency_t() : _currency(){}
currency_t::currency_t(double d) : _currency(d){}
currency_t::currency_t(const currency_t& c) : _currency(c._currency){}
currency_t::~currency_t(){}

currency_t& currency_t::operator =(const currency_t& c){
	if(this != &c){
		_currency = c._currency;
	}
	return *this;
}

currency_t& currency_t::operator+=(const currency_t& c){
	_currency += c._currency;
	return *this;
}

currency_t& currency_t::operator -=(const currency_t& c){
	_currency -= c._currency;
	return *this;
}

currency_t& currency_t::operator *=(const currency_t& c){
	_currency *= c._currency;
	return *this;
}

currency_t& currency_t::operator /=(const currency_t& c){
	_currency /= c._currency;
	return *this;
}

void currency_t::print(){
	std::cout << _currency << " dollars" << std::endl;
}

const currency_t operator+( const currency_t& a, const currency_t& b ){
	currency_t tmp(a);
	return tmp += b;
}

const currency_t operator-( const currency_t& a, const currency_t& b ){
	currency_t tmp(a);
	return tmp -= b;
}

const currency_t operator*( const currency_t& a, const currency_t& b ){
	currency_t tmp(a);
	return tmp *= b;
}

const currency_t operator/( const currency_t& a, const currency_t& b ){
	currency_t tmp(a);
	return tmp /= b;
}

//  онструкторы не наследуютс€ - определ€ем €вно!
roubles_t::roubles_t(double d) : currency_t(d){}

// !Ќеобходимо €вно определить конструктор понижающего преобразовани€!
roubles_t::roubles_t( const currency_t& c ) : currency_t(c){}

// переопределенный метод
void roubles_t::print(){
	std::cout << _currency << " rubles" << std::endl;
}

roubles_t& roubles_t::operator=( const currency_t& c){
	// €вный вызов родительского оператора в функциональной форме
	currency_t::operator =(c);

	// “.е. мы €вно преобразовали базовый класс в производный
	// “акие преобразовани€ называютс€ _понижающими_
	return *this;
}

// Ёто просто пример по виртуализации операторов
// ƒелать их виртуальными в данном случае необ€зательно
currency_t& roubles_t::operator*=( const currency_t& c ){
	currency_t::operator *=(c);
	return *this;
}

currency_t& roubles_t::operator/=( const currency_t& c ){
	currency_t::operator /=(c);
	return *this;
}

