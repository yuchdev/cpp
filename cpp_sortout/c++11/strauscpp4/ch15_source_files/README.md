Questions:
* how many at_quick_exit() functions could be?

New C++14 features:
* Abandoning a process: quick_exit(15.4.3)

Advices:
* A name that can be used in translation units different from the one in which it was defined is said to have external linkage
* A name that can be referred to only in the translation unit in which it is defined is said to have internal linkage
* By default, const objects (7.5), constexpr objects (10.4), type aliases (6.5), and anything declared static (6.3.4) 
  in a namespace scope have internal linkage
* An unnamed namespace can be used to make names local to a compilation unit
* The effect of an unnamed namespace is very similar to that of internal linkage
* spaces are significant within the < > or " " of an include directive
* The absence of a .h suffix does not imply anything about how the header is stored
  A header such as <map> is usually stored as a text file called map.h in some standard directory
* For each C standard-library header <X.h>, there is a corresponding standard C++ header <cX>
* We can specify a linkage convention to be used in an extern declaration
* This construct, commonly called a linkage block, can be used to enclose a complete C header to make a header suitable for C++ use
extern "C" {
// ...
}

* This technique is commonly used to produce a C++ header from a C header
* Alternatively, conditional compilation (12.6.1) can be used to create a common C and C++ header:
#ifdef __cplusplus
extern "C" {...
* A name with C linkage can be declared in a namespace (std::printf)
* A variable defined outside any function (that is, global, namespace, and class static variables) is initialized before main() is invoked
* Often, a function returning a reference to static is a good alternative to a global variable
* The initialization of a local static is thread-safe (42.3.3)
* The quick_exit() function is like exit() except that it does not invoke any destructors
* Register functions to be invoked by quick_exit() using at_quick_exit()
* Exit possibilities
	* std::_Exit (abort): doesn't execute static destructors or flush critical IO and soes not call handler
	* std::exit: executes static destructors and flushes critical IO and call handler if exists
	* std::quick_exit: doesn't execute static destructors, but does flush critical IO and call handler if exists

Book advices: 
* Use header files to represent interfaces and to emphasize logical structure; 15.1, 15.3.2.
* #include a header in the source file that implements its functions; 15.3.1.
* Don’t define global entities with the same name and similar-but-different meanings in different translation units; 15.2.
* Avoid non-inline function definitions in headers; 15.2.2.
* Use #include only at global scope and in namespaces; 15.2.2.
* #include only complete declarations; 15.2.2.
* Use include guards; 15.3.3.Section 15.5 Advice 445
* #include C headers in namespaces to avoid global names; 14.4.9, 15.2.4.
* Make headers self-contained; 15.2.3.
* Distinguish between users’ interfaces and implementers’ interfaces; 15.3.2.
* Distinguish between average users’ interfaces and expert users’ interfaces; 15.3.2.
* Avoid nonlocal objects that require run-time initialization in code intended for use as part of non-C++ programs; 15.4.1.