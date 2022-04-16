#pragma once
#include <typeinfo>
#include <limits>
namespace std {

// Подобно итератору, распределитель памяти - это чистая абстракция
// Класс, поддерживающий минимальный интерфейс распределителя STL
template <typename T>
class custom_allocator{
public:
	// определение типов-свойств
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T value_type;
	typedef T* pointer;
    typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;

	// Поддержка распределителя другого типа
	// Это может понадобиться, если в структуре данных
	// нужно хранить не только сами данные, а какую-либо
	// служебную информацию (деки, хэш-таблицы) и соответственно
	// выделять для нее память
	template <typename U>
	struct rebind{
		typedef custom_allocator<U> other;
	};

	// Аллокатор не имеет состояния, поэтому конструкторы и деструктор тривиальные
	custom_allocator() throw() {
		cout << "Type: " << typeid(T).name() << endl;
		cout << "Ref: " << typeid(reference).name() << endl;
		cout << "Ptr: " << typeid(pointer).name() << endl;
	}
	custom_allocator(const custom_allocator&) throw(){}
	~custom_allocator() throw() {}
	
	// конструктор для служебных типов
	// например, для узлов списка, массивов массивов дека, 
	// внутренних структур хеш-таблицы
	template <typename U>
	custom_allocator(const custom_allocator<U>&) throw() {}

	// возврат адресов
	pointer address(reference val){
		return &val;
	}

	const_pointer address(const_reference val){
		return &val;
	}

	// максимальное количество элементов
	size_type max_size() const throw(){
		return numeric_limits<size_t>::max() / sizeof(T);
	}

	// выделить память для n элементов без инициализации
	// элементы здесь не конструируются!
	// смысл второго элемента зависит от реализации
	// может быть использован при перегрузке new, например, указатель на предыдущий блок памяти
    pointer allocate(size_type num, typename custom_allocator<T>::const_pointer hint = nullptr){
		return reinterpret_cast<pointer>( ::operator new(num * sizeof(T)) );
	}

	// инициализация одного элемента выделенного блока значением val
	void construct(pointer p, const T& val){
		new (reinterpret_cast<void*>(p)) T(val);
	}

	// удаление одного элемента из блока
	void destroy(pointer p){
		// обин из случаев, когда деструктор вызывается явно
		p->~T();
	}
	
	// удаление самого блока
	// к этому моменту элемент должны быть уничтожены
	void deallocate(pointer p, size_type){
		// в общем случае deallocate не дает гарантий при передаче nullptr
		::operator delete(reinterpret_cast<void*>(p));
	}
private:
	// можно заставить распределитель хранить (и предоставлять)
	// некие служебные данные (где хранятся объекты, контроль контейнеров и тп)
};

// обязательно должна быть приведена специализация для void
template <>
class custom_allocator<void>{
public:
	// определение типов-свойств
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef void value_type;
	typedef void* pointer;
	typedef const void* const_pointer;

	template <typename U>
	struct rebind{
		typedef custom_allocator<U> other;
	};
};

// шаблонный оператор == сообщает, что все специализации взаимозаменяемы
// это значит, что память можно выделить одним, а уничтожить другим
template <typename T1, typename T2>
bool operator==(const custom_allocator<T1>&, const custom_allocator<T2>&) throw(){
	return true;
}

template <typename T1, typename T2>
bool operator!=(const custom_allocator<T1>&, const custom_allocator<T2>&) throw(){
	return false;
}


} //namespace std
