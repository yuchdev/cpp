#include "complex.h"

// Внимание - в списке инициализации можно передавать любое выражение
// В данном случае new complex_t*()...
complex_matrix::complex_matrix(const size_t count_x) 
		: _count_x(count_x)
		, _count_y()
		, _matrix( new complex_t*() ) {

	_matrix[0] = new complex_t[count_x];
}

// ... а здесь -  new complex_t*[count_y]
complex_matrix::complex_matrix(const size_t count_x, const size_t count_y)
		: _count_x(count_x)
		, _count_y(count_y)
		, _matrix(new complex_t*[count_y]) {

	for( unsigned i = 0 ; i < count_y ; i++ ){
		_matrix[i] = new complex_t[count_x];
	}
}

complex_matrix::complex_matrix( const complex_matrix& c )
		: _count_x(c._count_x)
		, _count_y(c._count_y)
		, _matrix(new complex_t*[c._count_y]) {

	for( unsigned i = 0 ; i < c._count_y ; i++ ){
		_matrix[i] = new complex_t[c._count_x];
		
		for(unsigned j = 0 ; j < c._count_x ; j++ ){
			_matrix[i][j] = c(i,j);
		}
	}
}

// Это классический вариант конструктора присваивания
// Он хорошо поясняет его суть, но более эффективен следующий
// (этот не компилируется без определения макроса _NO_SWAP)
#ifdef _NO_SWAP
complex_matrix& complex_matrix::operator =( const complex_matrix& c ){

	// проверили не присваиваем ли самому себе
	if(this != &c){

		// определили новые размеры
		size_t new_size_x = c._count_x;
		size_t new_size_y = c._count_y;

		// выделили память под новые размеры
		complex_t** new_matrix = new complex_t*[c._count_y];
		for( unsigned i = 0 ; i < c._count_y ; i++ ){
			new_matrix[i] = new complex_t[c._count_x];

			for(unsigned j = 0 ; j < c._count_x ; j++ ){
				new_matrix[i][j] = c(i,j);
			}
		}
		// очистили старую память
		clear();

		// вступили во вледение новой памятью
		_matrix = new_matrix;

	}
	return *this;
}
#else
// Этот вариант более эффективен, т.к. всю работу по выделению памяти и очистке 
// выполняет конструктор копирования и ф-ция clear()
complex_matrix& complex_matrix::operator =( const complex_matrix& c ){
	
	// Мейерс 1 Правило 17
	// Проверка в операторе присваивания самому себе - возможна ситуация, когда ресурсы будут уже освобождениы,
	// и присваивать будет нечего
	// можно также сравнивать объекты (при помощи operator==()), а не указатели - это повысит эффективность
	// В случае пулов объектов можно сравнивать по некеому ID
	if(this == &c){
		return *this;
	}

	complex_matrix* tmp = new complex_matrix(c);
	clear();
	_matrix = tmp->_matrix;
	return *this;
}
#endif // _NO_SWAP

// Также, в любых других функциях, принимающих ссылку на собственный класс,
// не нужно забывать о совмещении значений


complex_matrix::~complex_matrix(){
	clear();
}

void complex_matrix::clear(){
	for( unsigned i = 0 ; i < _count_y ; i++ ){
		delete[] _matrix[i];
	}
	delete[] _matrix;
	_matrix = nullptr; // не забываем обнулять!
}

// по 2 версии операторов индексирования
complex_t* complex_matrix::operator[](unsigned index){
	return _matrix[index];
}

complex_t& complex_matrix::operator()(unsigned index_x, unsigned index_y){
	return _matrix[index_x][index_y];
}

const complex_t* complex_matrix::operator[](unsigned index) const{
	return _matrix[index];
}

const complex_t& complex_matrix::operator()(unsigned index_x, unsigned index_y) const{
	return _matrix[index_x][index_y];
}
