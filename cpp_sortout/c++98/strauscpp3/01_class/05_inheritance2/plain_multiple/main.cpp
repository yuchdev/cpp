#include <iostream>
#include <typeinfo>

#include "virtual.h"
#include "simple.h"
#include "final.h"
#include "multimethod.h"
#include "member_pointer.h"

#pragma pack(1)

using std::cout;
using std::endl;
using std::type_info;

// Demonstration of simple multiple inheritance
void show_simple()
{

    size_t sz = 0;

    {
        control_t control;
        // Here and below we check the size of class
        // control_t = 8 (2x int)
        sz = sizeof(control);
        control.draw();
        control.hide();
        control.show();
    }

    {
        button_t button;
        // the object includes control_t + button_t = 12
        sz = sizeof(button);

        // Methods overridden in button_t
        button.push();
        button.draw();

        // Methods not redefined in derived class
        // and called from the parent
        button.hide();
        button.show();
    }

    {
        slider_t slider;
        // the object includes control_t + slider_t = 20 (5x int)
        sz = sizeof(slider);

        // call method from Slider
        slider.move(0);

        // call methods from parent class
        slider.draw();
        slider.hide();
        slider.show();
    }

    {
        // the control_t() constructor is called twice here
        // the sequence is as follows:
        // control_t->slider_t->control_t->button_t->window_draw
        // In other words, constructors are being called in the order declared during inheritance
        // control_t constructor is called twice
        // (see simple.h)
        window_draw drawer;

        // the object includes control_t x2 + button_t + slider_t + window_draw = 36
        // It means, control_t class is included twice
        sz = sizeof(drawer);

        // call methds from Drawer
        drawer.draw();
        drawer.hide();
        drawer.show();

        // call methods from parent class
        // (each of them has only 1 copy, so it works fine)
        drawer.move(0);
        drawer.push();
    }
}

int main()
{

    show_simple();
    show_vitrual();
    show_sizes();
    return 0;
}
