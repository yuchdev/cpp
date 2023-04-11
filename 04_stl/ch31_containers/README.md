## New C++14 features
* initializer_list constructors for containers (31.3.2)
* Move semantics for containers (31.3.2)
* A singly-linked list: forward_list (31.4.2)
* Hash containers: unordered_map, unordered_multimap, unordered_set, and unordered_multiset (31.4.3)
* emplace() operations for containers (31.3.6)

## STL Containers
* The STL containers (the sequence and associative containers) are all resource handles with copy and move operations
* A is defaulted to `std::allocator<T>` which uses `operator new()` and `operator delete()` when it needs to acquire or release memory
* When inserting and erasing elements of a vector, elements may be moved
* An empty `forward_list` takes up only one word. There are surprisingly many uses for lists where most are empty (and the rest are very short)
* Unordered containers are implemented as hash tables with linked overflow. The default hash function type, H, for a type K is `std::hash<K>`. The default for the equality function type, E, for a type K is `std::equal_to<K>`
* `basic_string` is typically optimized not to require free store for short strings (SSO)
* To be an element of a container, an object must be of a type that allows the container implementation to copy or move it, and to swap elements
* Some violations of the rules for standard containers can be detected by a compiler, but others cannot and might then cause unexpected behavior. For example, an assignment operation that throws an exception might leave a partially copied element behind
* The ordering criterion must define a strict weak ordering. Informally, this means that both less-than and equality (if defined) must be transitive
* The library doesn't check for equality very often, in about 50% of the cases only a single call of `cmp()` is needed, and often the compiler can optimize away the double check
* Operations that are provided for only some of the containers. In particular:
	* A `multi*` associative container or a set does not provide `operator[]()` or `at()`
	* A `forward_list` does not provide `insert()`, `erase()`,  `emplace()`, back()`, `push_back()`, `pop_back()`, or `emplace_back() instead, it provides the `*_after` operations.
	* A `forward_list` does not provide `reverse_iterator`, `const_reverse_iterator`, `rbegin()`, `rend()`, or `size()`
	* A `unordered_*` associative container does not provide operators <, <=, >, or >=
* Use `()` for size initializers and `{}` for every other kind of iterator
* `reserve()` is a way of increasing the predictability of performance and for avoiding invalidation of iterators
* When we don't need to modify elements, `cbegin()` and `cend()` are appropriate
* For most containers and most implementations, using `begin()` and `end()` repeatedly is not a performance problem
* Some implementations - especially debug versions - always do range checking
* Note that `pop_back()` does not return a value. Had it done so, a copy constructor throwing an exception could seriously complicate the implementation
* The `push_back()` is a perennial favorite for growing a container without preallocation or chance of overflow, but `emplace_back()` can be used similarly
* For containers with contiguous allocation, such as `vector` and `deque`, inserting and erasing an element can cause elements to be moved. An iterator pointing to a moved element becomes invalid. An element is moved if its position is after the insertion/deletion point or if all elements are moved because the new size exceeds the previous capacity
* The `emplace()` operation is used when it is notationally awkward or potentially inefficient to first create an object and then copy (or move) it into a container
* The `forward_list` does not provide operations, such as `insert()`, that operate before an element identified by an iterator
* `forward_iterator` provides operations, such as `insert_after()`, that operate after an element identified by an iterator
* Similarly, unordered containers use `emplace_hint()` to provide a hint rather than 'plain' `emplace()`
* Compared to vector, subscripting could be painfully slow, so subscripting is not provided for lists. If necessary, use `advance()` and similar operations to navigate lists
* For compactness, `forward_list` doesn't even provide a `size()`, an empty `forward_list` takes up just one word of memory. If you need to know the number of elements of a forward_list, just count them
* A `splice()` operation does not copy element values and does not invalidate iterators to elements
* These `list` operations are all stable; that is, they preserve the relative order of elements that have equivalent values
* The `insert(make_pair())` notation is rather verbose. Instead, we could `use emplace()`
* By default, an `unordered_map<X>` uses `hash<X>` for hashing and `equal_to<X>` to compare keys
* For common types, such as string, standard `hash<X>` specializations are provided, so the user need not provide them
* Be careful when constructing an `unordered_map` with one or two arguments. There are many possible combinations of types, and mistakes can lead to strange error messages
* Often, the easiest way of writing a hash function is to use the standard-library hash functions provided as specializations of hash
* Combining hash values using exclusive OR (^) preserves their distributions over the set of values of type `size_t`
* If we wanted to avoid separating the definitions of hf and eq from their point of use, we could try lambdas (named or embedded)
* Finally, we may prefer to define the meaning of hash and equality once for all unordered containers of user-type by specializing the standard-library `hash` and `equal_to` templates 
* The default hash and hashes obtained from it by using exclusive-or are often pretty good. Don't rush to use homemade hash functions without experimentation
* Significant parts of the implementation of an unordered container are made visible to the programmer. Keys with the same hash value are said to be 'in the same bucket'. A programmer can examine and set the size of the hash table (known as 'the number of buckets')
* Note that setting the `max_load_factor`, calling `rehash()`, or calling `reserve()` can be very expensive operations (worst case `O(n*n)`) because they can - and in realistic scenarios typically do - cause rehashing of all elements
* You need to experiment to find a suitable load factor for a given set of elements and a particular hash function, but 70% (0.7) is often a good choice
* One use for the bucket interface is to allow experimentation with hash functions: a poor hash function will lead to large `bucket_count()` for some key values. That is, it will lead to many keys being mapped to the same hash value
* A `priority_queue` is a queue in which each element is given a priority that controls the order in which the elements get to be the `top()`
* Keeping elements in order isn't free, but it needn't be expensive either. One useful way of implementing a `priority_queue`  is to use a tree structure to keep track of the relative positions of elements. This gives an `O(log(n))` cost of both `push()` and `pop()`. A `priority_queue` is almost certainly implemented using a heap
