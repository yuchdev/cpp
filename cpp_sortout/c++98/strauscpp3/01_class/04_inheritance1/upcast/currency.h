#pragma once

class currency_t
{
public:
    currency_t();
    currency_t(double c);
    currency_t(const currency_t& c);
    ~currency_t();

    currency_t& operator=(const currency_t& c);

    // Operator virtuality
    // 1. If the same actions are performed in the base and derived classes, 
    // it is not necessary to redefine the operators and make them virtual. 
    // A reference polymorphism kicks in, and the derived class is accepted as the base class by the reference
    currency_t& operator+=(const currency_t& c);
    currency_t& operator-=(const currency_t& c);

    // 2. But if there is a complex system of inheritance 
        // with overriding operations in derived classes takes place (a rare phenomenon),
    // operators can be virtual (just for the sake of example)
    virtual currency_t& operator*=(const currency_t& c);
    virtual currency_t& operator/=(const currency_t& c);

    friend const currency_t operator+(const currency_t& a, const currency_t& b);
    friend const currency_t operator-(const currency_t& a, const currency_t& b);
    friend const currency_t operator*(const currency_t& a, const currency_t& b);
    friend const currency_t operator/(const currency_t& a, const currency_t& b);



    void print();
protected:
    double _currency;
};

class roubles_t : public currency_t
{
public:
    roubles_t(double d);
    void print();

    // In order to work correctly with instances of the base class returned by value
    // overload the downcasting operator=()
    // this will allow to transform a base class into a derived one
    roubles_t& operator=(const currency_t& c);

    // Define the transform constructor as well
    roubles_t(const currency_t& c);

    // This is just an example of operator virtualization
    virtual currency_t& operator*=(const currency_t& c);
    virtual currency_t& operator/=(const currency_t& c);
};

// You cannot inherit from built-in types!
//class ext_int : public int {
//	int _i;
//	int _a;
//	ext_int( int i ) : _i(i), _a(i){}
//};
