#include <iostream>

#include <new>
#include <cstring>

// Memory pool class desinged for saving not more than N object 
// with size not more than M in a fixed size pre-allocated memory pool
template <typename T>
class memory_pool{
public:
	memory_pool() :
		block_size(sizeof(T)),
		head(::operator new(block_size * pool_size)){
		::memset(head, 0x0, block_size * pool_size);
	}

	~memory_pool() {
		::operator delete(head);
	}

	void* alloc(size_t s) {

		// it works only for blocks smaller than pool slot
		if (s > sizeof(T))
		{
			return ::operator new(s);
		}

		// Let's find first free slot, it should store 0
		unsigned char* ret = static_cast<unsigned char*>(head);
		for (size_t i = 0; (*ret != 0); ++i)
		{
			ret += block_size;
			if (i == pool_size)
				// pool is full, use standard new instead
				return ::operator new(s);
		}
		return ret;

	}

	void free(void* p, size_t s) const {
		// so as alloc, free it works only for blocks smaller than pool slot
		if (s > sizeof(T))
		{
			::operator delete(p);
		}

		// first byte of block again 0, block is available
		unsigned char* free_byte = static_cast<unsigned char*>(p);
		*free_byte = 0;
	}

private:
	const size_t block_size;
	void* head;
	static size_t pool_size;
};

// static 
template <typename T>
size_t memory_pool<T>::pool_size = 512;

class memory_pool_item{
public:
	memory_pool_item();
	virtual ~memory_pool_item();

	// new/delete overload for using the memory pool
	static void* operator new(size_t s);
	static void operator delete(void* p, size_t s);

	void test();
private:
	int i;
	static memory_pool<memory_pool_item> pool;
};


// Memory pool implementation
memory_pool<memory_pool_item> memory_pool_item::pool;

memory_pool_item::memory_pool_item() :i(){
	std::cout << "memory_pool_item()" << '\n';
}

//virtual 
memory_pool_item::~memory_pool_item(){
    std::cout << "~memory_pool_item()" << '\n';
}

//static 
void* memory_pool_item::operator new(size_t s){
	// memory in pool is already allocated
	return pool.alloc(s);
}
//static 
void memory_pool_item::operator delete(void* p, size_t s){
	// return memory to the pool
	pool.free(p, s);
}

void memory_pool_item::test(){
	std::cout << i++ << '\n';
}


void show_memory_pool(){

    // allocated from pool, addr = 0x003789d0
    memory_pool_item* mpi1 = new memory_pool_item();
    mpi1->test();

    // allocated from pool, addr = 0x003789d8 (prev + sizeof memory_pool_item)
    memory_pool_item* mpi2 = new memory_pool_item();
    mpi2->test();

    // deallocated both from pool
    delete mpi1;
    delete mpi2;

    // allocated from pool again, should again addr = 0x003789d0
    // (block 0 free again)
    memory_pool_item* mpi3 = new memory_pool_item();
    mpi3->test();
    delete mpi3;
}


int main()
{
    show_memory_pool();
    return 0;
}
