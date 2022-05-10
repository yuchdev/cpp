// Always design the inheritance hierarchy very carefully
// E.g. a square cannot inherit from a rectangle,
// otherwise it will inherit all of its methods, including the increment of one of its sides.
// And will itself slide into a rectangle.

/** Geometric shape abstraction */

/** Helper class */
class point
{
    int _x;
    int _y;
};

/** Helper class */
class line
{
    point _a;
    point _b;
};

/** Interface class */
class shape
{
public:
    // pure virtual method for drawing
    // class shape is abstract
    virtual void draw() = 0;

protected:

    // It is better to place constructor-destructor of an abstract class to protected section
    shape() {}
    virtual ~shape() {}

private:
    // Create a rule not to store any data in a base class
    // in order to avoid additional dependencies
    // Otherwise, there is a danger of making the base class
    // a "storehouse" of useful information, which it shouldn't be
};

// it is even better to store data in derived classes, separately from the interface,
// or in totally independent classes
class shape_info : public shape
{
    // the shape_info class does not need the virtual function shape::draw()
    // so it doesn't have to implement or even declare it
    // that's why its descendants remain abstract
    // virtual void draw() = 0;

protected:
    /** @brief color for ribs and surfaces */
    int _shape_color;	// 32-bit color format (RGB + transparency)
    int _border_color;
};


/** 2D shape abstraction */
class shape2d : public shape_info
{
public:

    // rotation method, defined by the rotation point
    virtual void rotate(point& p) = 0;
};

/** 2D shape abstraction */
class shape3d : public shape_info
{
public:
    // rotation method, defined by the 3 axes
    virtual void rotate(line& px, line& py, line& pz) = 0;
};

// classes representing two-dimensional shapes
// circle and rectangle
class circle : public shape2d
{
public:
    // In derived classes, virtual functions are implemented
    // only if necessary
    virtual void draw();
    virtual void rotate(point& p);
private:
    point _center;
    int _radius;
};

class rectangle : public shape2d
{
public:
    // The virtual keyword is not necessary
    virtual void rotate(point& p);
private:
    point _up_left;
    point _down_right;
};

// classes representing three-dimensional shapes
// sphere and cube
class sphere : public shape3d
{
public:
    virtual void draw();
    virtual void rotate(line& px, line& py, line& pz);
private:
    int center;
    int radius;
};

class cube : public shape3d
{
public:
    virtual void draw();
    virtual void rotate(line& px, line& py, line& pz);
private:
    int center;
    int rib;
};
