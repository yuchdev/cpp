## New handler

* It is possible to replace the standard `new_handler`, which is called on the situation of lack of memory.
  The standard `set_new_handler()` is called until memory is released or the program is finished
* In theory, it is possible to implement its own `set_new_handler()` for each specific class.
  You can create a mixture base class to inherit the `operator new()` and `new_handler`,
  or parameterize the themplate class with it.
  Using templates and CRTP to make sure that each base class is unique

## Overloading `new` and `delete`

* Know the difefrence between the `new` operation - it allocates memory and calls the constructor. It cannot be redefined.
  and `operator new()` function - it changes the way of memory allocation, and you can redefine it.
  Similarly with the `delete` operator and `operator delete()` function
* `operator new()` and `operator delete()` are usually overloaded to increase efficiency.
  This is especially true for applications allocating many small objects
* overriding `operator new()` for the class guarantees that it will be called for its instances
* `operator new()` is inherited by derived classes, so `operator new()` in a base class should be designed for derived classes as well
* Don't forget to implement the sibling `operator new[]()` for arrays
* `operator new()` may have any number of parameters
* If any form of `operator new()` other than normal is overloaded, e.g. "placement new", or with pointer to a `new_handler`,
  normal `operator new()` must be overloaded. Otherwise overloaded operators will hide the global `operator new()`
* The placement `operator new()` and placement `operator delete()` should appear only in pairs
  In other words, for each placement function allocating memory for an object `void operator new(size_t, P2, P3, ..., Pn)` 
  there must be a matching function releasing memory `void *operator delete(void *, P2, P3, ..., Pn)`
* Even doing so, the placement `operator delete()` is only being called to delete a partially constructed object 
  (in the situation of throwing exception from the constructor).
  A fully constructed object is deleted using the `delete` operation, and these expressions call the non-placement `operator delete()`
* If class-level `operator delete()` is a template function, it must have the return type of `void`, and the first argument type `void*`, 
  and it must have two or more parameters. 
  In other words, only placement forms can be templates.
* Without virtual destructors, `operator delete()` may not work properly
* `operator delete` is unique in being a non-member or static member function that is dynamically dispatched.
  A type with a virtual destructor performs the call to its own delete from the most derived destructor

### Sort it out later

* `delete[]` usually includes object counter by offset 0
* `new`/`delete` pointer should be compared to zero
* `new_handler` function is a function which is called by the default allocation functions when they fail to allocate
* every class may have its own new handler, set by `set_new_handler()`
* Mixture pattern could be used to apply some particular allocation algorithm towards the class
* Standard LLibrary has function pointers callback: `new_handler`

http://en.cppreference.com/w/cpp/memory/new/operator_new
http://en.cppreference.com/w/cpp/language/delete
http://en.cppreference.com/w/cpp/memory/new/operator_delete

How to allow placing of the object only in heap?
Make c-tors private, and open "virtual c-tor"

How to disallow placing of the object in heap?
Make operator new/new[] private

How to determine whether the object placed in the queue?
Stack grows top-to-bottom, heap - bottom-to-top

operator new/delete implementation notes
Normally on the compiler-level every generated class has "heap flag"
if it is == true, it means that objects is created on the heap, 
and operator delete() would be called after destructor

## Memory Pool

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

## Memory

Memory management.
According to the Standard, there are 3 types of memory - static, dynamic and automatic.
The basic unit of memory is the byte. The number of bits depends on the implementation.
It is necessary that the basic types char and unsigned char are fit in the byte.
Attention - the Standard describes the work with dynamic memory in many places of the document.

POD/non-POD types.

* According to the Standard, POD-types (Plain Old Data) simply allocate memory for their content.
* POD-types are
  * having only an automatic constructor
  * having no virtual functions
  * if a class has other classes, they are also POD-types
  * if a class inherits from another class, the parent is also a POD-type
* Non-POD classes are all other classes. Memory is allocated for them first, and then the constructor is called.
  If memory cannot be allocated, the constructor is not called.


* There are 3 standard forms of new() - default, placement and nothrow
* The new() operator calls an operator new() function to allocate memory, 
  and then, depending on the type allocated and the syntax used, initializes or calls a constructor
* Destructor of the class could be called explicitly, for example with placement new
* Class size not always equal the sum of all its members, because of the alignment

## Alignment

### Q: What subtle problem may contain following cross-platform cross-architecture C++ example?

```
std::uint32_t decode(const void * p)
{
    const auto pv = static_cast<const std::uint32_t *>(p);
    std::uint32_t v = 0;
    std: :memcpy(&v, p, sizeof(v));
    boost::endian: :little_to_native_inplace(v);
    return v;
}
```

A nasty one: **alignment** (and a related "it might work on x86, crash on ARM/SPARC" trap).

### A: What's subtly wrong here

* You do this cast:

```cpp
const auto pv = static_cast<const std::uint32_t*>(p);
```

Even if you *don't dereference* `pv` in the snippet, this is a giant foot-gun in real code because it invites dereferencing later (or gets "optimized back in" during refactors). On many architectures, a `std::uint32_t*` **must be 4-byte aligned**. If `p` points to packed/network data (e.g., `buf + 1`), then `pv` would be misaligned.

* On **x86/x64**, misaligned loads usually "work" (often slower).
* On **ARM (some modes), SPARC, older MIPS/PowerPC**, misaligned access can raise **SIGBUS** (exactly what your comment hints).

Your code uses `memcpy` (good!) which is the *portable* way to read unaligned bytes. But the presence of the typed pointer `pv` is a subtle cross-platform hazard.

### Additional cross-platform gotchas in this exact snippet

1. **Typos that often slip past review**

   * `std: :memcpy` and `boost::endian: :little_to_native_inplace` have spaces → won't compile. If the "real" code doesn't, ignore this; if it does, that's the immediate issue.

2. **Endianness vs intent**

   * You force `little_to_native_inplace(v)`. That's correct only if the byte stream is defined as *little-endian*. If the encoded data is "network order" (big-endian), you want `big_to_native_inplace`.

3. **Strict aliasing (what you did right)**

   * If you had used `*pv` to read, you'd also risk strict-aliasing / effective type issues in some cases. Using `memcpy` avoids that.

### Safer version (and remove the trap)

```cpp
std::uint32_t decode(const void* p)
{
    std::uint32_t v;
    std::memcpy(&v, p, sizeof(v));        // safe for unaligned
    boost::endian::little_to_native_inplace(v);
    return v;
}
```

If you *really* need a typed view, use `std::byte`/`unsigned char` for raw buffers, and only form a `std::uint32_t` value via `memcpy` (or `std::bit_cast` with the same constraints).

So the subtle cross-platform problem: **forming/using a `uint32_t*` from an arbitrary `void*` can lead to misaligned access and SIGBUS on stricter architectures** - and the cast itself is a "future bug magnet" even if today you only `memcpy`.
