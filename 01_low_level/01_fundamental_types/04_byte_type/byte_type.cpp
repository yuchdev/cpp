// ReSharper disable All
#include <cassert>
#include <cstddef>   // std::byte, std::to_integer
#include <cstdint>
#include <cstring>   // std::memcpy
#include <iostream>
#include <type_traits>

/// std::byte (C++17)
/// std::byte is an enum class intended for raw memory / object representation,
/// to avoid treating bytes as integers.
/// https://en.cppreference.com/w/cpp/types/byte
void byte_type()
{
#if __cplusplus < 201703L
    std::cout << "std::byte requires C++17 (compile with -std=c++17)\n";
    return;
#else

    static_assert(std::is_enum_v<std::byte>, "std::byte is an enum type");
    static_assert(!std::is_integral_v<std::byte>, "std::byte is NOT an integer type");

    // Construction is explicit (no implicit int -> std::byte)
    std::byte b{0x1F};

    // No arithmetic; only bitwise ops
    b |= std::byte{0x80};
    b &= std::byte{0xFF};

    // Convert to integer explicitly
    auto u = std::to_integer<unsigned>(b);
    std::cout << "std::byte b = 0x" << std::hex << u << std::dec << '\n';

    // Obscure but important: safe object representation access via std::byte* (std::byte array)
    std::uint32_t v = 0x12345678u;
    std::byte bytes[sizeof(v)]{};
    std::memcpy(bytes, &v, sizeof(v)); // portable copy of representation

    std::cout << "bytes of uint32_t 0x12345678: ";
    for (std::size_t i = 0; i < sizeof(v); ++i)
    {
        std::cout << std::hex << std::to_integer<unsigned>(bytes[i]) << ' ';
    }
    std::cout << std::dec << '\n';

    // If you really need to view object representation in-place:
    std::byte* raw = reinterpret_cast<std::byte*>(&v);
    std::cout << "first byte in-place: 0x"
              << std::hex << std::to_integer<unsigned>(raw[0]) << std::dec << '\n';

    // NOTE: endianness will affect the observed order of bytes.
    // std::byte does not "fix" endianness; it only fixes type intent.

#endif
}

int main(int argc, char* argv[])
{
    byte_type();
    return 0;
}
