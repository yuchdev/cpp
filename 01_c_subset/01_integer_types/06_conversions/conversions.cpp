#include <utilities/defines.h>
SUPPRESS_PRAGMA_WARNINGS()

#include <utilities/bitwise.h>
#include <utilities/defines.h>
SUPPRESS_UNSIGNED_COUNTEREXAMPLE_WARNINGS()

#include <iostream>
#include <cstdint>

// Fundamental C++ types
// https://en.cppreference.com/w/cpp/language/types



uint8_t safe_avg(uint8_t a, uint8_t b)
{
    return a/2 + b/2;
}
// What if we pass uint32_t instead of uint8_t

uint8_t truncate(uint32_t u)
{
    // no warnings?
    return u;
}

// TODO: int promotion traits
void numeric_conversions()
{
    // Unlike the promotions, numeric conversions may change the values, with potential loss of precision

    // Narrowing conversion - from larger to smaller

    // Truncation of 32-bit
    // TODO: no warning?
    uint32_t trunc_me = 0xffff;
    uint8_t truncated_uint = truncate(trunc_me);
    // Again, this is legacy on ANSI C. 
    // In modern languages like Rust problem of truncation and narrowing conversions does not exist
    // However, all explicit casts do not specify source type, only destination

    // warning: conversion with possible loss of data, even 1 is perfectly fit
    long long wide_ll = 1;

    // Clang-tidy: narrowing conversion from long long to int is implementation-defined
    long narrow_l = wide_ll;

    // When signed integer arithmetic operation overflows (the result does not fit), the behavior is undefined:
    // it may cut value, it may trap on some platforms or due to compiler options (e.g. -ftrapv in GCC and Clang), 
    // it can sometimes saturate to minimal or maximal value (on many DSPs), 
    // or may be completely optimized out by the compiler

    // floating-point narrowing conversion loses precision, implementation-defined
    // If narrow type can't represent wide type, it is UB
    float narrow_f = 1.5f;
    double wide_d = narrow_f;

    // fp -> integer leads to truncation
    int narrow_i = narrow_f;
}


int main(int argc, char* argv[])
{
    numeric_conversions();
    return 0;
}
