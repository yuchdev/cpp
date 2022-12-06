/** @brief
Derived classes, copying, virtual functions
*/
#include <string>
#include <vector>

// Plain non-virtual inheritance

// Base and derived classes
class Employee
{

public:
    Employee(const char* n);
    ~Employee();

    void show_name();
protected:
    int _salary;
private:
    string _name;
    short _departament;
};

// Looking at this notation, one might think that the compiler is generating code, 
// which someway express the relationship between classes. This is not true!
// In the raw memory, from the compiler's point of view, the Manager class 
// will look like a monolithic block of memory.
// All methods exist in a single instance.
// But the compiler uses an inheritance tree when performing type conversions.
class Manager : public Employee
{


public:

    // Class methods technically do not differ much from regular functions. 
    // The main difference is that they receive a hidden first parameter `this`
    // A special category are static class methods. They do not receive `this` pointer,
        // and technically they are just plain functions.
    // A static function simply doesn't know for which object it is called.
    // However, `this` can be passed as arguments

    // Constructor is not being inherited, it is exither created by default
    // or implemented explicitly
    Manager(const char* n, short l, int s);

    // Assignment operators and friend functions are not inheritedas well
    // Destructors are called in reverse order of the constructor and are not inherited
    ~Manager();

    // `Manager::show_name()` replaces the base class method with the same name
    void show_name();

    void show_level();
protected:
    // so as `_salary` field
    int _salary;
private:

    vector<Employee*> _group;
    short _level;
};
