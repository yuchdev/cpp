#pragma once

// Non-type template params could be
// 1. Integer types, including enum
// 2. Pointers with external linkage
// 3. References with external linkage

// Such param can't be static or mutable, keywords const and volatile are ignored
// Always rvalue

// ------------------- Integer type -------------------
template <typename T, int MAX>
class fixed_array
{
public:
    fixed_array();
    T& operator[](int index);
    const T& operator[](int index) const;
private:
    T _arr[MAX];
};

template <typename T, int MAX>
fixed_array<T, MAX>::fixed_array() : _arr() {}

template <typename T, int MAX>
T& fixed_array<T, MAX>::operator [](int index)
{
    if ((index < 0) || (index >= MAX))
        throw std::out_of_range("Array ranges error");
    return _arr[index];
}

template <typename T, int MAX>
const T& fixed_array<T, MAX>::operator [](int index) const
{
    if ((index < 0) || (index >= MAX))
        throw std::out_of_range("Array ranges error");
    return _arr[index];
}

// ------------------- Pointers -------------------
// The keyword typename may be used here
// if the parameter is type (which is often the case)
template <typename T, typename T::allocator* ALLOC>
class List;

// ------------------- References -------------------
template <typename T, int& MAX>
class Deque;
