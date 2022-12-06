### Content 
Standard containers: an overview of the element requirements
Vectors, lists, deques
Stacks, queues, priority queues
Associative containers: map, multimap, set, multiset
Almost containers: strings, valarray, bitset
Hash containers

### Containers

* The container suitable for most tasks is vector
* Always estimate the complexity of the O() required operation
* You can sort and search with multiple criteria
* When inserting-deleting, prefer to work with the end of container in order to ease compatibility
* If items must be ordered, choose map/set
* If search is important, choose unordered containers
* When no "less than" operation can be defined for items, choose unordered containers
* Use find() to check whether the key is in the map
* When the key is the only value, use set
* Use equial_range to find the range of elements in multimap/multeset
* Use bitset for bit operations, bit to string conversion
* Use predicate instead of function pointer
* Use built-in methods for list and map instead of corresponding algorithms (`list::sort`, `map::find`)
* The complexity of the list::splice operation depends on the complexity of list::size
* When inserting into the map, you can specify the element after which insert is performed
* With the splice function iterators point to the same list and be valid
* You cannot merge unsorted lists with list::merge
* The search/delete/unique list functions take predicates
* deque combines the properties of a vector and a list
* Constaineradapters are stack and queue
* The priority_queue is based on heap data structure
* The priority_queue adapter can accept a comparison predicate
* It is handy to define aliases for map and other types with wordy declarations
* An associative array requires that a "less than" operation for a key
* Insertion into map returns pair<>, which contains an iterator on the inserted string
  and a bool flag indicating successful insertion
* Iteration over the map happens by sorting order
* When the map key by `operator[]` is not found, a default value is created
* Elements from map can be removed one by one or by ranges
* Most containers offer strong exception guarantee, except associative ones on range insertion, and sorting lists
* Copy, swap, predicates should not throw 
* You can try to choose the best hash function (CRC32, SHA, FastHash, XOR)
