### Memory

Memory management.
According to the Standard, there are 3 types of memory - static, dynamic and automatic.
The basic unit of memory is the byte. The number of bits depends on the implementation.
It is necessary that the basic types char and unsigned char are fit in the byte.
Attention - the Standard describes the work with dynamic memory in many places of the document.

POD/non-POD types.

* According to the Standard, POD-types (Plain Old Data) simply allocate memory for their content.
* POD-types are
  * having only an automatic constructor
  * having no virtual functions
  * if a class has other classes, they are also POD-types
  * if a class inherits from another class, the parent is also a POD-type
* Non-POD classes are all other classes. Memory is allocated for them first, and then the constructor is called.
  If memory cannot be allocated, the constructor is not called.


* There are 3 standard forms of new() - default, placement and nothrow
* The new() operator calls an operator new() function to allocate memory, 
  and then, depending on the type allocated and the syntax used, initializes or calls a constructor
* Destructor of the class could be called explicitly, for example with placement new
* Class size not always equal the sum of all its members, because of the alignment
