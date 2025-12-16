#include <cstddef>
#include <cstdint>
#include <iostream>

// Demonstrates C/C++ ABI boundary patterns.
//
// `extern "C"` disables C++ name mangling for a symbol.
// This is REQUIRED when a C program links against a function compiled as C++,
// or when you want to find a symbol via dlsym/GetProcAddress.
//
// RULES AT C ABI BOUNDARY:
// - Only use C-compatible types in exported signatures.
// - NEVER let exceptions cross into C (wrap with try/catch, return error codes).
// - NEVER expose std::string/std::vector/classes in C ABI.
// - Use opaque handles when you need objects.

using c_callback_t = void(*)(int code, void* user);

extern "C" int c_api_register_and_fire(c_callback_t cb, void* user)
{
    try {
        if (!cb) return -1;   // null callback
        cb(123, user);
        return 0;
    } catch (...) {
        // Never allow exceptions to escape into C.
        return -2;
    }
}

static void my_callback(int code, void* user)
{
    // The `user` pointer is opaque: C API does not interpret it.
    auto* value = static_cast<int*>(user);
    std::cout << "callback code=" << code
              << " user=" << (value ? *value : 0) << "\n";
}

int main()
{
    int payload = 7;
    int rc = c_api_register_and_fire(&my_callback, &payload);
    std::cout << "rc=" << rc << "\n";
    return 0;
}
