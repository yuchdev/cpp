#include "simple_stream.h"
#include "file_stream.h"
#include "format_stream.h"
#include "formatted_user_input.h"
#include "c_library_io.h"

#include <iostream>
#include <iterator>
#include <fstream>
#include <algorithm>
#include <vector>

/*

New features:

Examples:
1. Standard input examples
2. Standard output examples
3. String output examples
4. String streams
5. Other istream examples
6. Standard file stream examples
7. Serialize fstream examples
8. Random file access
9. Formatting streams: flags, methods and manipulators
10. User-defined formatting
11. User-defined input
12. Iterators

*/

// 1. Standard input examples
void show_standard_streams_out()
{
    cpp::simple_ostream();
}


// 2. Standard output examples
void show_standard_streams_input()
{
    cpp::simple_istream();
    cpp::istream_end_input();
    cpp::istream_noskipws();
    cpp::istream_get_read();
}

// 3. String output examples
void show_string_input()
{
    cpp::istream_strings_get();
    cpp::istream_strings_ignore();
    cpp::istream_strings_getline();
}

// 4. String streams
void show_string_streams()
{
    cpp::output_string_streams();
    cpp::bidirect_string_streams();
}



// 5. Other istream examples
void show_istream_other()
{
    cpp::stream_bad_state();
    cpp::stream_place_back();
    cpp::stream_redirect_buffers();
    cpp::stream_tie();
}

// 6. Standard file stream examples
// 6. Serialize fstream examples
// 8. Random file access
void show_simple_fstream()
{

    // C++ app deal not with a file, but with an abstract stream
    // real files can be connect-disconnect to the stream
    // It is useful to apply RAII with a file

    // file streams are buffered, so aborted program do not flush buffer on the disk

    // text streams are formatted and could be splitted into strings
    // binary does not

    cpp::file_stream_write();
    cpp::file_stream_read();
    cpp::file_stream_copy();
    cpp::file_stream_binary_write();
    cpp::file_stream_binary_read();
    cpp::show_fstream_serialize();

    cpp::show_rnd_access();
    // check what we write
    cpp::file_stream_binary_read();
}


// 9. Formatting streams: flags, methods and manipulators
// 10. User-defined formatting
void show_formatted_streams()
{
    cpp::show_formattings_flags();
    cpp::show_formattings_methods();
    cpp::show_formattings_manipulators();
    cpp::show_formatted_input();
    cpp::show_user_manip();
}

// 11. User - defined input
void show_user_defined_input()
{
    cpp::show_formatted_overloads();
}


// 13. Iterators
namespace cpp
{

// Show output iterator
void show_out_iter()
{
    // Stream as a container example
    // Assigning iterator == out to the console
    // 2nd optional param is a terminator for out every value
    std::ostream_iterator<std::string> oi(cout);
    *oi = "aaa";
    ++oi;
    *oi = " bbb\n";
}

// Show iterator that refers to the input stream
// String that assigns passed from the keyboard
void show_istr_iter()
{

    // request for the first string
    std::istream_iterator<std::string> iinput_iter(cin);

    // iterator has that value
    string s1 = *iinput_iter;

    // request for the second string
    ++iinput_iter;

    // iterator has that value
    string s2 = *iinput_iter;

    // Result
    cout << "First string: " << s1 << '\n' << "Second string: " << s2 << endl;
}

int show_iterators()
{
    // TODO: write test file first
    // does not work because of OOS cmake build

    // Iterator for invitation cout
    std::ostream_iterator<std::string> out_iter(cout);

    // Invite to input first filename
    *out_iter = "Input file:";

    // Read input filename (test.txt)
    // Initialize istream by cin
    std::istream_iterator<std::string> iinput_iter(cin);
    std::string from = *iinput_iter;

    // Invite to input second filename (test1.txt)
    ++out_iter;
    *out_iter = "Output file:";

    ++iinput_iter;
    std::string to = *iinput_iter;

    // file read from
    std::ifstream from_file(from.c_str());

    // iterator to read from file (the stream as a container)
    std::istream_iterator<std::string> from_it_begin(from_file);

    // Default istream iterator constructor creates eof
    std::istream_iterator<std::string> from_it_end;

    // From file to vector
    std::vector<std::string> b(from_it_begin, from_it_end);

    // sort values
    std::sort(b.begin(), b.end());

    // write to that file
    std::ofstream to_file(to.c_str());

    // Write to file iterator
    ostream_iterator<string> to_it_begin(to_file, "\n");

    // Copy only unique from sorted
    std::unique_copy(b.begin(), b.end(), to_it_begin);

    // return error state
    return !from_file.eof() && !to_file;
}

} // namespace cpp

void show_stream_iterators()
{
    cpp::show_out_iter();
    cpp::show_istr_iter();
    cpp::show_iterators();
}

int main()
{

    show_standard_streams_out();
    show_standard_streams_input();
    show_string_input();
    show_string_streams();
    show_istream_other();
    show_simple_fstream();
    show_formatted_streams();
    show_user_defined_input();
    show_stream_iterators();

    return 0;
}
