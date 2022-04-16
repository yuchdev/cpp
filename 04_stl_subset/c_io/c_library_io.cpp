#include <iostream>
#include <cctype>
#include <cstdio>
#include "streams.h"

using namespace std;

void char_functions(){
	// Character classification functions
    // return non-zero if TRUE, 0 if FALSE
    
    cout << "isalnum(A) " << isalnum('A') << endl;
    cout << "isalnum(1) " << isalnum('1') << endl;
    cout << "isalpha(A) " << isalpha('A') << endl;
    cout << "isdigit(1) " << isdigit('1') << endl;
    //cout << "isblank(' ') " << isblank(' ') << endl;
    //cout << "isblank('\t') " << isblank('\t') << endl;
    // all except blank
    cout << "isgraph(a) " << isgraph('a') << endl;
    cout << "isgraph(\t) " << isgraph('\t') << endl;

    // non-printable chars
    cout << "iscntrl(0x00) " << iscntrl(0x0) << endl;
    // printable chars
    cout << "isprint(a) " << isprint('a') << endl;

    cout << "islower(a) " << islower('a') << endl;
    cout << "isupper(A) " << isupper('A') << endl;

    cout << "ispunct(,) " << ispunct(',') << endl;
    cout << "isspace(' ') " << isspace(' ') << endl;
    
    // hex digits
    cout << "isxdigit('f') " << isxdigit('f') << endl;
}

void show_c_print(){
    printf("Signed decimal integer: %d or %i \n", 392, 392);
    printf("Unsigned decimal integer: %u \n", 7235);
    printf("Unsigned octal: %o \n", 610);
    printf("Unsigned hexadecimal integer: %x \n", 0x7f);
    printf("Unsigned hexadecimal integer (uppercase): %X \n", 0x7f);

    printf("Decimal floating point, lowercase: %f \n", 392.65);
    printf("Decimal floating point, uppercase: %F \n", 0.001); // does not printed

    printf("Scientific notation (mantissa/exponent), lowercase: %e \n", 3.9265e+2);
    printf("Scientific notation (mantissa/exponent), uppercase: %E \n", 3.9265e+2);

    printf("Use the better representation: e or f: %g \n", 3.9265e+2);
    printf("Use the better representation: E or F, uppercase: %G \n", 3.9265e+2);

    printf("Hexadecimal floating point, lowercase: %a \n", 3.9290);
    printf("Hexadecimal floating point, uppercase: %A \n", 3.9290);

    printf("Character: %c \n", 'a');
    printf("String of characters: %s \n", "sample");

    int a = 0;
    printf("Pointer address: %p \n", &a);

    // The corresponding argument must be a pointer to a signed int.
    // The number of characters written so far is stored in the pointed location
    // In modern libraries disabled for security reasons
    //printf("Nothing printed: %n", &a);

    printf("A %% followed by another %% character will write a single %% to the stream");
}

void show_c_print_formatting(){

    printf (" - Left-justify within the given field width (see width sub-specifier): %-d \n", 10);

    printf ("+ Forces to precede the result with a plus or minus sign (+ or -): %+i \n", 10);
    
    printf ("# Used with o, x or X specifiers the value is preceeded with 0,");
    printf ("0x or 0X respectively for values different than zero: %#x, %#X \n", 10, 10);
    printf ("Used with a, A, e, E, f, F, g or G it forces the written output to contain\n");
    printf ("a decimal point even if no more digits follow: %#a, %#A, %#e, %#E, %#f, %#F, %#g or %#G\n", 
        3.14, 3.14, 3.14, 3.14, 3.14, 3.14, 3.14, 3.14 );
    
    printf ("(number) Minimum number of characters to be printed.\n");
    printf ("If the value to be printed is shorter than this number, the result is padded with blank spaces: %10d \n", 10);
    printf ("The value is not truncated even if the result is larger.\n");

    printf ("0 Left-pads the number with zeroes (0) instead of spaces when padding is specified: %010d \n", 10);
    
    printf ("* The width is not specified in the format string, but as an additional integer value argument: %*d\n", 5, 10);
    
    printf ("For integer specifiers (d, i, o, u, x, X)\n");
    printf ("precision specifies the minimum number of digits to be written: %.4d, %.4i, %.4o, %.4u, %.4x, %.4X \n", 
        1000, 10000, 100000, 1000, 10000, 100000);
    
    printf ("For a, A, e, E, f and F specifiers\n");
    printf ("Number of digits to be printed after the decimal point (by default 6): %.4a, %.4A, %.4e, %.4E, %.4f and %.4F\n",
        0.001, 0.0001, 0.00001, 0.001, 0.0001, 0.00001);
     
    printf ("For g and G specifiers: This is the maximum number of significant digits to be printed: %.4g, %.4G\n",
        0.001, 0.0001);

    printf ("For s: this is the maximum number of characters to be printed: %.5s\n", "qwerty");

    // More examples
    printf ("More examples:\n");

    printf ("Characters: %c %c \n", 'a', 65);

    printf ("Decimals: %d %ld\n", 1977, 650000L);

    printf ("Preceding with blanks: %10d \n", 1977);

    printf ("Preceding with zeros: %010d \n", 1977);

    printf ("Some different radices: %d %x %o %#x %#o \n", 100, 100, 100, 100, 100);

    printf ("floats: %4.2f %+.0e %E \n", 3.1416, 3.1416, 3.1416);

    printf ("Width trick: %*d \n", 5, 10);

    printf ("%s \n", "A string");

    // Size specifiers
    printf ("Size specifiers:\n");
    printf ("Byte: %hhd \n", (char)10);
    printf ("Short: %hd \n", (short)10);
    printf ("Long: %ld \n", 10l);
    printf ("Long long: %lld \n", 10ll);
    printf ("Int: %jd \n", 10ll);
    printf ("size_t: %zd \n", 10);
    printf ("ptrdiff_t: %td \n", 10);
    printf ("Long double: %Lf \n", 1000.5l);
}

void show_c_io(){
    char_functions();
    show_c_print();
    show_c_print_formatting();
}
