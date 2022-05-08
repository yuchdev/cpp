// Глава 9.4 Джосаттиса
// Шаблоны классов и наследование

// ------------------- Независимые базовые классы -------------------
// Независимый базовый класс - не зависящий от параметра шаблона
// Может наследоваться как классом, так и шаблоном класса

// Базовый шаблон
template <typename T>
class base_tmpl
{
public:
    base_tmpl() {}
    int basefield;
    // Т становится int'ом (перекрывает переданный параметр?)
    typedef int T;
};

// Не шаблон
class derived1 : public base_tmpl< void >
{
public:
    void func()
    {
        // Обычный доступ к наследнику
        basefield = 3;
    }
};

// Шаблон, но базовый шаблон от его параметров не зависит
template <typename T>
class derived2 : public base_tmpl< double >
{
public:
    void func()
    {
        // Обычный доступ к наследнику
        basefield = 7;
        strange = 1;
    }
    // Т здесь наследуется от базового шаблона
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

    // Это ошибка, поскольку d4.strange имеет базовый тип
    // base_tmpl<T>::T, а не int*
#if 0
    int* pi = d4.strange;
#endif

}

// ------------------- Зависимые базовые классы -------------------
// Зависимые базовые классы определяются параметром шаблона
// Шаблон, базовый шаблон которого зависит от его параметров 
template <typename T>
class derived3 : public base_tmpl< T >
{
public:
    void func()
    {
        // Доступ к наследнику в шаблоне
        // лучше явно указывать this->
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