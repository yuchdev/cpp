#include <string>

std::string& global_name()
{
    static std::string name = "B_global_name";
    return name;
}
