## Simple smart pointers
* It is possible to implement different strategies for transferring smart pointers ownership
* While using the reference counting, either each object must have a reference counter (like the base ObjectCounter), or the reference counter shoud be implemented in external memory
* It is handy to implement a counter strategy class to store the counter (plain, thread-safe, etc.)
* It is handy to use the delete strategy class to determine how to delete (`free`, `delete`, `delete[]`, the approach of Standard Library)
