Questions:

New C++14 features:

* A singly-linked list: forward_list (4.4.5, 31.4.2)
* Hash containers: unordered_map, unordered_multimap, unordered_set, and unordered_multiset (4.4.5, 31.4.3)

Advices:

* List examples could be written identically using vector and (if understand machine architecture) 
  perform better with a small vector than with a small list
* The cost of a map lookup is O(log(n))
* Hash containers are referred to as 'unordered' because they don't require an ordering function
* In many standard-library search algorithms, find returns end() to indicate 'not found'
* Returning a vector is efficient because of vector provides move semantics


Book advices:

* Don't reinvent the wheel
* When you have a choice, prefer the standard library over other libraries
* Do not think that the standard library is ideal for everything
* Remember to #include the headers for the facilities you use
* Remember that standard-library facilities are defined in namespace std
* Prefer strings over C-style strings (a char*)
* iostreams are type sensitive, type-safe, and extensible
* Prefer vector<T>, map<K,T>,and unordered_map<K,T> overT[]
* Know your standard containers and their tradeoffs
* Use vector as your default container
* Prefer compact data structures
* If in doubt, use a range-checked vector (such as Vec)
* Use push_back() or back_inserter() to add elements to a container
* Use push_back() on a vector rather than realloc() on an array
* Catch common exceptions in main()
* Know your standard algorithms and prefer them over handwritten loops
 