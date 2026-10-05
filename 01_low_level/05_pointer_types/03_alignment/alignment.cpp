#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

static void show_natural_alignment()
{
    std::cout << "\n== Natural alignment and padding ==\n";

    struct MyStruct
    {
        char c;
        int i;
        double d;
    };

    struct MyStruct2
    {
        double d;
        int i;
        char c;
    };

    std::cout << "alignof(char): " << alignof(char) << '\n';
    std::cout << "alignof(int): " << alignof(int) << '\n';
    std::cout << "alignof(double): " << alignof(double) << '\n';

    std::cout << "alignof(MyStruct): " << alignof(MyStruct) << '\n';
    std::cout << "sizeof(MyStruct): " << sizeof(MyStruct) << '\n';
    std::cout << "offsetof(c/i/d): "
              << offsetof(MyStruct, c) << '/'
              << offsetof(MyStruct, i) << '/'
              << offsetof(MyStruct, d) << '\n';

    std::cout << "alignof(MyStruct2): " << alignof(MyStruct2) << '\n';
    std::cout << "sizeof(MyStruct2): " << sizeof(MyStruct2) << '\n';

    // The alignment of a class is at least sufficient for each non-static data
    // member, but do not phrase the rule as "always exactly the largest member
    // alignment": alignas and implementation choices can make it stronger.
}

static void show_over_alignment()
{
    std::cout << "\n== alignas and over-aligned types ==\n";

    struct alignas(32) CacheLineFragment
    {
        int i;
        char c;
    };

    static_assert(alignof(CacheLineFragment) >= 32);
    static_assert(sizeof(CacheLineFragment) % alignof(CacheLineFragment) == 0);

    std::cout << "alignof(CacheLineFragment): "
              << alignof(CacheLineFragment) << '\n';
    std::cout << "sizeof(CacheLineFragment): "
              << sizeof(CacheLineFragment) << '\n';

    CacheLineFragment local{};
    const auto address = reinterpret_cast<std::uintptr_t>(std::addressof(local));
    std::cout << "runtime address % alignment = "
              << (address % alignof(CacheLineFragment)) << '\n';

    // Since C++17, ordinary new is required to honor over-aligned allocation
    // when the requested type has extended alignment.
    auto owned = std::make_unique<CacheLineFragment>();
    const auto heap_address =
        reinterpret_cast<std::uintptr_t>(owned.get());

    std::cout << "new address % alignment = "
              << (heap_address % alignof(CacheLineFragment)) << '\n';
}

static void show_member_alignment_cost()
{
    std::cout << "\n== Aligning every member is not the same as aligning a vector ==\n";

    struct alignas(16) PackedVector
    {
        float r;
        float g;
        float b;
        float a;
    };

    struct IndividuallyAligned
    {
        alignas(16) float r;
        alignas(16) float g;
        alignas(16) float b;
        alignas(16) float a;
    };

    std::cout << "PackedVector: align=" << alignof(PackedVector)
              << ", size=" << sizeof(PackedVector) << '\n';
    std::cout << "IndividuallyAligned: align=" << alignof(IndividuallyAligned)
              << ", size=" << sizeof(IndividuallyAligned) << '\n';

    std::cout << "IndividuallyAligned offsets: "
              << offsetof(IndividuallyAligned, r) << ", "
              << offsetof(IndividuallyAligned, g) << ", "
              << offsetof(IndividuallyAligned, b) << ", "
              << offsetof(IndividuallyAligned, a) << '\n';

    std::puts("Align the aggregate when SIMD wants one aligned block; aligning every scalar can waste substantial space.");
}

static void show_assume_aligned()
{
    std::cout << "\n== std::assume_aligned (C++20) ==\n";

    alignas(32) int values[8]{1, 2, 3, 4, 5, 6, 7, 8};

    int* p = values;
    int* assumed = std::assume_aligned<32>(p);

    std::cout << "first value through assumed-aligned pointer = "
              << *assumed << '\n';

    // std::assume_aligned is an optimization promise, not a runtime aligner.
    // Passing a pointer that is not actually aligned as promised violates its
    // precondition and gives the optimizer false information.
}

static void show_manual_aligned_storage()
{
    std::cout << "\n== Raw storage alignment and object lifetime ==\n";

    struct Payload
    {
        double x;
        std::uint64_t tag;
    };

    alignas(Payload) std::byte storage[sizeof(Payload)];

    Payload* p = std::construct_at(
        reinterpret_cast<Payload*>(storage),
        Payload{3.5, 99});

    std::cout << "Payload{x=" << p->x << ", tag=" << p->tag << "}\n";

    std::destroy_at(p);

    // Alignment answers "may a Payload live here?".
    // Lifetime answers "does a Payload live here now?".
    // Those are distinct requirements.
}

int main()
{
    show_natural_alignment();
    show_over_alignment();
    show_member_alignment_cost();
    show_assume_aligned();
    show_manual_aligned_storage();
}
