// Иерархия классов может быть нециклическим графом
// Глубина иерархии ничем не ограничена
// Поля с явным обозначением типа настоятельно не рекомендуются,
// т.к. они не могут быть проверены компилятором,
// в отличие от механизма RTTI и виртуальных функций

// Необходимо тщательно планировать иерархию наследования.
// Например, квадрат не может наследовать от прямоугольника,
// иначе он наследует все его методы, в том числе приращение одной из сторон
// и сам выродится в прямоугольник

/** Абстракция геометрической фигуры */

/** Вспомогательный класс */
class point{
    int _x;
    int _y;
};

/** Вспомогательный класс */
class line{
    point _a;
    point _b;
};

/** Интерфейсный класс */
class shape{
public:
    // метод для рисования, чисто виртуальный
    // класс shape - абстрактный
    virtual void draw() = 0;
protected:

    // better to place constructor-destructor to protected section
    // That makes classes abstract even WITHOUT pure virtual functions
    shape(){}
    virtual ~shape(){}

private:

    // В базовом классе необходимо хранить только _самые_ общие данные
    // во избежание дополнительных зависимостей
    // Иначе существует вероятность сделать базовый класс
    // "хранилищем" полезной информации, чем он быть не должен...
};

// ...еще лучше хранить данные в производных классах, отдельно от интерфейса
// Или вообще в независимых  - тогда наследовать будет так:
// class shape2d : public shape, public shape_info
class shape_info : public shape
{
    // класс shape_info не нуждается в виртуальной функции shape::draw()
    // поэтому не обязан ее реализовывать или даже объявлять:
    // благодаря этому он остается абстрактным
    // virtual void draw() = 0;

protected:
    /** @brief цвет для ребер и поверхностей */
    int _shape_color;	// 32-битный цвет (RGB + прозрачность)
    int _border_color;
};


/** Абстракция двухмерной фигуры */
class shape2d : public shape_info{
public:

    // содержит метод для вращения, задаваемый точкой вращения
    virtual void rotate(point& p) = 0;
};

/** Абстракция трехмерной фигуры */
class shape3d : public shape_info{
public:
    // содержит метод для вращения, задаваемый тремя осями
    virtual void rotate(line& px, line& py, line& pz) = 0;
};

// ----------------------------------
// классы представлящие двухмерные фигуры -
// окружность и прямоугольник
class circle : public shape2d{
public:
    // В производных классах виртуальные функции реализуются
    // только если в этом есть необходимость
    virtual void draw();
    virtual void rotate(point& p);
private:
    point _center;
    int _radius;
};

class rectangle : public shape2d{
public:
    // Ключевое слово virtual не обязательно, но желательно
    // чтобы не возникало путиницы в наследовании
    virtual void draw();
    virtual void rotate(point& p);
private:
    point _up_left;
    point _down_right;
};
// ----------------------------------

// ----------------------------------
// классы представлящие трехмерные фигуры -
// сферу и куб
class sphere : public shape3d{
public:
    virtual void draw();
    virtual void rotate(line& px, line& py, line& pz);
private:
    int center;
    int radius;
};

class cube : public shape3d{
public:
    virtual void draw();
    virtual void rotate(line& px, line& py, line& pz);
private:
    int center;
    int rib;
};
// ----------------------------------
