#pragma once

// Plain (non-virtual) multiple inheritance, illustrated with a small GUI
// "control" hierarchy. control_t is a common base; button_t and slider_t
// each extend it independently; window_draw inherits from BOTH of them.
// Because neither button_t nor slider_t inherits control_t virtually,
// control_t is a REPEATED base in window_draw: the object contains two
// entirely separate control_t sub-objects, one reached through button_t
// and one through slider_t.

class control_t
{
public:
    control_t();
    virtual ~control_t();

    virtual void draw();
    virtual void hide();
    virtual void show();

protected:
    int _state;
    int _flags;
};

class button_t : public control_t
{
public:
    button_t();

    void push();
    virtual void draw();

private:
    int _pressed;
};

class slider_t : public control_t
{
public:
    slider_t();

    void move(int pos);

private:
    int _position;
};

// window_draw has TWO control_t sub-objects: one inherited through
// button_t, one through slider_t. An unqualified wd.draw() would be
// ambiguous (the compiler cannot tell which control_t's draw() you mean)
// and does not compile; see main.cpp for how to resolve it.
class window_draw : public button_t, public slider_t
{
public:
    window_draw();
};
