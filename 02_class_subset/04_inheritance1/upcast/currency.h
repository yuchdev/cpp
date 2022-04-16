#pragma once

class currency_t
{
public:
	currency_t();
	currency_t(double c);
	currency_t(const currency_t& c);
	~currency_t();

	currency_t& operator=( const currency_t& c );

	// ������������� ����������
	// 1. ���� ������������ ���� � �� �� �������� � ������� � ����������� �������, 
	// �������������� � ������ ������������ ��������� ������������� - 
	// ����������� ����������� �� ������, � ����������� ����� ����������� ���������� ��� �������
	currency_t& operator+=( const currency_t& c );
	currency_t& operator-=( const currency_t& c );

	// 2. �� ���� ������������ ������� ������� ������������ � ����������������
	// �������� � ����������� ������� (������ �������),
	// �� ��������� ������ ����� ���� ������������, �������� ���
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

	// ��� ������ � ������������� �� ������� ������������ �������� ������
	// ���������� ������������� operator=()
	// ��� �������� ������������� ������� ����� � �����������
	//����� �������������� ���������� _�����������_
	roubles_t& operator=( const currency_t& c);

	// !!! ����������� ����� ����������� ����������� ��������������
	// �������� ��������� ������ ������� ������ ����� ��������
	roubles_t( const currency_t& c );

	// ��� ������ ������ �� ������������� ����������
	virtual currency_t& operator*=( const currency_t& c );
	virtual currency_t& operator/=( const currency_t& c );
};

// �� ���������� ����� ����������� ������!
//class ext_int : public int {
//	int _i;
//	int _a;
//	ext_int( int i ) : _i(i), _a(i){}
//};
