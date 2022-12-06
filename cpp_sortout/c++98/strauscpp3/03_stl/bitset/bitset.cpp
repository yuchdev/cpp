#include <bitset>
#include <string>
#include <iostream>
#include <stdexcept>
#include "containers2.h"
using namespace std;

void show_bitset()
{

    // all 0
    bitset<10> b1;
    bitset<16> b2(0xaaaa);
    bitset<32> b3(0xaaaa);

    // does not convert from literal
    bitset<16> b4(string("00100001001"));
    // from position 4
    bitset<16> b5(string("00100001001"), 4);
    // from position 2 to 8
    bitset<16> b6(string("00100001001"), 2, 8);

    try {
        bitset<16> b6(string("ytcivy"));// exception invalid_argument
    }
    catch (const std::invalid_argument& e) {
        cout << e.what() << endl;
    }

    string loong("000000000000111111111111100000000001111111110000111111111111000000");
    try {
        bitset<128> b7(loong);
        long l = b7.to_ulong(); // exception overflow_error
    }
    catch (const std::overflow_error& e) {
        cout << e.what() << endl;
    }

    // as a string
    cout << b2.to_string() << endl;

    // Operations:
    // shifts
    b2 >>= 4;
    b3 <<= 2;

    // Bit setting
    b1.set();	// all 1
    b2.reset();	// all 0

    b1.set(2, 0);	// second 0
    b1.reset(3);	// third 0

    b3.flip();	// invert

    // operator= works only for equal sizes
    // inversion with assignment
    b4 = ~b5;

    // shift with assignment
    b5 = b6 >> 5;

    try {
        //b1[16] = 1;	
                // any reference by index performed with check
        // In DEBUG config not an exception, but a debug assert
    }
    catch (const std::exception& e) {
        cout << e.what() << endl;
    }

    // output
    cout << b2 << endl << b3 << endl;
}
