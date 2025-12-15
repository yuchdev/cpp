#include "file_stream.h"
#include <iostream>
#include <fstream>
#include <ctime>

using namespace std;
using namespace cpp;

// C++ app deal not with a file, but with an abstract stream
// real files can be connect-disconnect to the stream
// It is useful to apply RAII with a file

// file streams are buffered, so aborted program do not flush buffer on the disk

// text streams are formatted and could be splitted into strings
// binary does not


void cpp::file_stream_write()
{
    srand(static_cast<unsigned>(time(0)));

    // write 10 random numbers to file
    ofstream f("random.txt");
    for (int i = 0; i < 10; i++) {
        f << rand() % 10 << '\n';
    }
}


void cpp::file_stream_read()
{

    // file stream could be connected-disconnected to any file any number of times

    // read string-by-string from the text file
    ifstream f("random.txt");
    while (!f.eof()) {
        char buf[20] = {};	// enough for int
        f.getline(buf, 20);
        cout << buf << endl;
    }
}

void cpp::file_stream_copy()
{

    const std::string& from("random.txt");
    const std::string& to("random2.txt");

    ifstream inf(from);
    ofstream outf(to);
    char c = 0;
    // copy symbol-by-symbol
    while (inf.get(c)) outf.put(c);
}

void cpp::file_stream_binary_write()
{
    srand(static_cast<unsigned>(time(0)));

    // generate 10 random numbers
    cout << "Generate random array\n";
    int a[10] = {};
    for (int i = 0; i < 10; i++) {
        a[i] = rand();
        cout << a[i] << '\n';
    }

    // write them to the binary file
    // accept flags (create, rewrite etc)
    ofstream f("random.bin", ios::binary);


    // write all array at once
    size_t sz = sizeof(a);
    f.write(reinterpret_cast<char*>(a), sz);
}


void cpp::file_stream_binary_read()
{
    int a[10] = {};

    // read all array at once
    cout << "Read random array\n";
    ifstream f("random.bin", ios::binary);
    size_t sz = sizeof(a);
    f.read(reinterpret_cast<char*>(a), sz);
    for (int i = 0; i < 10; i++) {
        cout << a[i] << '\n';
    }
}


void cpp::RandomArray::load(std::ifstream& f)
{
    if (f.is_open()) {
        // read array size
        f.read(reinterpret_cast<char*>(&sz), sizeof(size_t));

        // read array itself
        f.read(reinterpret_cast<char*>(arr), sz * sizeof(int));
    }
}

void cpp::RandomArray::save(std::ofstream& f)
{
    if (f.is_open()) {
        // write array size
        f.write(reinterpret_cast<char*>(&sz), sizeof(size_t));

        // write array itself
        f.write(reinterpret_cast<char*>(arr), sz * sizeof(int));
    }
}

void cpp::show_rnd_access()
{

    // functions family tell/seek allow to set a position 
    // for reading-writins in a filestream

    // open for reading
    {
        ifstream f("random.bin", ios::binary);

        // current absolute position (for reading)
        ios::pos_type read_pos = f.tellg();

        // move to absolute position for reading
        // (param could be pos_type or int)
        f.seekg(0);

        // read one value
        int out = 0;
        f.read(reinterpret_cast<char*>(&out), sizeof(int));

        // relative readong position 
        // could be from user offset, from begin, cut, end
        // e.g. set to the end
        f.seekg(0, ios::end);

        // trying to write over the file end set the stream to bad
        // or throw an exception it the flag is set
    }

    // open for writing
    {
        // ios::in should be set
        // without it file is truncated!
        ofstream f("random.bin", ios::in | ios::binary/*|ios::trunc*/);

        // current absolute writing position
        streampos write_pos = f.tellp();

        // move to absolute writing position
        // (param could be pos_type or int)
        f.seekp(0, ios::beg);

        // writing
        int in = 100;
        f.write(reinterpret_cast<char*>(&in), sizeof(int));

        // move to relative position
        // e.g. set current + 1 int (4 bytes)
        f.seekp(4, ios::cur);

        // writing
        in = 200;
        f.write(reinterpret_cast<char*>(&in), sizeof(int));
    }
}


void cpp::show_fstream_serialize()
{

    {
        cpp::RandomArray ra(10);
        ra.generate();
        ofstream f("rarray.bin", ios::binary);
        ra.save(f);
    }

    {
        cpp::RandomArray ra(10);
        ifstream f("rarray.bin", ios::binary);
        ra.load(f);
    }
}
