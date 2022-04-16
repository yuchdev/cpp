// �������� ������� ����� ���� ������������� ������
// ������� �������� ����� �� ����������
// ���� � ����� ������������ ���� ������������ �� �������������,
// �.�. ��� �� ����� ���� ��������� ������������,
// � ������� �� ��������� RTTI � ����������� �������

// ���������� ��������� ����������� �������� ������������.
// ��������, ������� �� ����� ����������� �� ��������������,
// ����� �� ��������� ��� ��� ������, � ��� ����� ���������� ����� �� ������
// � ��� ��������� � �������������

/** ���������� �������������� ������ */

/** ��������������� ����� */
class point{
    int _x;
    int _y;
};

/** ��������������� ����� */
class line{
    point _a;
    point _b;
};

/** ������������ ����� */
class shape{
public:
    // ����� ��� ���������, ����� �����������
    // ����� shape - �����������
    virtual void draw() = 0;
protected:

    // better to place constructor-destructor to protected section
    // That makes classes abstract even WITHOUT pure virtual functions
    shape(){}
    virtual ~shape(){}

private:

    // � ������� ������ ���������� ������� ������ _�����_ ����� ������
    // �� ��������� �������������� ������������
    // ����� ���������� ����������� ������� ������� �����
    // "����������" �������� ����������, ��� �� ���� �� ������...
};

// ...��� ����� ������� ������ � ����������� �������, �������� �� ����������
// ��� ������ � �����������  - ����� ����������� ����� ���:
// class shape2d : public shape, public shape_info
class shape_info : public shape
{
    // ����� shape_info �� ��������� � ����������� ������� shape::draw()
    // ������� �� ������ �� ������������� ��� ���� ���������:
    // ��������� ����� �� �������� �����������
    // virtual void draw() = 0;

protected:
    /** @brief ���� ��� ����� � ������������ */
    int _shape_color;	// 32-������ ���� (RGB + ������������)
    int _border_color;
};


/** ���������� ���������� ������ */
class shape2d : public shape_info{
public:

    // �������� ����� ��� ��������, ���������� ������ ��������
    virtual void rotate(point& p) = 0;
};

/** ���������� ���������� ������ */
class shape3d : public shape_info{
public:
    // �������� ����� ��� ��������, ���������� ����� �����
    virtual void rotate(line& px, line& py, line& pz) = 0;
};

// ----------------------------------
// ������ ������������� ���������� ������ -
// ���������� � �������������
class circle : public shape2d{
public:
    // � ����������� ������� ����������� ������� �����������
    // ������ ���� � ���� ���� �������������
    virtual void draw();
    virtual void rotate(point& p);
private:
    point _center;
    int _radius;
};

class rectangle : public shape2d{
public:
    // �������� ����� virtual �� �����������, �� ����������
    // ����� �� ��������� �������� � ������������
    virtual void draw();
    virtual void rotate(point& p);
private:
    point _up_left;
    point _down_right;
};
// ----------------------------------

// ----------------------------------
// ������ ������������� ���������� ������ -
// ����� � ���
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
