Algorithms, functors, adapters and binders

* Algorithms are templates that take sequences in the form of iterators
* There is no point in including all algorithms or more complex algorithms in the program
* A search failure is usually reported by the end-of-sequence iterator.
* Algorithms do not perform range checks on input and output
* Most algorithms allow its user to define a comparison or transformation operation
* Heap operations keep the sequence in a state close to sorted
* Most algorithms are declared in `<algorithm>`, a few are declared in `<numeric>`
* Prefer algorithms to loops
* Use `for_each` and `transform` only when there is no more specific algorithm for the task
* for_each() is non-modifying algorithm
* Regularly review your knowledge of algorithms
* Modifying algorithms do not change the number of elements in the container
* Use ANSI C algorithms only for compatibility
* Modifying algorithms do not necessarily write changes in the input sequence
* The use of inserters does not allow to go beyond the container range
  allows you to conveniently write to the end of sequence
* Input and output sequences may overlap
* Algorithms can return different types, which ones are described in iterator_traits
