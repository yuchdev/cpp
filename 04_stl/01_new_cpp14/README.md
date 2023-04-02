Questions:

### STL Basics

* List examples could be written identically using vector and (if understand machine architecture) perform better with a small vector than with a small list
* The cost of a map lookup is O(log(n))
* Hash containers are referred to as 'unordered' because they don't require an ordering function
* In many standard-library search algorithms, find returns end() to indicate 'not found'
* Returning a vector is efficient because of vector provides move semantics

### New C++14 features:

* A singly-linked list: forward_list (4.4.5, 31.4.2)
* Hash containers: unordered_map, unordered_multimap, unordered_set, and unordered_multiset (4.4.5, 31.4.3)
 