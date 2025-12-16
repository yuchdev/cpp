#pragma once
#include <string>
#include <iostream>

inline int add(int a, int b) { return a + b; }

// Uncomment to trigger ODR multiple definition if included by multiple TUs:
// int bad_add(int a, int b) { return a + b; }

inline int g_counter = 0;                 // C++17 inline variable
inline constexpr int kAnswer = 42;        // C++17 inline constexpr
inline std::string g_name = "hdr-inline"; // OK: inline variable

inline void bump()
{
    ++g_counter;
    std::cout << "bump: " << g_counter << " name=" << g_name << "\n";
}
