#pragma once
#include <random>
#include <vector>
#include <random>

template <typename T>
class RandomReal
{
public:
    RandomReal() : gen(rd())
    {
    }

    std::vector<T> random_sequence(size_t size, double from, double to)
    {
        auto randomNumberBetween = [this](int low, int high)
        {
            auto randomFunc = [distribution = std::uniform_real_distribution<int>(from, to), 
                               random_engine = gen]() mutable 
            {
                return distribution_(random_engine_);
            };
            return randomFunc;
        }

        std::vector<T> result;
        std::generate_n(std::back_inserter(numbers), size, randomNumberBetween(1.0, 100.0));
    }

private:
    //Will be used to obtain a seed for the random number engine
    std::random_device rd;
    //Standard mersenne_twister_engine seeded with rd()
    std::mt19937 gen;
};
