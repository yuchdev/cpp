### New handler

* It is possible to replace the standard `new_handler`, which is called on the situation of lack of memory.
  The standard `set_new_handler()` is called until memory is released or the program is finished
* In theory, it is possible to implement its own `set_new_handler()` for each specific class.
  You can create a mixture base class to inherit the `operator new()` and `new_handler`,
  or parameterize the themplate class with it.
  Using templates and CRTP to make sure that each base class is unique

### Overloading `new` and `delete`

* Know the difefrence between the `new` operation - it allocates memory and calls the constructor. It cannot be redefined.
  and `operator new()` function - it changes the way of memory allocation, and you can redefine it.
  Similarly with the `delete` operator and `operator delete()` function
* `operator new()` and `operator delete()` are usually overloaded to increase efficiency.
  This is especially true for applications allocating many small objects
* overriding `operator new()` for the class guarantees that it will be called for its instances
* `operator new()` is inherited by derived classes, so `operator new()` in a base class should be designed for derived classes as well
* Don't forget to implement the sibling `operator new[]()` for arrays
* `operator new()` may have any number of parameters
* If any form of `operator new()` other than normal is overloaded, e.g. "placement new", or with pointer to a `new_handler`,
  normal `operator new()` must be overloaded. Otherwise overloaded operators will hide the global `operator new()`
* The placement `operator new()` and placement `operator delete()` should appear only in pairs
  In other words, for each placement function allocating memory for an object `void operator new(size_t, P2, P3, ..., Pn)` 
  there must be a matching function releasing memory `void *operator delete(void *, P2, P3, ..., Pn)`
* Even doing so, the placement `operator delete()` is only being called to delete a partially constructed object 
  (in the situation of throwing exception from the constructor).
  A fully constructed object is deleted using the `delete` operation, and these expressions call the non-placement `operator delete()`
* If class-level `operator delete()` is a template function, it must have the return type of `void`, and the first argument type `void*`, 
  and it must have two or more parameters. 
  In other words, only placement forms can be templates.
* Without virtual destructors, `operator delete()` may not work properly
* `operator delete` is unique in being a non-member or static member function that is dynamically dispatched.
  A type with a virtual destructor performs the call to its own delete from the most derived destructor

#### Sort it out later
* `delete[]` usually includes object counter by offset 0
* `new`/`delete` pointer should be compared to zero
* `new_handler` function is a function which is called by the default allocation functions when they fail to allocate
* every class may have its own new handler, set by `set_new_handler()`
* Mixture pattern could be used to apply some particular allocation algorithm towards the class


http://en.cppreference.com/w/cpp/memory/new/operator_new
http://en.cppreference.com/w/cpp/language/delete
http://en.cppreference.com/w/cpp/memory/new/operator_delete

How to allow placing of the object only in heap?
Make c-tors private, and open "virtual c-tor"

How to disallow placing of the object in heap?
Make operator new/new[] private

How to determine whether the object placed in the queue?
Stack grows top-to-bottom, heap - bottom-to-top

operator new/delete implementation notes
Normally on the compiler-level every generated class has "heap flag"
if it is == true, it means that objects is created on the heap, 
and operator delete() would be called after destructor
