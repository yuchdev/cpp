### Content

Output streams.
Built-in and user-defined streams.
Character input.
Stream state, sentinels, exception, stream linking.
Formatting, format state, number output, alignment, manipulators.
File streams. Buffering. Localization.

### Streams

* `ostream` could be considered as a convertion object->symbols. `istream` does vise versa
* streams use `char_traits` internally
* `basic_ios` controls formatting, locales and buffer management
* `istream` and `ostream` could not be copied
* `ostream` could be "virtualized" passing to the `operator<<` a reference to base class
* input stream could be converted to `void*`, which is used for end-of-input sign, e.g. `while(is >> buf)`
* `<iosfwd>` is used for the forward declaration if all IO templates
* error streams are `cerr`(non-buffered), and `clog` (buffered)
* all integer types output as they are
* floating precision = 6 by default
* `bool` output is 1/0
* pointers output as hexedecimals
* flags or manipulators could be used to change formatting
* `cout::put`/`cout::write` could be used for output as well
* `operator>>`/`operator<<` priority is low
* EOL is end-of-input
* whitespace is delimeter by defaut (which switched off using `noskipws()`)
* `put`/`write` opposite is `get`/`read`
* after `cin.get()` call `cin.ignore()` should be called to skip `\n`
* `cout`/`cin` could be 'tied' using `tie()` method to be used in pair
* in case of custom delimeters (e.g. `cin.get(s1, 50, '.')`) `.` should be ignored as well
* `std::getline()` ignores `\n` automatically
* stream error flags: `eof`, `fail` (recoverable error), `bad` (fatal error)
* after fail stream could be recovered using `cin.clear(/*ios_base::goodbit*/)`
* `cin.rdstate()` reads flags
* `cin.exceptions(ios::badbit|ios::failbit)` sets thowing exceptions insted of flags setting
* every buffered stream have his own buffer. It could be accessed by `rdbuf()`
* buffer could be used for the stream redirection
* with non-latin wide symbols locale should be set using `imbue()` method
* file streams are RAII wrappers under files
* text filestreams are formatted, binary don't
* filestreams are buffered
* while random write access `ios::in` must be set
* `stringstream` allocates `std::string` using `new()` function
* setting flags:
  * `flags()`
  * `setf()` is OR analog
  * `unsetf()` is NOR analog
* stream format could be copied by `copyfmt()` method
* `precision()` sets count of numbers ofter the point. Works indefenitely
* `fill()` sets fill symbol. Works indefenitely
* `width()` sets input fiels width. Works for the single input operation.
* user manipulator without params could be easily created
* user manipulator with params called effector is a class with a single-param constructor and `ostream& operator<< ()` overload
  Could be used for complicated formatting (dates, currencies, etc)
