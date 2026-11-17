#include "abstract.h"

// shape is abstract: draw() is pure virtual, and its constructor is
// protected specifically so nobody can instantiate a bare `shape`.
// circle/rectangle/sphere/cube each implement every pure virtual they
// inherit, so they -- and only they -- can be instantiated.
void show_2d_shapes()
{
    point p;

    shape2d* shapes[] = { new circle(), new rectangle() };

    for (shape2d* s : shapes) {
        s->draw();
        s->rotate(p);
    }

    for (shape2d* s : shapes)
        delete s; // safe: shape::~shape() is virtual
}

void show_3d_shapes()
{
    line px, py, pz;

    shape3d* shapes[] = { new sphere(), new cube() };

    for (shape3d* s : shapes) {
        s->draw();
        s->rotate(px, py, pz);
    }

    for (shape3d* s : shapes)
        delete s;
}

int main()
{
    show_2d_shapes();
    show_3d_shapes();
    return 0;
}
