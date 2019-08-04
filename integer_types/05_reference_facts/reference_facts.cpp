#include <iostream>

// Facts about references

void show_references(){

    int ii = 10;

    // Reference must be initialized, and could not be re-initialized
    int& rr = ii;

    // When we meet function T& ref = boo(),
    // we expect that object lives long enough not to create "dead" reference
}

int main()
{
    return 0;
}
