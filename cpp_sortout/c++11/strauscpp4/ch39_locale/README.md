Questions:
1. Windows UTF-8 locale name (english_us.65001?)
2. codecvt and encodings at all

3. Money facets do not out CCY symbols
4. try money facet my_money_io
5. messages: try on POSIX

http://www.drdobbs.com/the-standard-librarian-defining-a-facet/184403785
https://habrahabr.ru/post/107679/
https://habrahabr.ru/post/104417/
http://www.cyberforum.ru/cpp-beginners/thread955355.html
https://en.wikipedia.org/wiki/Byte_order_mark

New C++14 features:

Advices:
* The notion of a locale is extensible so that a programmer can add new facets to a locale representing locale-specific 
  entities not directly supported by the standard library, such as postal codes (zip codes) and phone numbers
* A locale encapsulates cultural conventions, such as the appearance of a date. Such con- ventions vary in many subtle and unsystematic ways
  These conventions have nothing to do with programming languages, so a programming language cannot standardize them
* The concept of a locale must be extensible, because it is not possible to enumerate every cultural convention that is important to every C++ user
* A locale is composed of facets that control individual aspects, such as the character used for punctuation in the output of a floating-point value 
  (decimal_point()) and the format used to read a monetary value (moneypunct)
  A facet is an object of a class derived from class locale::facet. We can think of a locale as a container of facets
* If a locale of a given name or a facet referred to doesn’t exist, the locale operation naming it throws a runtime_error
* the resulting locale has a name, that name is implementation-defined
* For a locale without a name, name() returns ""
* There are no facilities for a programmer to specify a name string for a newly created locale
* A programmer can replace facets from existing categories. However, there is no way for a programmer to define a new category
* Each istream and ostream has its own locale. The locale of a stream is by default the global locale at the time of the stream’s creation
  The locale of a stream can be set by the imbue() operation, and we can extract a copy of a stream’s locale using getloc()
* Setting the global locale does not affect existing I/O streams
* A locale is constructed from another locale and from facets. The simplest way of making a locale is to copy an existing one
* The names of locales are not standardized for C++. Instead, a variety of organizations, such as POSIX and Microsoft, 
  maintain their own (differing) standards
* Most operating systems have ways of setting a default locale for a program
  Typically, that is done through environment variables with names such as LC_ALL, LC_COLLATE, and LANG
* It is generally a good idea to avoid embedding locale name strings in the program text
* A new locale is made by taking an existing locale and adding or replacing facets
* The locale uses the type of the Facet* argument to determine the type of the facet at compile time
* There is no way of modifying a locale. Instead, the locale operations provide ways of making new locales from existing ones, for run-time efficiency
* Comparing two strings according to a locale is possibly the most common use of a locale outside I/O
* The facet class is designed to be a base class and has no public functions
* A facet is intended to be managed through pointers stored in locales. 
  A 0 argument to the facet constructor means that locale should delete the facet when the last reference to it goes away
  Conversely, a nonzero constructor argument ensures that locale never deletes the facet
  A nonzero argument is meant for the rare case in which the lifetime of a facet is controlled directly by the programmer 
  rather than indirectly through a locale 
* Each kind of facet interface must have a separate id
* Alternatively, think of use_facet as a kind of explicit type conversion (cast) of a locale to a specific facet
* The standard facets are guaranteed to be available for all locales, so we don’t need to use has_facet for standard facets
* Non-standard facets should have id() only for base classes, implementation for some particular language does not need that
* it is often helpful to produce an 'out-of-range' representation for an out-of-range value
* Because of the generality of the locale/facet mechanism, the possibilities for user-defined facets are unlimited
  Plausible candidates for representation as facets are dates, time zones, phone numbers, social security numbers 
  (personal identification numbers), product codes, temperatures, general (unit,value) pairs, postal codes (zip codes), 
  clothing sizes, and ISBN numbers
* monetary::International can be true or false; true means that a three-character (plus zero terminator) 
  'international' representation of a currency symbol is used
* Each standard facet has a _byname version. An F_byname facet is derived from the facet F. 
  F_byname provides the identical interface to F, except that it adds a constructor taking a string argument naming a locale
