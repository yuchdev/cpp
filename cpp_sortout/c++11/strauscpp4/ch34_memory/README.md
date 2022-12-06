Questions:
// TODO: piecewise construct

Garbage collection

New C++14 features:
* Resource management pointers: unique_ptr, shared_ptr, and weak_ptr (34.3)
* tuples (34.2.4.2)
* A fixed-sized contiguous sequence container: array (34.2.1)
* Scoped allocators(34.4.4)
* Garbage collection ABI (34.5)

Advices:
* No management information (e.g. size) is stored in an array. This implies that moving an array is no more efficient than copying it 
* To save us from a special case, the number of elements can be zero
* array’s size() is a constexpr function
* The declaration of array<T,N>::swap() basically says that if a swap() of T can throw, then so can a swap() of an array<T,N>
* The standard library provides two ways of grouping values of arbitrary types into a single object: pair and tuple
* The piecewise_construct is the name of an object of type piecewise_construct_t used to distinguish between constructing 
  a pair with members of tuple types and constructing a pair using tuples as argument lists for its first and second
  (see example)
* The number of tuple elements is zero or positive
* Note that the general tuple constructor is explicit
* 

Book advices: 