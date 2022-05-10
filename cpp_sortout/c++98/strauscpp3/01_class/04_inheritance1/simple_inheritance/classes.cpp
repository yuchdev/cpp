#include "classes.h"


// Show methods of plain non-virtual inheritance
Employee::Employee(const char* n) : _name(n)
{

}

Employee::~Employee() {}

// If the base class does not have a default constructor,
// In a derived class, it must be called explicitly
Manager::Manager(const char* n, short l, int s)
    : Employee(n), _level(l), _salary(s)
{

    // since the _salary field has the same name as in the Employee class
    // it remains uninitialized there
    std::cout << "Employee::_salary=" << Employee::_salary << '\n';
}

Manager::~Manager() {}

void Employee::show_name()
{
    cout << _name << endl;
}

void Manager::show_level()
{
    cout << _level << endl;
}

void Manager::show_name()
{
    cout << "Manager: ";
    // An explicit call of the base class method
    Employee::show_name();
}
