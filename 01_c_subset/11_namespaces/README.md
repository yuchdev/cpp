## Namespaces

* Use namespaces to express logical structure; 14.3.1
* The inline specifies that the nested namespace is the default meaning of sub-namespace. This could be useful for the namespace versionig

## Namespaces

* A name that can be used in translation units different from the one in which it was defined is said to have external linkage
* A name that can be referred to only in the translation unit in which it is defined is said to have internal linkage
* By default, const objects (7.5), constexpr objects (10.4), type aliases (6.5), and anything declared static (6.3.4) 
  in a namespace scope have internal linkage
* An unnamed namespace can be used to make names local to a compilation unit
* The effect of an unnamed namespace is very similar to that of internal linkage
* spaces are significant within the < > or " " of an include directive
* The absence of a .h suffix does not imply anything about how the header is stored
  A header such as `<map>` is usually stored as a text file called map.h in some standard directory
* For each C standard-library header `<X.h>`, there is a corresponding standard C++ header `<cX>`
* We can specify a linkage convention to be used in an extern declaration
* This construct, commonly called a linkage block, can be used to enclose a complete C header to make a header suitable for C++ use
extern "C" {
// ...
}

* This technique is commonly used to produce a C++ header from a C header
* Alternatively, conditional compilation (12.6.1) can be used to create a common C and C++ header:
```
#ifdef __cplusplus
extern "C" {...}
```

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
