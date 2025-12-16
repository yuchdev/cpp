
#include <iostream>
#include <string>
#include <cassert>

// Modern replacement for macro-based printing
template <typename... Ts>
inline void println(Ts&&... xs)
{
    (std::cout << ... << xs) << '\n';
}

// Top-level const ignored in function types
void f(int);
void f(const int); // same as above

// Default arguments
int add(int a, int b = 10)
{
    return a + b;
}

// constexpr function
constexpr int square(int x)
{
    return x * x;
}

// [[nodiscard]]
[[nodiscard]] int must_use()
{
    return 42;
}

// [[noreturn]]
[[noreturn]] void fatal()
{
    std::exit(1);
}

// Local static initialization (thread-safe since C++11)
int counter()
{
    static int c = 0;
    return ++c;
}

int main()
{
    println("add:", add(1));
    static_assert(square(4) == 16);
    println("counter:", counter());
    println("counter:", counter());
    // fatal(); // uncomment to test [[noreturn]]
    return 0;
}
