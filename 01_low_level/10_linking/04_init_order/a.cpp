#include <string>
#include <iostream>

extern std::string& global_name();

std::string a_value = "a sees: " + global_name();

void print_a()
{
    std::cout << "a_value=" << a_value << "\n";
}
