
#include <iostream>

template <typename... Ts>
inline void println(Ts&&... xs)
{
    (std::cout << ... << xs) << '\n';
}

[[deprecated("use new_function instead")]]
void old_function()
{
    println("old function");
}

void new_function()
{
    println("new function");
}

int main()
{
#ifdef __cplusplus
    println("__cplusplus =", __cplusplus);
#endif

    old_function();
    new_function();
    return 0;
}