* reading a facet from the environment once and then using the copy in main memory repeatedly is usually a good idea
* Only if you have a detailed knowledge of individual facets can you successfully mix and match facets 
  or add new versions of the standard facets. In other words, beyond the simple operations 
  (such as imbue() for iostreams and using collate for sort()), the locale mechanisms are not meant to be directly used

* The standard collate facet provides ways of comparing arrays of characters
* The constructor argument specifies whether a locale or a user is responsible for deleting the facet
  The default (0) means 'let the locale manage'
* The transform() function produces a string that, when compared to another transform()ed string, 
  gives the same result as comparing the strings (???)
* The collate member functions compare [b:e) ranges of C rather than basic_strings or zero-terminated C-style strings
  In particular, a C with the numeric value 0 is treated as an ordinary character rather than as a terminator
* The standard-library string is not locale sensitive. That is, it compares strings according to the rules of the implementation's character set
* It can be more convenient to use collate’s compare() indirectly through a locale’s operator()
* One obvious way of storing facets in an execution environment would be as data in a file
  A less flexible alternative would be to represent a facet as program text and data in a _byname facet

* Numeric output is done by a num_put facet writing into a stream buffer. Conversely, numeric input is done
  by a num_get facet reading from a stream buffer
* The numpunct facet defines the I/O format of built-in types, such as bool, int, and double
* We can define a new punctuation style by deriving from numpunct
* When writing to a stream buffer, an ostream relies on the num_put facet
* The << for a built-in type, such as double, must be implemented by writing directly to a stream buffer
* If we want to read really unusual numeric formats, we have to override do_get()
  For example, we might define a num_get that reads Roman numerals, such as XXI and MM 

* There is no standard money type. Instead, the money facets are meant to be used explicitly for numeric values 
  that the programmer knows to represent monetary amounts
* The facet controlling the presentation of monetary amounts, moneypunct, naturally resembles the facet for controlling plain numbers, numpunct
* The facilities offered by moneypunct are intended primarily for use by implementers of money_put and money_get facets
* Representing a negative number using parentheses is achieved by having negative_sign() return a string containing the two characters ()
* Date and time formats are controlled by time_get<C,In> and time_put<C,Out>. The representation of dates and times used is tm

* Naturally, classification of characters depends on the alphabet used. Consequently, a facet ctype 
  is provided to represent character classification in a locale
* For example, for the ASCII character set, the integer value 125 represents the character }, which is a punctuation character (punct)
  However, in the Danish national character set, 125 represents the vowel <a>, which in a Danish locale must be classified as an alpha
* A call is(m,c) tests whether the character c belongs to the classification m
* A call scan_is(m,b,e) returns a pointer to the first character in [b:e) that is an m
* Translation between unrelated character representations, such as ASCII and EBCDIC, can also be done by using widen()
* A call narrow(ch,def) produces a char value corresponding to the character ch from the C type
  The simplest reasonable transformation is to be used. If no such corresponding char exists, def is returned
* For example, a digit, such as 7, often has several separate representations in a large character set:
widen(narrow(ch,def)) == ch || widen(narrow(ch,def)) == widen(def) // not guaranteed

* The narrow() and widen() functions respect character classifications wherever possible
  For example, if is(alpha,c), then is(alpha,narrow(c,'a')) and is(alpha,widen(c)) wherever alpha is a valid mask for the locale used

* Consequently, the codecvt facet provides a mechanism for converting characters from one representation to another as they are read or written
* The codecvt facet provides conversion between different character sets when a character is moved between a stream buffer and external storage
* A codecvt facet is used by basic_filebuf to read or write characters

* The library provides a simple mechanism for keeping a locale-specific set of strings from which a programmer 
  can compose simple messages. In essence, messages implements a trivial read-only database
* All messages member functions are const, so the catalog data structure (the vector<Set>) is stored outside the facet

* convenience interfaces are provided to simplify notation and minimize mistakes
* The most common use of the ctype facet is to inquire whether a character belongs to a given classification (isspace, isalpha)
* The one-argument versions of these functions use the current C global locale
* Case conversions (toupper/tolower) can be locale sensitive
* Character code conversions can be locale sensitive. Class template wstring_convert performs conversions between a wide string and a byte string
* We can use a code conversion facet to write directly into or to read directly from a stream buffer (class wbuffer_convert)


Book advices: 
