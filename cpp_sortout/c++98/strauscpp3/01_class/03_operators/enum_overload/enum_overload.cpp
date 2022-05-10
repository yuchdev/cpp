#include <iostream>

// Let's show enumeration overload
enum Wday { mon = 1, tus, wed, thr, frd, sat, sun };

Wday& operator++(Wday& d)
{
    return d = (d == sun) ? mon : Wday(d + 1);
}


int main()
{
    Wday d = sun;
    ++d;
    ++d;
    return 0;
}
