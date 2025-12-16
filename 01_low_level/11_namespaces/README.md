## Namespaces

* Use namespaces to express logical structure; 14.3.1
* The inline specifies that the nested namespace is the default meaning of sub-namespace. This could be useful for the namespace versionig
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
