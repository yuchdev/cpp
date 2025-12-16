#include <iostream>

// Demonstrates core C++ features absent in C, plus a correct interop pattern.
//
// C++ provides:
// - classes with constructors/destructors (RAII)
// - namespaces, overloads, templates, exceptions
// - references, new/delete, operator overloading
//
// When exposing C++ to C, use a C ABI "facade": opaque handles + create/destroy + error codes.
// Never expose C++ objects directly.

class Accumulator
{
public:
    void add(int x) { sum_ += x; }
    int sum() const { return sum_; }
private:
    int sum_ = 0;
};

extern "C" {

// Opaque handle type for C callers.
// C code only sees `struct AccHandle;` and uses pointers to it.
struct AccHandle {
    Accumulator* impl;
};

AccHandle* acc_create()
{
    try {
        return new AccHandle{ new Accumulator() };
    } catch (...) {
        return nullptr;
    }
}

void acc_destroy(AccHandle* h)
{
    if (!h) return;
    delete h->impl;
    delete h;
}

int acc_add(AccHandle* h, int x)
{
    if (!h || !h->impl) return -1;
    try {
        h->impl->add(x);
        return 0;
    } catch (...) {
        return -2;
    }
}

int acc_sum(const AccHandle* h, int* out_sum)
{
    if (!h || !h->impl || !out_sum) return -1;
    *out_sum = h->impl->sum();
    return 0;
}

} // extern "C"

int main()
{
    AccHandle* h = acc_create();
    if (!h) {
        std::cout << "acc_create failed\n";
        return 1;
    }

    acc_add(h, 10);
    acc_add(h, 20);

    int s = 0;
    acc_sum(h, &s);
    std::cout << "sum=" << s << "\n";

    acc_destroy(h);
    return 0;
}
