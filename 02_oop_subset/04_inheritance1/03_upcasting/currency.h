#pragma once

class Currency
{
public:
    Currency();
    Currency(double c);
    Currency(const Currency& c);
    ~Currency();

    Currency& operator=(const Currency& c);

    // Operator virtuality
    // 1. If the same actions are performed in the base and derived classes, 
    // it is not necessary to redefine the operators and make them virtual. 
    // A reference polymorphism kicks in, and the derived class is accepted as the base class by the reference
    Currency& operator+=(const Currency& c);
    Currency& operator-=(const Currency& c);

    // 2. But if there is a complex system of inheritance 
        // with overriding operations in derived classes takes place (a rare phenomenon),
    // operators can be virtual (just for the sake of example)
    virtual Currency& operator*=(const Currency& c);
    virtual Currency& operator/=(const Currency& c);

    friend Currency operator+(const Currency& a, const Currency& b);
    friend Currency operator-(const Currency& a, const Currency& b);
    friend Currency operator*(const Currency& a, const Currency& b);
    friend Currency operator/(const Currency& a, const Currency& b);



    void print() const;
protected:
    double _currency;
};

class Usd : public Currency
{
public:
    Usd(double d);
    void print();

    // In order to work correctly with instances of the base class returned by value
    // overload the downcasting operator=()
    // this will allow to transform a base class into a derived one
    Usd& operator=(const Currency& c);

    // Define the transform constructor as well
    Usd(const Currency& c);

    // This is just an example of operator virtualization
    virtual Currency& operator*=(const Currency& c);
    virtual Currency& operator/=(const Currency& c);
};

// You cannot inherit from built-in types!
//class ext_int : public int {
//	int _i;
//	int _a;
//	ext_int( int i ) : _i(i), _a(i){}
//};
