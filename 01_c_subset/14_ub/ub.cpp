#include <iostream>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wuninitialized"
void initialization_order()
{
    // example of UB due to bad initialization
    int i = i * 0;

    // no defined order of initialization within one expression
}
#pragma clang diagnostic pop

int main()
{
    return 0;
}
