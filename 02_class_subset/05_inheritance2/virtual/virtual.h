#pragma once
// ������������ ������������ �������������� ������������

enum lang{
	cpp_lang = 0,
	java_lang = 1,
	php_lang = 2,
	lang_count
};

// ������������� ������������ ������ ��������� � ������ 
// ������������ �������� ������
// �� ����� ������ ����������� ����������
// �.�. ������ ��� ����������� ������� �������
// ����� ����� ����������� �� ��������, � ���������� �� ������
// ����� ������� ����� ����������� � �������� ������� ������ ���
class code_generator_base{
public:
	code_generator_base(int i);
	~code_generator_base();

	// ������� ����������� �����
	virtual void print() = 0;
	virtual void generate(lang l);
protected:
	// ���������� ������ ����������� ��������������� �� ������ ��������������
	// ��� �� ��������� � ���������� ��������

private:
	int _i;
};

// � ������ ������ ���������� ���������� ������� ����� �������������� �������� ���

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

	// ����������� ����� print() �������������
	virtual void print();
	virtual void f1();
private:
};

class java_code_generator : public virtual code_generator_base{
public:
	java_code_generator(int i);
	~java_code_generator();
	virtual void f1();
	// ����������� ����� print() �����������
private:

};

class php_code_generator : public virtual code_generator_base{
public:
	php_code_generator(int i);
	~php_code_generator();
	virtual void f1();
	// ����������� ����� print() �����������
private:

};

// ������� ����� � �������� ��������� ��� ������������!
class code_formatter :		public cpp_code_generator
						,	public java_code_generator
						,	public php_code_generator {
public:
	code_formatter(int i);
	~code_formatter();

	// ������� ������������� ������� � ���������:
	// ����������� ����� print() ����������� �� ���� ���� ������� �������
	// �� ���������� ������ ���, ��� �� ���� �������������
	// warning C4250: 'code_formatter' : inherits 'cpp_code_generator::print' via dominance
	virtual void generate(lang l);

	// ���� ��������� ���������������, ����� ��������� �� � ������� ��������� using 
	// ��� ���� ���������
	using php_code_generator::f1;
	// virtual void f1();

protected:

private:
};


// ��� ���� ��������, ��� ��������� ����������� � ������������� ������������
class Base{
public:
	virtual void f_base() = 0;
};

// ������������ �����������, �� ���� �� ������� ���������� ������������
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

// ���������� �������, ����� ����� ��������� �� ������ ���������� ��������,
// � �� ���������� ��������

// ���� ����� ������������� � ���������� �������, ��� ���������������
// ���� ���� ����� private!
// ��������� ������������ ������� �� ������ ������ ������ ��������!