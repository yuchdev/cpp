
#include <cstdio>
extern "C" {
#include <string.h>
}

namespace cwrap {
inline size_t length(const char* s) { return strlen(s); }
}

int main()
{
    const char* s = "hello";
    std::printf("len=%zu\n", cwrap::length(s));
    return 0;
}
