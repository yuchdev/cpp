### Content

Standard template library.
Base containers. Iterators.
Designing containers.


### Vector

* Prefer algorithms with reverse iterators to explicit end-to-begin loops
* Use base() to extract iterator from the reverse_iterator
* Use constant iterators if you don't want to change elements
* Don't use iterators in a resized vector, they get disabled during memory reallocation
* Use reserve() for better allocation predictability
* Vector has strong exception guarantees
* insert() and push_back() do not make changes in case of throw
* pop_back() and swap() do not throw
