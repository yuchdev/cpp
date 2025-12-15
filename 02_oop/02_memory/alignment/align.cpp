#include <iostream>

// Alignment
struct S { char c; int i; };

// Wrong compare
bool is_equal(S a, S b) { return 0 == memcmp(&a, &b, sizeof(S)); }

int main()
{
    return 0;
}

