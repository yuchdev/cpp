#pragma once

class currency_t
{
public:
	currency_t();
	currency_t(double c);
	currency_t(const currency_t& c);
	~currency_t();

	currency_t& operator=( const currency_t& c );

	// ¬иртуальность операторов
	// 1. ≈сли выполн€ютсс€ одни и те же действи€ в базовом и производном классах, 
	// переопредел€ть и делать виртуальными операторы необ€зательно - 
	// срабатывает полиморфизм по ссылке, и производный класс принимаетс€ параметром как базовый
	currency_t& operator+=( const currency_t& c );
	currency_t& operator-=( const currency_t& c );

	// 2. Ќо если присутствует сложна€ система наследовани€ с переопределением
	// операций в производных классах (редкое €вление),
	// то операторы вполне могут быть виртуальными, например так
	virtual currency_t& operator*=( const currency_t& c );
	virtual currency_t& operator/=( const currency_t& c );

	friend const currency_t operator+( const currency_t& a, const currency_t& b );
	friend const currency_t operator-( const currency_t& a, const currency_t& b );
	friend const currency_t operator*( const currency_t& a, const currency_t& b );
	friend const currency_t operator/( const currency_t& a, const currency_t& b );

	

	void print();
protected:
	double _currency;
};

class roubles_t : public currency_t{
public:
	roubles_t(double d);
	void print();

	// ƒл€ работы с возвращаемыми по знчению экземп€лрами базового класса
	// перегрузим преобразующий operator=()
	// Ёто позволит преобразовать базовый класс в производный
	//“акие преобразовани€ называютс€ _понижающими_
	roubles_t& operator=( const currency_t& c);

	// !!! ќб€зательно также перегрузить конструктор преобразовани€
	// —тандарт позвол€ет писать сколько угодно таких операций
	roubles_t( const currency_t& c );

	// Ёто просто пример по виртуализации операторов
	virtual currency_t& operator*=( const currency_t& c );
	virtual currency_t& operator/=( const currency_t& c );
};

// ќт встроенных типов наследовать нельз€!
//class ext_int : public int {
//	int _i;
//	int _a;
//	ext_int( int i ) : _i(i), _a(i){}
//};
