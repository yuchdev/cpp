#include <iostream>
#include <tuple>
#include <memory>
#include <vector>
#include <scoped_allocator>

using namespace std;

// TODO: piecewise construct
// TODO: scoped_allocator_adapter

/*

New features:
* Resource management pointers: unique_ptr, shared_ptr, and weak_ptr (34.3)
* tuples (34.2.4.2)
* A fixed-sized contiguous sequence container: array (34.2.1)
* Scoped allocators(34.4.4)
* Garbage collection ABI (34.5)

Examples:
1. tuple example (34.2.4)
2. weak_ptr example (34.3.3)
3. Scoped allocator examples (34.4.4)
4. Garbage collection (34.5)
5. Low-level memory algorithms 34.6

*/

// 1. tuple example (34.2.4)
namespace cpp4
{
std::tuple<char, int, double>  create_tuple()
{
    // return { 'a', 1, 1.0 }; error until C++17
    return std::make_tuple('a', 1, 1.0);
}

} // namespace cpp4 

void show_tuple()
{

    // create tuple
    std::tuple<char, int, double> t = cpp4::create_tuple();

    char c {};
    int i {};
    double d {};

    // unpack from tuple
    std::tie(c, i, d) = t;

    // TODO: piecewise construct
}

// 2. weak_ptr example (34.3.3)
void show_weak_ptr()
{
    // std::weak_ptr is a very good way to solve the dangling pointer problem. By just using raw pointers
    // it is impossible to know if the referenced data has been deallocated or not. 
    // Instead, by letting a shared_ptr manage the data, and supplying weak_ptr to users of the data, 
    // the users can check validity of the data by calling expired() or lock()

    // OLD, problem with dangling pointer
    // PROBLEM: ref will point to undefined data!
    int* ptr = new int(10);
    int* ref = ptr;
    delete ptr;

    // NEW, 
    // SOLUTION: check expired() or lock() to determine if pointer is valid

    std::weak_ptr<int> weak1;
    {
        std::shared_ptr<int> sptr;

        // takes ownership of pointer
        sptr.reset(new int);
        *sptr = 10;

        // refcount == 2
        std::shared_ptr<int> sptr2 = sptr;

        // get pointer to data without taking ownership
        // refcount is still 2
        weak1 = sptr;

        // lock, counter == 3
        if (auto tmp = weak1.lock()) {
            std::cout << *tmp << '\n';
        }
        else {
            std::cout << "weak1 is expired\n";
        }
    }

    // lock, but object is destroyed
    if (auto tmp = weak1.lock()) {
        std::cout << *tmp << '\n';
    }
    else {
        std::cout << "weak1 is expired\n";
    }

    // Then you have a dependency cycle.If you use shared_ptr, objects will no longer be automatically 
    // freed when you abandon reference on them, because they reference each other in a cyclic way.
    // This is a memory leak

    // You break this by using weak_ptr.The "owner" typically use shared_ptr and the "owned" use a weak_ptr to its parent,
    // and convert it temporarily to shared_ptr when it needs access to its parent

    // weak_ptr is also good to check the correct deletion of an object - especially in unit tests

    /*
    std::weak_ptr<X> weak_x{ shared_x };
    shared_x.reset();
    BOOST_CHECK(weak_x.lock());
    ... //do something that should remove all other copies of shared_x and hence destroy x
    BOOST_CHECK(!weak_x.lock());
    */
}

// 3. Scoped allocator examples (34.4.4)
namespace std
{

// Like an iterator allocator is a pure abstraction
// Minimal interface that supports STL interface
template <typename T>
class custom_allocator
{
public:
    // value-types
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;

    // Support of any other allocator
    // it could be useful if a data structure holds some 'service' data
    // (deque, unordered containers) and they need their own allocator
    template <typename U>
    struct rebind
    {
        typedef custom_allocator<U> other;
    };

    // allocator doesn't hold any state so ctor/~ are trivial
    custom_allocator() noexcept
    {
        cout << "Type: " << typeid(T).name() << endl;
        cout << "Ref: " << typeid(reference).name() << endl;
        cout << "Ptr: " << typeid(pointer).name() << endl;
    }
    custom_allocator(const custom_allocator&) noexcept {}
    ~custom_allocator() noexcept {}

    // ctor for service data
    // deque, unordered
    template <typename U>
    custom_allocator(const custom_allocator<U>&) throw() {}

    // dereference
    pointer address(reference val)
    {
        return &val;
    }

    const_pointer address(const_reference val)
    {
        return &val;
    }

