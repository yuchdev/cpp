#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>

// Class size is not the sum of its members: the compiler inserts padding
// so each member lands on an address satisfying its own alignment
// requirement.
struct S
{
    char c;
    int i;
};

void show_padding_exists()
{
    // sizeof(char) + sizeof(int) is 5 on every mainstream platform; the
    // actual size is larger because `i` needs 4-byte alignment and `c`
    // only occupies 1 of the 4 bytes before it.
    assert(sizeof(char) + sizeof(int) < sizeof(S));
    assert(sizeof(S) % alignof(int) == 0); // trailing padding too, so arrays of S stay aligned
    std::cout << "sizeof(S) = " << sizeof(S) << " (not " << (sizeof(char) + sizeof(int)) << ")\n";
}

// memcmp compares ALL bytes of the object representation, padding
// included. Padding bytes are not specified to hold any particular
// value, and are not guaranteed equal between two objects that are
// logically equal in every named member -- so a bytewise comparison can
// report "different" for objects whose actual data is identical.
void show_memcmp_is_unsafe_for_padded_types()
{
    // Pre-fill both buffers with DIFFERENT garbage before constructing
    // into them -- placement-constructing S{c, i} only writes the two
    // named members; it does not (and is not obliged to) touch the
    // padding byte(s) between them, so whatever was already in that
    // byte survives construction.
    alignas(S) unsigned char buf1[sizeof(S)];
    alignas(S) unsigned char buf2[sizeof(S)];
    std::memset(buf1, 0xAA, sizeof(S));
    std::memset(buf2, 0xBB, sizeof(S));

    S* a = new (buf1) S { 'x', 42 };
    S* b = new (buf2) S { 'x', 42 };

    // Logically identical: every named member matches.
    assert(a->c == b->c && a->i == b->i);

    // ...yet a bytewise comparison disagrees, because the padding byte
    // between c and i is still 0xAA in one and 0xBB in the other.
    bool bytewise_equal = (0 == std::memcmp(a, b, sizeof(S)));
    assert(!bytewise_equal);

    std::cout << "members equal, memcmp says different: "
              << std::boolalpha << !bytewise_equal << '\n';

    // The correct comparison is member-wise, which this wrong version
    // deliberately gets right by NOT touching the padding at all:
    bool is_equal = (a->c == b->c) && (a->i == b->i);
    assert(is_equal);
}

// alignof queries a type's required alignment; alignas demands a
// (stronger) one for a specific declaration.
void show_alignof_and_alignas()
{
    assert(alignof(char) == 1);
    assert(alignof(int) >= 4);
    assert(alignof(S) == alignof(int)); // a struct's alignment is (at least) its strictest member's

    // alignas(16) char buffer[64] forces 16-byte alignment, e.g. for SIMD
    // loads that fault (or are slow) on misaligned addresses.
    alignas(16) char simd_buffer[64];
    assert(reinterpret_cast<std::uintptr_t>(simd_buffer) % 16 == 0);

    // alignas can only STRENGTHEN a type's alignment, never weaken it
    // below what the type would naturally require. This isn't merely
    // "ignored" -- requesting a weaker alignment than the type's own
    // strictest member needs is ILL-FORMED, a hard compile error, not a
    // silent downgrade or a packing request:
#if 0
    struct alignas(1) weaker_than_natural { int i; }; // error: requested
        // alignment (1) is less than the minimum alignment (4) for this type
#endif
    // alignas(alignof(int)) is fine -- it just restates what the
    // compiler already enforces.
    struct alignas(alignof(int)) matches_natural { int i; };
    assert(alignof(matches_natural) == alignof(int));
}

// Forming a typed pointer over raw/packed/arbitrary-offset bytes is
// dangerous even if you never dereference it through that pointer -- it
// invites a later `*pv` during a refactor, and on architectures stricter
// than x86/x64 (some ARM modes, SPARC, older MIPS/PowerPC), an actual
// misaligned load can fault outright rather than just run slower.
std::uint32_t decode_unsafe(const void* p)
{
    const auto* pv = static_cast<const std::uint32_t*>(p); // forms a possibly-misaligned pointer
    return *pv; // the actual fault, if p isn't 4-byte aligned
}

// The fix: never materialize the typed pointer at all. memcpy (or
// std::bit_cast under the same size/trivially-copyable constraints) reads
// through an untyped byte pointer and is well-defined regardless of p's
// alignment.
std::uint32_t decode_safe(const void* p)
{
    std::uint32_t v;
    std::memcpy(&v, p, sizeof(v));
    return v;
}

void show_safe_decode_of_misaligned_data()
{
    // Deliberately misalign: a 5-byte buffer, reading a uint32_t starting
    // at offset 1 is NOT 4-byte aligned on a typical platform.
    unsigned char raw[5] = { 0x00, 0x78, 0x56, 0x34, 0x12 };
    const void* misaligned = raw + 1;

    std::uint32_t value = decode_safe(misaligned);
    assert(value == 0x12345678u); // little-endian reconstruction, byte-exact

    // decode_unsafe(misaligned) is NOT called here: on x86/x64 it would
    // likely just work (slower); on a stricter target it can fault. The
    // point is that it compiles cleanly either way, which is exactly why
    // this class of bug survives code review until it ships to different
    // hardware.
    (void)&decode_unsafe;
}

int main()
{
    show_padding_exists();
    show_memcmp_is_unsafe_for_padded_types();
    show_alignof_and_alignas();
    show_safe_decode_of_misaligned_data();
    return 0;
}
