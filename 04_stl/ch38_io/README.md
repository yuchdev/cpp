## TODOs
* Manip - inset trailing zeros
* ANSI C IO library

## C++ IO Library

* Note that no copy operations are provided: sharing or cloning the fairly complex state of a stream would be difficult to implement and expensive to use. The move operations are intended for use by derived classes and are therefore protected. Moving an `iostream` without moving the state of its defining derived class (e.g., an `fstream`) would lead to errors
* Forward declarations for `stream` types and `stream` objects are provided in `<iosfwd>`
* An `istream` can be connected to an input device (e.g., a keyboard), a file, or a string. Similarly, an `ostream` can be connected to an output device (e.g., a text window or an HTML engine), a file, or a string
* In addition, the string streams override the `basic_ios` protected virtual functions `underflow()`, `pbackfail()`, `overflow()`, `setbuf()`, `seekoff()`, and `seekpos()`
* A file stream does not have copy operations. If you want two names to refer to the same file stream, use a reference or a pointer, or carefully manipulate file streambufs
* If an `fstream` fails to open, the stream is in the `bad()` state
* You can open a file in one of several modes, as specified in `<ios_base>`: `ios_base::app`, `ios_base::ate`, `ios_base::binary`, `ios_base::in`, `ios_base::out`, `ios_base::trunc`
* In each case, the exact effect of opening a file may depend on the operating system, and if an operating system cannot honor a request to open a file in a certain way, the result will be a stream that is in the `bad()` state
* A string stream does not have copy operations. If you want two names to refer to the same string stream, use a reference or a pointer
* It is not possible to directly output a string stream; `str()` must be used, which copies the string
* `iostream` converts to its state to bool for testing
```
if (iss) { 
    // the last operation of iss succeeded
}
```
* Be careful with the termination criteria when using `get()`, `getline()`, and `read()`
* Any operation attempted on a stream that is not in the `good()` state has no effect; it is a no-op
* Use exceptions to handle iostream errors that do not expected to be recovered from. That usually means all `bad()` exceptions
* Unless otherwise stated, an stream operation returns a reference to its stream, so that we can chain operations
* The input operations are not virtual. That is, a user cannot do an `in >> base` where base is a class hierarchy and automatically have the `operator>>()` resolved to an operation on the appropriate derived class
* If you have a choice, use formatted input instead these low-level input functions
* The `basic_ios` class manages the state of a stream
* Note that `ios_base` has no copy or move operations
* The exact meaning of `ios_base::binary` is implementation-dependent. However, the usual meaning is that a character gets mapped to a byte
* Use binary I/O when dealing with objects that are 'just bags of bits' and do not have an obvious and reasonable character string representation. Images and sound/video streams are good examples
* The `seekg()` and `seekp()` operations require a direction: `beg`, `cur`, `end`
* The conversion of an `ios` (including istreams and ostreams) to bool is essential to the usual idiom for reading many values
```
for (X x; cin>>x;) {...
```
* The `tie()` is used to ensure that output from a tied stream appears before an input from the stream to which it is tied
* Class `ios_base` provides a function `xalloc()` to allocate space for such simple state information. The value returned by `xalloc()` identifies a pair of locations that can be accessed by `iword()` and `pword()`
* The `ios::register_callback()` registers a function to be called when its event occurs. Thus, a call of `imbue()`, `copyfmt()`, or `ios_base()` will call a function registered for an `imbue_event`, `copyfmt_event`, or `erase_event`, respectively
* If you have a genuine need for frequent flushing, consider `cerr` and `unitbuf`. Normally, use `\n` for EOL
* When an `ostream_iterator` is constructed with a second (string) argument, that string is output as a terminator after every element value
* Typically, the streambuf stores characters in an array until an overflow forces it to write the characters to their real destination
* The `basic_streambuf` has a host of operations. Many of the public operations simply call a protected virtual function that ensures that a function from a derived class implemented the operation appropriately for the particular kind of buffer
* All constructors are protected because basic_streambuf is designed as a base class
* A `streambuf` has a put area into which `operator<<()` and other output operations write, and a get area from which `operator>>()` and other input operations read
* The `showmanyc()` (show how many characters) function is an odd function intended to allow a user to learn something about the state of a machine's input system
* Don't use `showmanyc()` without a careful reading of your system documentation and conducting a few experiments
* Use `sync_with_stdio(true)` if you want to mix C-style and iostream I/O (`sync_with_stdio(false)` to optimize iostreams)