    // max number of elements
    size_type max_size() const throw()
    {
        return numeric_limits<size_t>::max() / sizeof(T);
    }

    // the pool for n elements without calling ctor
    // hint meaning is implementation-dependent
    // it could be used for example with overloaded new, a pointer to a prev free memory slot
    pointer allocate(size_type num, typename custom_allocator<T>::const_pointer hint = nullptr)
    {
        return reinterpret_cast<pointer>(::operator new(num * sizeof(T)));
    }

    // release the memory block
    // all destructors are called that moment
    void deallocate(pointer p, size_type)
    {
        // deallocate does not provide any guarantee passing nullptr
        ::operator delete(reinterpret_cast<void*>(p));
    }

    // construct one element
    void construct(pointer p, const T& val)
    {
        new (reinterpret_cast<void*>(p)) T(val);
    }

    // destroy one element
    void destroy(pointer p)
    {
        // case of explicit destructor call
        p->~T();
    }

private:
    // in private section could be any cache or service data
};

// obligatory specialization for void!
template <>
class custom_allocator<void>
{
public:
    // value types
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef void value_type;
    typedef void* pointer;
    typedef const void* const_pointer;

    template <typename U>
    struct rebind
    {
        typedef custom_allocator<U> other;
    };
};

// template operator == says that allocators are compatible
// i.e. create one, destroy other 
template <typename T1, typename T2>
bool operator==(const custom_allocator<T1>&, const custom_allocator<T2>&) throw()
{
    return true;
}

template <typename T1, typename T2>
bool operator!=(const custom_allocator<T1>&, const custom_allocator<T2>&) throw()
{
    return false;
}


} //namespace std

void show_custom_allocator()
{

    // objects with different allocators do not mixed
    // i.e. list list::splice with different allocators would work,
    // not actual splice, but per-element insertions

    typedef std::vector<int, std::custom_allocator<int> > custom_vector;
    custom_vector v;
    v.push_back(1);
    v.push_back(2);
    v.pop_back();

    // A rather sneaky problem can occur when using containers and user - defined allocators: 
    // Should an element be in the same allocation area as its container?

    // The solution is the ability to tell a container which allocator to pass to elements
    // The key to that is the class scoped_allocator, which provides the mechanism to keep track of an outer allocator

    // We have three alternatives for allocation of vectors of strings

    // 1. vector (only) uses custom_allocator and string uses its own allocator (the default)
    std::vector<std::string, std::custom_allocator<std::string>> v1;

    // 2. vector and string use custom_allocator (as above):
    using custom_string = std::basic_string<char, char_traits<char>, std::custom_allocator<char>>;
    std::vector<custom_string, std::custom_allocator<custom_string>> v2;

    // However, this is awkward and error-prone, because it's too easy to accidentally insert a string
    // which doesn't use the same allocator:
    //v2.push_back(std::string("oops!")) // allocator argument needed!

    // The purpose of std::scoped_allocator_adaptor is to automatically propagate an allocator to the objects 
    // it constructs if they support construction with an allocator. So the code above would become:
    std::vector<custom_string, std::scoped_allocator_adaptor<std::custom_allocator<custom_string>>> v3;

    // no allocator argument needed!
    v3.push_back(custom_string("A"));
    v3.push_back(custom_string("B"));

    // Now the vector's allocator is automatically used to construct its elements, 
    // even though the objects being inserted, String("hello") and String("world"), 
    // are not constructed with the same allocator
}

// 4. Garbage collection(34.5)
// C++ garbage collectors have traditionally been conservative collectors; 
// that is, they do not move objects around in memory and have to assume that every word in memory 
// might contain a pointer
// Only interface is provided

// 5. Low-level memory algorithms
void show_low_level_func()
{
    // STL has functions for working with uninitialized memory
    // they all have strong exception guarantee

    // init memory with the value
    int arr[10] = {};
    uninitialized_fill(arr, arr + sizeof(arr) / sizeof(int), 1);

    // init n elements with the value
    uninitialized_fill_n(arr, 5, 2);

    // copy raw interval
    int arr2[10] = {};
    uninitialized_copy(arr, arr + sizeof(arr) / sizeof(int), arr2);

    // short-term memory buffer
    pair<int*, ptrdiff_t>p = get_temporary_buffer<int>(10);
    // returns pair = pointer to memory and size

    // doesn't have nullptr-check
    if (p.first != 0)
        return_temporary_buffer(p.first);
}

int main()
{
    show_tuple();
    show_weak_ptr();
    show_custom_allocator();
    return 0;
}
