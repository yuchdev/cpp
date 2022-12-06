### Memory Pool

Production implementation of memory pool
http://www.pjsip.org/pjlib/docs/html/files.htm

Simple memory pool examples
http://www.codeproject.com/Articles/27487/Why-to-use-memory-pool-and-how-to-implement-it
http://www.codeproject.com/Articles/15527/C-Memory-Pool
http://stackoverflow.com/questions/16378306/c11-memory-pool-design-pattern

Facebook jemalloc
https://www.facebook.com/notes/facebook-engineering/scalable-memory-allocation-using-jemalloc/480222803919

Windows Low fragmentation heap
http://msdn.microsoft.com/en-us/library/aa366750.aspx

As for fragmentation: it still depends. Unfortunately I can't find the link now, 
but I remember a blog post from somebody at Microsoft who had worked on a C++ server 
application that suffered from memory fragmentation. The team solved the problem by 
allocating memory from two regions. Memory for all requests would come from region A 
until it was full (requests would free memory as normal). 
When region A was full, all memory would be allocated from region B. 
By the time region B was full, region A was completely empty again. 
This solved their fragmentation problem.
