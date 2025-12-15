#pragma once
#include <iosfwd>

namespace cpp
{

void file_stream_write();

void file_stream_read();

void file_stream_copy();

void file_stream_binary_write();

void file_stream_binary_read();

void show_rnd_access();

void show_fstream_serialize();

// Self-generated random array with serialization methods
// They are usually inherited from some interface
struct RandomArray
{

    size_t sz;
    int* arr;

    RandomArray(size_t s) :sz(s), arr(new int[sz])
    {
        for (int i = 0; i < 10; i++) {
            arr[i] = 0;
        }
    }

    void generate()
    {
        for (int i = 0; i < 10; i++) {
            arr[i] = rand();
        }
    }

    ~RandomArray()
    {
        delete[] arr;
    }

    //
    void load(std::ifstream& f);

    //
    void save(std::ofstream& f);
};

} // namespace cpp
