// ����� 9.4 ����������
// ������� ������� � ������������

// ------------------- ����������� ������� ������ -------------------
// ����������� ������� ����� - �� ��������� �� ��������� �������
// ����� ������������� ��� �������, ��� � �������� ������

// ������� ������
template <typename T>
class base_tmpl
{
public:
    base_tmpl() {}
    int basefield;
    // � ���������� int'�� (����������� ���������� ��������?)
    typedef int T;
};

// �� ������
class derived1 : public base_tmpl< void >
{
public:
    void func()
    {
        // ������� ������ � ����������
        basefield = 3;
    }
};

// ������, �� ������� ������ �� ��� ���������� �� �������
template <typename T>
class derived2 : public base_tmpl< double >
{
public:
    void func()
    {
        // ������� ������ � ����������
        basefield = 7;
        strange = 1;
    }
    // � ����� ����������� �� �������� �������
    T strange;
};



void test_tmpl_inherit()
{
    base_tmpl<double> bd;

    derived1 dd;
    dd.func();

    derived2<int> d2;
    d2.func();

    int i = d2.strange;
    double d = d2.strange;

    derived2<double> d3;
    d3.func();

    int i2 = d3.strange;
    d = d3.strange;

    derived2<int*> d4;
    d4.func();

    // ��� ������, ��������� d4.strange ����� ������� ���
    // base_tmpl<T>::T, � �� int*
#if 0
    int* pi = d4.strange;
#endif

}

// ------------------- ��������� ������� ������ -------------------
// ��������� ������� ������ ������������ ���������� �������
// ������, ������� ������ �������� ������� �� ��� ���������� 
template <typename T>
class derived3 : public base_tmpl< T >
{
public:
    void func()
    {
        // ������ � ���������� � �������
        // ����� ���� ��������� this->
        this->basefield = 7;
    }
};

void test_tmpl_inherit2()
{
    derived3<void> dd1;
    dd1.func();

    derived3<double> dd2;
    dd2.func();

}