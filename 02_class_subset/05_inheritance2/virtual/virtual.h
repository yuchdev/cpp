#pragma once
// Демонстрация виртуального множественного наследования

enum lang{
	cpp_lang = 0,
	java_lang = 1,
	php_lang = 2,
	lang_count
};

// Множественное наследование хорошо управлемо в случае 
// абстрактного базового класса
// От этого класса наследуются виртуально
// т.е. делают его виртуальным базовым классом
// такой класс реализуется не объектом, а указателем на объект
// таким образом может встречаться в иерархии сколько угодно раз
class code_generator_base{
public:
	code_generator_base(int i);
	~code_generator_base();

	// Базовый виртуальный метод
	virtual void print() = 0;
	virtual void generate(lang l);
protected:
	// Объявление данных защищенными свидетельствует об ошибке проектирования
	// Это не относится к защищенным функциям

private:
	int _i;
};

// В памяти объект включающий вируальный базовый класс представляется примерно так

// cpp_code_generator:
// vptr
// pointer to code_generator_base

// java_code_generator:
// vptr
// pointer to code_generator_base

// php_code_generator:
// vptr
// pointer to code_generator_base

// code_formatter:
// vptr

// code_generator_base:
// vptr
// int

class cpp_code_generator : public virtual code_generator_base {
public:
	cpp_code_generator(int i);
	~cpp_code_generator();

	// виртуальный метод print() переопределен
	virtual void print();
	virtual void f1();
private:
};

class java_code_generator : public virtual code_generator_base{
public:
	java_code_generator(int i);
	~java_code_generator();
	virtual void f1();
	// виртуальный метод print() унаследован
private:

};

class php_code_generator : public virtual code_generator_base{
public:
	php_code_generator(int i);
	~php_code_generator();
	virtual void f1();
	// виртуальный метод print() унаследован
private:

};

// Крайний класс в иерархии наследует уже невиртуально!
class code_formatter :		public cpp_code_generator
						,	public java_code_generator
						,	public php_code_generator {
public:
	code_formatter(int i);
	~code_formatter();

	// Принцип доминирования состоит в следующем:
	// виртуальный метод print() унаследован от всех трех базовых классов
	// но выполнится только там, где он явно переопределен
	// warning C4250: 'code_formatter' : inherits 'cpp_code_generator::print' via dominance
	virtual void generate(lang l);

	// Если возникает наоднозначность, можно разрешить ее с помощью директивы using 
	// или явно заместить
	using php_code_generator::f1;
	// virtual void f1();

protected:

private:
};


// Еще одна иерархия, где участвует виртуальное и невиртуальное наследование
class Base{
public:
	virtual void f_base() = 0;
};

// Наследование ромбовидное, но один из классов наследован невиртуально
class Derived1 : public Base{
	virtual void f1() = 0;
};

class Derived2 : virtual public Base{
	virtual void f2() = 0;
};

class Final : public Derived1, public Derived2{
public:
	virtual void f_base();
	virtual void f1();
	virtual void f2();
};

// Интересный паттерн, когда класс наследует от класса интерфейса публично,
// а от реализации приватно

// Если метод переопределен в нескольких классах, это неоднозначность
// Даже если метод private!
// изменение модификатора доступа не должно менять логику програмы!