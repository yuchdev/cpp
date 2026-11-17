#include "classes.h"

// Plain, non-virtual inheritance: show_name() is not declared virtual anywhere
// in classes.h, so which version runs is decided by the STATIC type of the
// expression used to call it, not by the dynamic type of the object.
void show_static_binding()
{
    Manager m("Alice", 3, 90000);

    // Called through its own (derived) type: Manager::show_name() runs.
    m.show_name();

    // Called through a base-class reference: because show_name() is not
    // virtual, this calls Employee::show_name() even though the object is
    // actually a Manager. This is exactly the "non-virtual inheritance"
    // behavior simple_inheritance.h's comments describe.
    Employee& e = m;
    e.show_name();
}

// Manager re-declares a field named _salary, which hides (does not override
// or share storage with) Employee::_salary. The object has two independent
// _salary sub-objects; which one a given piece of code sees depends on
// which class's scope that code is written in.
void show_field_hiding()
{
    Manager m("Bob", 1, 50000);
    m.show_level();
}

int main()
{
    show_static_binding();
    show_field_hiding();
    return 0;
}
