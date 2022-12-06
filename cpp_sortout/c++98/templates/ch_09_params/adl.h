#pragma once

// http://en.wikipedia.org/wiki/Argument-dependent_name_lookup

/*
Example of ADL

#include<iostream>

int main()
{
    string hello = "Hello World, where did operator<<() come from?";
    std::cout << hello << std::endl; // (using namespace std is not defined, however we found operator<< by its arguments)
}

*/