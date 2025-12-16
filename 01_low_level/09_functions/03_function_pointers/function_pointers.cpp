
#include <iostream>
#include <new>

template <typename... Ts>
inline void println(Ts&&... xs)
{
    (std::cout << ... << xs) << '\n';
}

void plain(int) {}
void no_throw(int) noexcept {}

int main()
{
    void (*p1)(int) = plain;
    void (*p2)(int) noexcept = no_throw;

    std::set_new_handler([]{
        println("allocation failed");
        std::abort();
    });

    println("function pointers assigned");
    return 0;
}
