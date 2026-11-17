#include "simple.h"

#include <iostream>

using std::cout;
using std::endl;

control_t::control_t() : _state(0), _flags(0)
{
    cout << "control_t()\n";
}

control_t::~control_t()
{
    cout << "~control_t()\n";
}

void control_t::draw() { cout << "control_t::draw\n"; }
void control_t::hide() { cout << "control_t::hide\n"; }
void control_t::show() { cout << "control_t::show\n"; }

button_t::button_t() : _pressed(0)
{
    cout << "button_t()\n";
}

void button_t::push() { _pressed = 1; cout << "button_t::push\n"; }
void button_t::draw() { cout << "button_t::draw\n"; }

slider_t::slider_t() : _position(0)
{
    cout << "slider_t()\n";
}

void slider_t::move(int pos) { _position = pos; cout << "slider_t::move\n"; }

window_draw::window_draw()
{
    cout << "window_draw()\n";
}
