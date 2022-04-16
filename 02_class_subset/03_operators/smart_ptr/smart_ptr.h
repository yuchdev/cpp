#pragma once

// В самописных smart ptrs можно приводить тип одного указателя к другому с помощью 
// шаблонного оператора приведения типа (Мейерс 2-28)
template <typename T>
class simple_ptr{
public:

	// конструктор, принимающий управление памятью
	explicit simple_ptr(T* mem = nullptr) : _ptr(mem){}

	// оператор преобразования simple_ptr в
	// simple_ptr любого типа! (конечно если указатель приводится к нему)
	// Достаточно опасный подход, здесь приведен для примера
	template <typename NewType>
	operator simple_ptr<NewType>(){
		// return 'new' value because new pointer should own a new copy
		// so that destroy it (otherwise it should be something like refcount)
		return simple_ptr<NewType>(new T(*_ptr));
	}

	// освобождение памяти
	~simple_ptr(){
		delete _ptr;
	}

	// Операции указателя
	T* operator->() const {
		return _ptr;
	};

	T& operator*() const {
		return *_ptr;
	};

	// функциональный аналог оператора ->
	T* get() const {
		return _ptr;
	};

	// освобождение от владения
	T* release() {
		T* ptr = _ptr;
		_ptr = 0;
		return ptr;
	};

private:
	T* _ptr;
};



class music{
public:
    virtual ~music(){}
	virtual void play() = 0;
	virtual void pause() = 0;
};

class mp3 : public music{
public:
	virtual ~mp3(){}
	virtual void play(){ cout << "mp3 play\n"; }
	virtual void pause(){ cout << "mp3 pause\n"; }
};

class ogg : public music{
public:
	virtual ~ogg(){}
	virtual void play(){ cout << "ogg play\n"; }
	virtual void pause(){ cout << "ogg pause\n"; }
};

// этот пример не будет компилироваться без преобразования 
// в simple_ptr<music>. Для этого определен соответствующий оператор в классе
void test_play(const simple_ptr<music>& mucis){
	mucis->play();
}

void show_smart_ptr(){

	simple_ptr<mp3> mymp3(new mp3);
	simple_ptr<ogg> myogg(new ogg);
	test_play(mymp3);
}

int main()
{
	show_smart_ptr();
	return 0;
}
