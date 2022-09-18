#include <iostream>
#include <forward_list>
#include <unordered_map>
#include <unordered_set>
#include <functional>

using namespace std;

/*

New features:
* initializer_list constructors for containers (31.3.2)
* Move semantics for containers (31.3.2)
* A singly-linked list: forward_list (31.4.2)
* Hash containers: unordered_map, unordered_multimap, unordered_set, and unordered_multiset (31.4.3)
* emplace() operations for containers (31.3.6)

Examples:
1. forward_list operations (31.4.2)
2. emplace and associative containers (31.4.3.1)
3. unordered_map constructors (31.4.3.3)
4. unordered_map own hash function (31.4.3.4)
5. unordered_map own hash function - lambda (31.4.3.4)
6. unordered_map own hash function - std namespace specialization (31.4.3.4)
7. unordered_map bucket count (31.4.3.5)

*/

//1. forward_list operations(31.4.2)
void show_forware_list()
{
    std::forward_list<int> fwd = { 1,2,3,4,5 };

    // operations:

    //You could find the iterator to the last element by incrementing .before_begin N times
    auto before_end = fwd.before_begin();
    for (auto& _ : fwd)
        ++before_end;

    auto inserted = fwd.insert_after(before_end, 6);
    //fwd.emplace_after

    fwd.push_front(0);
    fwd.pop_front();

    fwd.splice_after(inserted, { 7,8,9 });
}

//2. emplace and associative containers(31.4.3.1)
void show_unordered_emplace()
{

    std::unordered_map<std::string, std::string> um;
    um.insert(std::make_pair("atatat0", "0"));
    um.emplace("atatat1", "1");
    um["atatat2"] = "2";

}

//3. unordered_map constructors(31.4.3.3)
//4. unordered_map own hash function(31.4.3.4)
//5. unordered_map own hash function - lambda(31.4.3.4)
namespace cpp4
{

struct record
{
    size_t s;
    std::string n;
};

struct record_hash
{
    size_t operator()(const record& r) const
    {
        return std::hash<size_t>()(r.s) ^ std::hash<std::string>()(r.n);
    }
};

struct record_equal
{
    bool operator()(const record& r1, const record& r2) const
    {
        return (r1.s == r2.s) && (r1.n == r2.n);
    }
};

size_t rec_hash(const record& r)
{
    return std::hash<size_t>()(r.s) ^ std::hash<std::string>()(r.n);
}

bool rec_eq(const record& r1, const record& r2)
{
    return (r1.s == r2.s) && (r1.n == r2.n);
}

} // namespace cpp4 

void show_custom_hash_func()
{

    // hash and equal as template params
    std::unordered_set<cpp4::record, cpp4::record_hash, cpp4::record_equal> s1;

    // hash and equal as constructor params
    const cpp4::record_hash h;
    const cpp4::record_equal e;
    std::unordered_set<cpp4::record, cpp4::record_hash, cpp4::record_equal> s2 { 100, h , e };

    // hash and equal as functions
    std::unordered_set<cpp4::record, decltype(&cpp4::rec_hash), decltype(&cpp4::rec_eq)> s3;

    // hash and equal as lambda
    unordered_set<cpp4::record,
        std::function<size_t(const cpp4::record&)>,
        std::function<bool(const cpp4::record&, const cpp4::record&)> > s4 { 10,
        [] (const cpp4::record& r) { return std::hash<size_t>()(r.s) ^ std::hash<std::string>()(r.n); },
        [] (const cpp4::record& r, const cpp4::record& r2) { return r.s == r2.s && r.n == r2.n; }
    };

    // ... or with naming
    auto hash_me = [] (const cpp4::record& r) { return std::hash<size_t>()(r.s) ^ std::hash<std::string>()(r.n); };
    auto equal_me = [] (const cpp4::record& r, const cpp4::record& r2) { return r.s == r2.s && r.n == r2.n; };

    unordered_set<cpp4::record,
        std::function<size_t(const cpp4::record&)>,
        std::function<bool(const cpp4::record&, const cpp4::record&)> > s5 { 10, hash_me, equal_me };
}

//6. unordered_map own hash function - std namespace specialization(31.4.3.4)
namespace std
{

template<>
struct hash<cpp4::record>
{

    size_t operator()(const cpp4::record& r) const
    {
        return std::hash<size_t>()(r.s) ^ std::hash<std::string>()(r.n);
    }
};

template<>
struct equal_to<cpp4::record>
{
    bool operator()(const cpp4::record& r1, const cpp4::record& r2) const
    {
        return (r1.s == r2.s) && (r1.n == r2.n);
    }
};

}

void show_std_hash_specialization()
{
    // use our specialization
    std::unordered_set<cpp4::record> s1;
}

//7. unordered_map bucket count(31.4.3.5)
void show_load_bucket()
{

    // Note that setting the max_load_factor, calling rehash(), or calling reserve() can be very expensive operations 
    // (worst case O(n*n)) because they can � and in realistic scenarios typically do � cause rehashing of all elements
    std::unordered_set<cpp4::record> s { 1000 };

    // get hash&equality func
    auto hash_func = s.hash_function();
    auto eq_func = s.key_eq();

    s.emplace(cpp4::record { 1, std::string("one") });
    s.emplace(cpp4::record { 2, std::string("two") });

    float lf = s.load_factor();
    float mlf = s.max_load_factor();
    s.max_load_factor(0.7f);

    // Rehashing occurs only if the new number of elements is greater than max_load_factor()*bucket_count()
    s.rehash(2000);
    //reserve

    size_t bc = s.bucket_count();
    size_t mbc = s.max_bucket_count();
}

// 21.	Is it a valid code in C++? What the undescore symbol does stand for?
synchronized void Test() {
    std::map<int, string> id_names = {{1, "John"}, {2, "Tom"}, {3, "Sara"}};
    for (auto [_, name] : id_names) {
        std::cout << name << std::endl;
    }
}


int main()
{
    show_forware_list();
    show_unordered_emplace();
    show_custom_hash_func();
    show_std_hash_specialization();
    show_load_bucket();
    return 0;
}
