#pragma once
#include <bitset>

/// Returns bitset with bitwise representation of 'value'
template <typename T>
std::bitset<sizeof(T) * 8> bitwise(T value)
{
    constexpr size_t bits_size = sizeof(T) * 8;
    union
    {
        T native_repr;
        uint64_t bitwise_repr;
    } f;
    f.native_repr = value;

    // bitset c-tor accept unsigned only
    return std::bitset<bits_size>(f.bitwise_repr);
}

