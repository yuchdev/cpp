#include <stdexcept>
#include "complex.h"

complex_t::complex_t(double r/* = 0.*/, double i /*= 0.*/) : re(r), im(i){}
complex_t::complex_t(int r) : re(r), im(0){}

double complex_t::real() const { return re; }
double complex_t::imag() const { return im; }



// ------------------------------ 
complex_t& complex_t::operator+=( const complex_t& a ){
	re += a.re; im += a.im;
	return *this;
}

// ------------------------------
complex_t& complex_t::operator-=( const complex_t& a ){
	re -= a.re; im -= a.im;
	return *this;
}

// ------------------------------
complex_t& complex_t::operator*=( const complex_t& a ){
	re += (re * a.re) - (im * a.im); 
	im += (re * a.im) + (im * a.re);
	return *this;
}

// ------------------------------
complex_t& complex_t::operator/=( const complex_t& a ){
	if(a == 0)
		throw std::runtime_error( "null division" );

	double tmp = re*re + a.re*a.re;
	re =  ((re * a.re) + (im * a.im))/tmp;
	im = ((a.re * im) - (re * a.im))/tmp;
	return *this;
}

// ------------------------------
bool complex_t::operator==( const complex_t& a ) const {
	return ( (re == a.re) && (im == a.im) );
}

// ------------------------------
bool complex_t::operator!=( const complex_t& a ) const {
	return !( *this == a );
}

complex_t& complex_t::operator++(){
	re += 1.;
	im += 1.;
	return *this;
}

complex_t& complex_t::operator--(){
	re -= 1.;
	im -= 1.;
	return *this;
}

complex_t complex_t::operator++(int){
	// постфикс - возвращаем по значению
	// (сначала сохраняем значение, увеличиваем потом)
	complex_t tmp = *this;
	re += 1.;
	im += 1.;
	return tmp;
}

complex_t complex_t::operator--(int){
	// постфикс - возвращаем по значению
	complex_t tmp = *this;
	re -= 1.;
	im -= 1.;
	return tmp;
}

complex_t::operator int(){ 
	return static_cast<int>(re); 
}

complex_t::operator double(){ 
	return re; 
}



// ------------------------------
// Секция глобальных дружественных функций
// (* и / определены для примера в теле класса)
// ------------------------------
const complex_t operator+( const complex_t& a, const complex_t& b ){
	complex_t tmp(a);
	return tmp += b;
}

// ------------------------------
const complex_t operator-( const complex_t& a, const complex_t& b ){
	complex_t tmp(a);
	return tmp -= b;
}

// в коммутативных операторах для повышения производительности иногда стоит отказаться от неявных приведений
// Т.е. явно оперделить
// const complex_t operator+( const complex_t& a, int b )
// const complex_t operator+( int a, const complex_t& b )
// const complex_t operator+( const complex_t& a, double b )
// ...
// или воспользоваться шаблоном и специализациями

complex_t& get_cache()
{
	static const int cache_size = 10;
	static int nbuf = 0;

	// Заготовим кэш на 10 комплексных чисел
	static complex_t buf[cache_size];
	
	// сброс кэша
	if( nbuf == cache_size ) 
		nbuf = 0;

	// возвращаем текущее закешированное значение
	return buf[nbuf++];

}

complex_t& operator%( const complex_t& a, const complex_t& b )
{
	complex_t& res = get_cache();
	// считаем %
	 return res;
}
