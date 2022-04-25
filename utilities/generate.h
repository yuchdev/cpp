#pragma once
#include <random>
#include <vector>

template <typename T>
class RandomReal
{
public:
    RandomReal() : gen(rd())
    {
    }

    /// @brief Generate a random sequence of numbers between 'to' and 'from'
    std::vector<T> generate(T from, T to, size_t size)
    {
        std::vector<T> result;
        result.reserve(size);
        std::uniform_real_distribution<T> dist(from, to);
        for (size_t i = 0; i < size; ++i)
        {
            result.push_back(dist(gen));
        }
        return result;
    }

    //Standard mersenne_twister_engine seeded with rd()
    std::mt19937 gen;

private:
    //Will be used to obtain a seed for the random number engine
    std::random_device rd;
};
