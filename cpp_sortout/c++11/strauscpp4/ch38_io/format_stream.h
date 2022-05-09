#pragma once
#include <iosfwd>
#include <limits>

namespace cpp4
{

void show_formattings_flags();

void show_formattings_methods();

void show_formattings_manipulators();

void show_formatted_input();

void show_user_manip();

} // namespace cpp4


namespace cpp4
{

// User-defined manipulators
// W/o argumwnts
std::ostream& tab(std::ostream& os);

// With arguments, so-called effector class
template <typename T> class binary;
template <typename T>
std::ostream& operator<<(std::ostream& os, const binary<T>& t);

template <typename T>
class binary
{
public:
    binary(T n) :_num(n) {}
    friend std::ostream& operator << <> (std::ostream& os, const binary<T>& t);
private:
    T _num;
};

template <typename T>
std::ostream& operator << (std::ostream& os, const binary<T>& t);

} // namespace cpp4 
