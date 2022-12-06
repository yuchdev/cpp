#pragma once

// An explicit instantiation model
// You can explicitly instantiate all the types you may need in a *.cpp file
// Doing so, you can separate the definition and declaration in *.cpp and *.h
template <typename T>
class direct_instance
{
public:
    direct_instance(const T& t);
    ~direct_instance();
    T test() const;
private:
    T _x;
};

// The disadvantage of this method is that you have to explicitly watch over all instantiated instances
// (Not recommended for large projects)
