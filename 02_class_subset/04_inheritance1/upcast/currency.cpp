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

// ������������ �� ����������� - ���������� ����!
roubles_t::roubles_t(double d) : currency_t(d){}

// !���������� ���� ���������� ����������� ����������� ��������������!
roubles_t::roubles_t( const currency_t& c ) : currency_t(c){}

// ���������������� �����
void roubles_t::print(){
	std::cout << _currency << " rubles" << std::endl;
}

roubles_t& roubles_t::operator=( const currency_t& c){
	// ����� ����� ������������� ��������� � �������������� �����
	currency_t::operator =(c);

	// �.�. �� ���� ������������� ������� ����� � �����������
	// ����� �������������� ���������� _�����������_
	return *this;
}

// ��� ������ ������ �� ������������� ����������
// ������ �� ������������ � ������ ������ �������������
currency_t& roubles_t::operator*=( const currency_t& c ){
	currency_t::operator *=(c);
	return *this;
}

currency_t& roubles_t::operator/=( const currency_t& c ){
	currency_t::operator /=(c);
	return *this;
}

