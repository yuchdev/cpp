#pragma once

// Inclusion model

template <typename T>
class include_model
{
public:
    include_model(const T& t);
    ~include_model();
    T test() const;
private:
    T _x;
};

// The definition cannot be put into *.cpp file, because it is not known yet at compile time,
// what exactly type of template instance must be instantiated.

// As a result, the "inclusion model" is used,
// in other words, defining a template class and methods in the header

// This causes a problem of increasing of included heders size 
// Note, all standard heders from header with the template also have to be included


// Defenition:
template <typename T>
include_model<T>::include_model(const T& t) : _x(t) {}

template <typename T>
include_model<T>::~include_model(void) {}

template <typename T>
T include_model<T>::test() const
{
    return _x;
}
