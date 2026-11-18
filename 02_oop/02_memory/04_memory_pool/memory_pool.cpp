#include <cassert>
#include <cstring>
#include <iostream>
#include <new>
#include <vector>

// Memory pool class designed for saving not more than N objects
// with size not more than M in a fixed size pre-allocated memory pool.
template <typename T>
class memory_pool
{
public:
    memory_pool() :
        block_size(sizeof(T)),
        head(::operator new(block_size * pool_size))
    {
        ::memset(head, 0x0, block_size * pool_size);
    }

    ~memory_pool()
    {
        ::operator delete(head);
    }

    void* alloc(size_t s)
    {
        // it works only for blocks smaller than pool slot
        if (s > sizeof(T)) {
            return ::operator new(s);
        }

        // Let's find first free slot, it should store 0.
        //
        // The ORIGINAL version of this loop checked `i == pool_size`
        // AFTER already advancing `ret` past the slot it had just
        // examined, and only on the NEXT iteration's condition check did
        // it read `*ret` again -- by which point, once i reached
        // pool_size - 1, `ret` had already been pushed one slot past the
        // end of the buffer, and that out-of-bounds byte got READ before
        // the bounds check had another chance to fire. A fully-occupied
        // pool therefore read one byte past its own allocation on every
        // exhaustion -- a heap-buffer-overflow, caught by
        // AddressSanitizer the first time this path was actually
        // exercised with a full pool (see
        // show_pool_exhaustion_and_oversized_fallback()). Checking the
        // bound BEFORE reading the slot, as below, never reads outside
        // [head, head + block_size*pool_size).
        unsigned char* ret = static_cast<unsigned char*>(head);
        for (size_t i = 0; i < pool_size; ++i, ret += block_size) {
            if (*ret == 0)
                return ret;
        }
        return ::operator new(s); // pool is full, use standard new instead
    }

    void free(void* p, size_t s) const
    {
        // alloc() falls back to ::operator new(s) in TWO cases: an
        // oversized request (s > sizeof(T)), or a normally-sized request
        // after the pool is exhausted. Only the first case is visible
        // from `s` alone -- a fallback allocation made because the pool
        // was full still reports s == sizeof(T), identical to a real
        // pool slot, so a size check can never catch it. Checking
        // whether `p` actually falls within the pool's own slab handles
        // both cases correctly with one test.
        //
        // The ORIGINAL version of this function checked only `s >
        // sizeof(T)`, which (a) missed the exhausted-pool case entirely
        // -- leaking that allocation forever, since it was never
        // actually freed -- and (b) still fell through to the pool-slot
        // bookkeeping below even when it DID detect an oversized
        // request, writing a zero byte into memory it had just freed
        // through ::operator delete(p): a real use-after-free, caught
        // with AddressSanitizer once this code path was actually
        // exercised (see show_pool_exhaustion_and_oversized_fallback()).
        unsigned char* pool_start = static_cast<unsigned char*>(head);
        unsigned char* pool_end = pool_start + block_size * pool_size;
        unsigned char* candidate = static_cast<unsigned char*>(p);

        if (candidate < pool_start || candidate >= pool_end) {
            ::operator delete(p);
            return;
        }

        // first byte of block again 0, block is available
        *candidate = 0;
    }

    static size_t pool_size;

private:
    const size_t block_size;
    void* head;
};

// static. Deliberately small so exhausting it in a demo means allocating
// a handful of objects, not hundreds -- the pool's bookkeeping logic
// doesn't care how big pool_size is, only that it's fixed for the
// lifetime of this static memory_pool<memory_pool_item> instance (see
// show_pool_exhaustion_and_oversized_fallback(): pool_size must NOT be
// mutated after construction, since head's actual buffer size was
// already fixed using whatever pool_size held at that point).
template <typename T>
size_t memory_pool<T>::pool_size = 4;

class memory_pool_item
{
public:
    memory_pool_item();
    virtual ~memory_pool_item();

    // new/delete overload for using the memory pool. operator delete
    // here takes a size_t -- the sized-deallocation form (C++14) -- so
    // the pool knows the block size without recomputing or storing it.
    static void* operator new(size_t s);
    static void operator delete(void* p, size_t s);

    void test();

private:
    int i;
    static memory_pool<memory_pool_item> pool;
};

// Memory pool implementation
memory_pool<memory_pool_item> memory_pool_item::pool;

memory_pool_item::memory_pool_item() : i()
{
    std::cout << "memory_pool_item()" << '\n';
}

memory_pool_item::~memory_pool_item()
{
    std::cout << "~memory_pool_item()" << '\n';
}

void* memory_pool_item::operator new(size_t s)
{
    // memory in pool is already allocated
    return pool.alloc(s);
}

void memory_pool_item::operator delete(void* p, size_t s)
{
    // return memory to the pool
    pool.free(p, s);
}

void memory_pool_item::test()
{
    std::cout << i++ << '\n';
}

void show_memory_pool()
{
    // Allocated from the pool; freeing and re-allocating a slot reuses
    // the same address -- proven below, not just claimed in a comment.
    memory_pool_item* mpi1 = new memory_pool_item();
    mpi1->test();

    memory_pool_item* mpi2 = new memory_pool_item();
    mpi2->test();
    assert(mpi2 != mpi1); // distinct, adjacent slots

    void* addr1 = mpi1;
    delete mpi1;
    delete mpi2;

    // Allocated from the pool again: block 0 is free again, so this
    // reuses the exact address mpi1 had.
    memory_pool_item* mpi3 = new memory_pool_item();
    mpi3->test();
    assert(mpi3 == addr1);
    delete mpi3;
}

// The README's claim this file never actually exercised before: "pool
// exhausted, fall back to ::operator new". The happy path (a handful of
// objects, never more than fit in the pool) never reaches it.
//
// Note what this does NOT do: shrink memory_pool::pool_size at runtime
// to make exhaustion cheaper to trigger. head's buffer is sized ONCE, in
// the constructor, using whatever pool_size held at that moment; mutating
// pool_size afterwards desynchronizes alloc()/free()'s bounds-checking
// logic from the buffer's real size -- confirmed with AddressSanitizer,
// which reported a bogus "free on address which was not malloc()-ed" the
// first time this test tried exactly that. pool_size is deliberately
// small (4) at its point of definition instead, so exhausting it for
// real costs only a few objects.
void show_pool_exhaustion_and_oversized_fallback()
{
    std::vector<memory_pool_item*> items;

    // One more object than the pool has slots for: the last allocation
    // must fall back to the global allocator instead of running off the
    // end of the pool's slab.
    for (size_t n = 0; n < memory_pool<memory_pool_item>::pool_size + 1; ++n)
        items.push_back(new memory_pool_item());

    for (memory_pool_item* item : items)
        delete item; // exercises free()'s fallback path too, for the one
                      // item that came from ::operator new directly
}

int main()
{
    show_memory_pool();
    show_pool_exhaustion_and_oversized_fallback();
    return 0;
}
