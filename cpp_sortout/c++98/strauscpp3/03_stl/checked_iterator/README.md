Iterators
Plain, inserters, reverse iterators, stream iterators.
Memory allocators, standard and user-defined.

### Iterators

* Use inserter to insert into containers using algorithms 
 (except when the output sequence is the same as the input sequence)
* The algorithms also implicitly pass the "iterator trait" object
* To design algorithms that use different types of iterators, use iterator_traits
 (for storing types associated with the iterator: diff, ref, etc.)
* Prefer form ++it with iterators


### Allocators

* Memory operations can be optimized with your allocator
* STL Allocators have a standard interface that is supported by the Standard
* The allocator can allocate memory not only for container elements,
  but also for service structures (`rebind` structure)
* You can make the allocator store (and provide)
  some service data (where objects are stored, container control, etc.)
* `construct`/`destroy` works with the allocated memory and calls constructors in it
* `allocate`/`deallocate` initialize "raw" memory
* Memory release by default has no check for null
* The specialization for void must necessarily be given for the allocator
* Allocator template `operator==` reports that memory can be allocated by one allocator and destroyed by another
* STL includes functions for low-level work with 
  user containers and algorithms (`uninitialized_*` family)
* Use ANSI C functions for raw memory management only for compatibility or on low-level
