#include "abstract.h"

#include <cassert>
#include <iostream>
#include <sstream>

namespace
{
    template <typename F>
    std::string captured_output(F&& f)
    {
        std::ostringstream out;
        std::streambuf* old = std::cout.rdbuf(out.rdbuf());
        f();
        std::cout.rdbuf(old);
        return out.str();
    }
}

#if 0
// shape cannot be instantiated: draw() is pure virtual, and even if it
// weren't, shape's constructor is protected specifically to forbid this.
shape s; // error: cannot declare variable 's' to be of abstract type 'shape'
#endif

// shape is abstract: draw() is pure virtual, and its constructor is
// protected specifically so nobody can instantiate a bare `shape`.
// circle/rectangle/sphere/cube each implement every pure virtual they
// inherit, so they -- and only they -- can be instantiated.
void show_2d_shapes()
{
    point p;

    shape2d* shapes[] = { new circle(), new rectangle() };

    assert(captured_output([&] { shapes[0]->draw(); }) == "circle::draw\n");
    assert(captured_output([&] { shapes[0]->rotate(p); }) == "circle::rotate\n");
    assert(captured_output([&] { shapes[1]->draw(); }) == "rectangle::draw\n");
    assert(captured_output([&] { shapes[1]->rotate(p); }) == "rectangle::rotate\n");

    for (shape2d* s : shapes)
        delete s; // safe: shape::~shape() is virtual
}

void show_3d_shapes()
{
    line px, py, pz;

    shape3d* shapes[] = { new sphere(), new cube() };

    assert(captured_output([&] { shapes[0]->draw(); }) == "sphere::draw\n");
    assert(captured_output([&] { shapes[0]->rotate(px, py, pz); }) == "sphere::rotate\n");
    assert(captured_output([&] { shapes[1]->draw(); }) == "cube::draw()\n");
    assert(captured_output([&] { shapes[1]->rotate(px, py, pz); }) == "cube::rotate\n");

    for (shape3d* s : shapes)
        delete s;
}

int main()
{
    show_2d_shapes();
    show_3d_shapes();
    return 0;
}
