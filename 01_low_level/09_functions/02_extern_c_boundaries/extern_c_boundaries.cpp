/*
================================================================================
02_extern_c_boundaries.cpp

Advanced topic: extern "C" boundaries and exception/error policy.

extern "C" is an ABI contract:
  - C linkage (no C++ name mangling)
  - C ABI calling convention
  - no overloading for the same name

extern "C" does NOT magically make things "C compatible":
  - passing std::string / references / templates is ABI-unsafe
  - exceptions MUST NOT cross the boundary

This file demonstrates:
  1) A C-compatible surface using status codes and out-parameters
  2) Translating exceptions into error codes at the boundary
  3) Buffer-writing APIs and size checks
  4) Documented "don't do this" examples
================================================================================
*/

#include <iostream>
#include <cstring>
#include <stdexcept>

template <class... Ts>
void println(Ts&&... xs) { ((std::cout << xs), ...) << "\n"; }

extern "C" {

enum CStatus : int {
    C_OK = 0,
    C_ERR_INVALID = 1,
    C_ERR_EXCEPTION = 2,
    C_ERR_BUFFER_TOO_SMALL = 3,
};

CStatus c_parse_positive_int(const char* s, int* out_value) noexcept;
CStatus c_get_message(int code, char* out_buf, int out_buf_size) noexcept;

} // extern "C"

static int parse_positive_int_cpp(const char* s) {
    if (!s || !*s) throw std::invalid_argument("empty");

    int v = 0;
    for (const char* p = s; *p; ++p) {
        if (*p < '0' || *p > '9') throw std::invalid_argument("non-digit");
        v = v * 10 + (*p - '0');
        if (v < 0) throw std::overflow_error("overflow");
    }
    if (v <= 0) throw std::invalid_argument("not positive");
    return v;
}

extern "C" CStatus c_parse_positive_int(const char* s, int* out_value) noexcept {
    if (!out_value) return C_ERR_INVALID;

    try {
        *out_value = parse_positive_int_cpp(s);
        return C_OK;
    } catch (const std::exception& e) {
        std::cerr << "c_parse_positive_int caught: " << e.what() << "\n";
        *out_value = 0;
        return C_ERR_EXCEPTION;
    } catch (...) {
        std::cerr << "c_parse_positive_int caught unknown exception\n";
        *out_value = 0;
        return C_ERR_EXCEPTION;
    }
}

extern "C" CStatus c_get_message(int code, char* out_buf, int out_buf_size) noexcept {
    if (!out_buf || out_buf_size <= 0) return C_ERR_INVALID;

    const char* msg = (code == 0) ? "ok" : "something went wrong";
    const int need = static_cast<int>(std::strlen(msg)) + 1;

    if (out_buf_size < need) {
        out_buf[0] = '\0';
        return C_ERR_BUFFER_TOO_SMALL;
    }

    std::memcpy(out_buf, msg, static_cast<std::size_t>(need));
    return C_OK;
}

// DON'T DO THIS across extern "C":
// extern "C" void bad(std::string);
// extern "C" void bad_ref(int&);

static void demo_parse(const char* s) {
    int out = -1;
    auto st = c_parse_positive_int(s, &out);
    println("input='", (s ? s : "(null)"), "' status=", st, " out=", out);
}

int main() {
    println("== extern \"C\" boundary demo ==");

    demo_parse("123");
    demo_parse("12x");
    demo_parse("-7");
    demo_parse("");

    char buf[8]{};
    auto st = c_get_message(0, buf, (int)sizeof(buf));
    println("c_get_message status=", st, " buf='", buf, "'");

    char tiny[2]{};
    st = c_get_message(1, tiny, (int)sizeof(tiny));
    println("c_get_message tiny status=", st, " tiny='", tiny, "'");

    println("\nTakeaway: extern \"C\" is ABI. Keep types C-compatible; translate exceptions.");
    return 0;
}
