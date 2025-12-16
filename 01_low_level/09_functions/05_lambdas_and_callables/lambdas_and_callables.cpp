
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>

template <typename... Ts>
inline void println(Ts&&... xs)
{
    (std::cout << ... << xs) << '\n';
}

int main()
{
    std::vector<int> v{1,2,3,4,5};

    int offset = 10;

    // Lambda with capture
    std::for_each(v.begin(), v.end(), [offset](int x){
        println(x + offset);
    });

    // Mutable lambda
    int state = 0;
    auto inc = [state]() mutable {
        return ++state;
    };
    println(inc(), inc(), inc());

    // Lambda as std::function (recursive)
    std::function<int(int)> fact = [&](int n){
        return n <= 1 ? 1 : n * fact(n-1);
    };
    println("fact(5) =", fact(5));

    return 0;
}
