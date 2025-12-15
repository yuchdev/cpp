#include <iostream>

// Hamming weight (bits count)

//types and constants used in the functions below

const uint64_t m1 = 0x5555555555555555; //binary: 0101...
const uint64_t m2 = 0x3333333333333333; //binary: 00110011..
const uint64_t m4 = 0x0f0f0f0f0f0f0f0f; //binary:  4 zeros,  4 ones ...
const uint64_t m8 = 0x00ff00ff00ff00ff; //binary:  8 zeros,  8 ones ...
const uint64_t m16 = 0x0000ffff0000ffff; //binary: 16 zeros, 16 ones ...
const uint64_t m32 = 0x00000000ffffffff; //binary: 32 zeros, 32 ones
const uint64_t hff = 0xffffffffffffffff; //binary: all ones
const uint64_t h01 = 0x0101010101010101; //the sum of 256 to the power of 0,1,2,3...

//This is a naive implementation, shown for comparison,
//and to help in understanding the better functions.
//It uses 24 arithmetic operations (shift, add, and).
uint64_t popcount_1(uint64_t x)
{
    x = (x & m1) + ((x >> 1) & m1); //put count of each  2 bits into those  2 bits 
    x = (x & m2) + ((x >> 2) & m2); //put count of each  4 bits into those  4 bits 
    x = (x & m4) + ((x >> 4) & m4); //put count of each  8 bits into those  8 bits 
    x = (x & m8) + ((x >> 8) & m8); //put count of each 16 bits into those 16 bits 
    x = (x & m16) + ((x >> 16) & m16); //put count of each 32 bits into those 32 bits 
    x = (x & m32) + ((x >> 32) & m32); //put count of each 64 bits into those 64 bits 
    return x;
}

//This uses fewer arithmetic operations than any other known  
//implementation on machines with slow multiplication.
//It uses 17 arithmetic operations.
uint64_t popcount_2(uint64_t x)
{
    x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
    x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits 
    x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits 
    x += x >> 8;  //put count of each 16 bits into their lowest 8 bits
    x += x >> 16;  //put count of each 32 bits into their lowest 8 bits
    x += x >> 32;  //put count of each 64 bits into their lowest 8 bits
    return x & 0x7f;
}

//This uses fewer arithmetic operations than any other known  
//implementation on machines with fast multiplication.
//It uses 12 arithmetic operations, one of which is a multiply.
uint64_t popcount_3(uint64_t x)
{
    x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
    x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits 
    x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits 
    return (x * h01) >> 56;  //returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ... 
}


// The above implementations have the best worst - case behavior of any known algorithm.
// However, when a value is expected to have few nonzero bits, it may instead be more efficient 
// to use algorithms that count these bits one at a time.
// The bitwise and of x with x - 1 differs from x only in zeroing out the least significant nonzero bit : 
// subtracting 1 changes the rightmost string of 0s to 1s, and changes the rightmost 1 to a 0. 
// If x originally had n bits that were 1, then after only n iterations of this operation,
// x will be reduced to zero.The following implementation is based on this principle.


//This is better when most bits in x are 0
//It uses 3 arithmetic operations and one comparison/branch per "1" bit in x.
uint64_t popcount_4(uint64_t x)
{
    int count;
    for (count = 0; x; count++)
        x &= x - 1;
    return count;
}


// If we are allowed greater memory usage, we can calculate the Hamming weight faster than the above methods.
// With unlimited memory, we could simply create a large lookup table of the Hamming weight of every 64 bit integer.
// If we can store a lookup table of the hamming function of every 16 bit integer, 
// we can do the following to compute the Hamming weight of every 32 bit integer.


static uint8_t wordbits[65536] = { /* bitcounts of integers 0 through 65535, inclusive */ };
static uint64_t popcount(uint32_t i)
{
    return (wordbits[i & 0xFFFF] + wordbits[i >> 16]);
}

// TODO: https://stackoverflow.com/questions/109023/how-to-count-the-number-of-set-bits-in-a-32-bit-integer
void show_hamming_weight()
{
    uint64_t bits1 = popcount_1(1);
    std::cout << "Hamming weight(1) = " << bits1 << '\n';

    uint64_t bits2 = popcount_2(2);
    std::cout << "Hamming weight(2) = " << bits2 << '\n';

    uint64_t bits3 = popcount_3(3);
    std::cout << "Hamming weight(3) = " << bits3 << '\n';

    uint64_t bits4 = popcount_4(8);
    std::cout << "Hamming weight(8) = " << bits4 << '\n';
}

int main()
{
    // TODO: insert any function call
    return 0;
}
