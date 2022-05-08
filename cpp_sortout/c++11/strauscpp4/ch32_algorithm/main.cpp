#include <iostream>
#include <algorithm>
#include <functional>
#include <numeric>
#include <vector>
#include <list>
#include <string>
#include <iterator>

using namespace std;

/*

New features:
* More algorithms, such as move(), copy_if(), and is_sorted() (32)
* Improved function adaptors: function and bind() (33.5)

Examples:
1. C++11 algorithms
2. Iterator tags (see basics)
3. Iterator traits (33.1.3)
4. Iterator adapters (move iterator) (33.2)
5. bind&men_fn (33.5.1-2)
*/

//1. C++11 algorithms
void show_new_algorithms() {
    // copy_if(), copy_n(), uninitialized_copy_n()

    int source[5] = { 0,12,34,50,80 };
    int target[5];
    //copy 5 elements from source to target
    copy_n(source, 5, target);

    // uninitialized_copy_n() is chiefly useful in low-level memory management operations 
    // where you want to separate the allocation of raw memory from the construction operation
    const int n = 4;
    int first[n] = { 0,1,2,3 };
    int result[n];
    uninitialized_copy_n(first, n, result);

    copy_if(first, first + n, result, [](const int& x) {return x > 0; });

    // iota (in <numeric>)
    // The new algorithm iota() was inspired by an APL operator with the same name
    // iota() creates a range of sequentially increasing values
    int a[5] = { 0 };
    iota(a, a + 5, 10); //changes a to {10,11,12,13,14}

    // all_of, any_of and none_of
    // check if the supplied unary predicate return true for all, any or no element in a range
    int numbers[] = { 1, 2, 42, 7, 0 };
    bool b = std::all_of(std::begin(numbers), std::end(numbers), [](const int& x) {return x > 0; });
    b = std::any_of(std::begin(numbers), std::end(numbers), [](const int& x) {return x == 0; });
    b = std::none_of(std::begin(numbers), std::end(numbers), [](const int& x) {return x < 0; });

    // is_sorted and is_sorted_until
    // The first algorithm, is_sorted, checks if the elements of the range are sorted ascending
    // The second algorithm, is_sorted_until, is similar in that it checks that the elements of a range are sorted
    // However, the function does not return a boolean value, but an iterator to the upper bound of the largest sorted sub-range
    b = std::is_sorted(std::begin(numbers), std::end(numbers));
    auto last_ascending = std::is_sorted_until(std::begin(numbers), std::end(numbers));

    // is_partitioned
    // Algorithm is_partitioned checks whether a range is partitioned, 
    // i.e. sorted according to the supplied unary predicate so that all elements 
    // that satisfy the predicate come before all elements that do not satisfy the predicate
    // The function returns true if the range is empty
    struct message
    {
        std::string text;
        bool delivered;
    };

    std::vector<message> messages = {
        { "first message", true },
        { "second message", true },
        { "third message", false },
        { "fourth message", false },
        { "fifth message", true },
    };

    auto is_delivered = [](message const & msg) {return msg.delivered; };

    // is not partitioned
    std::cout << std::boolalpha
        << std::is_partitioned(std::begin(messages), std::end(messages), is_delivered)
        << std::endl;

    // is_permutation
    // Algorithm is_permutation determines whether a range is a permutation of another range
    int numbers1[] = { 1, 2, 42, 7, 0 };
    int numbers2[] = { 0, 1, 2, 7, 42 };
    std::cout << std::boolalpha
        << std::is_permutation(std::begin(numbers1), std::end(numbers1), std::begin(numbers2))
        << std::endl;

    // minmax_element
    // std::tie() creates a tuple of lvalue references to its arguments or instances of std::ignore
    // std::vector<int>::iterator minimum, maximum;
    //TODO: ? std::tie(minimum, maximum) = std::minmax_element(std::begin(numbers), std::end(numbers));

    auto mm = std::minmax_element(std::begin(numbers), std::end(numbers));

    std::cout << "minimum = " << *mm.first << std::endl;
    std::cout << "maximum = " << *mm.second << std::endl;
}

//2. Iterator tags(see ch.05 example)

//3. Iterator traits(33.1.3)
namespace cpp4 {

// TODO: wtf unable to compile
//template <typename Iter>
//using category<Iter> = typename std::iterator_traits<Iter>::iterator_category;

} // namespace cpp4 

void show_iterator_traits() {
    // For an iterator that does not have these member types(e.g., an int*), 
    // we provide a specialization of iterator_traits
    std::vector<int> v(5);
    int arr[5] = {};

    //template <typename Iter>
    //using difference<Iter> = typename iterator_traits<Iter>::difference_type;

    //using it1_category = typename cpp4::category<it1>;
    //using it1_diff = typename difference<it1>;

    std::cout << typeid(typename std::iterator_traits<decltype(v.begin())>::iterator_category).name() << std::endl;
    std::cout << typeid(typename std::iterator_traits<decltype(begin(arr))>::iterator_category).name() << std::endl;
    std::cout << typeid(typename std::iterator_traits<decltype(v.begin())>::difference_type).name() << std::endl;
    std::cout << typeid(typename std::iterator_traits<decltype(begin(arr))>::difference_type).name() << std::endl;

}

//4. Iterator adapters(move iterator) (33.2)
void show_iterator_adapters() {

    // In <iterator>, the standard library provides adaptors to generate useful related iterator types
    // reverse_iterator 
    // back_insert_iterator 
    // front_insert_iterator 
    // insert_iterator 
    {
        std::vector<int> v{ 1,2,3,4,5 };
        std::list<int> l{ -1,-2,-3 };
        std::copy(v.begin(), v.end(), // may be simplified with std::inserter
            std::insert_iterator<std::list<int>>(l, std::next(l.begin())));
    }

    {
        std::vector<int> v{ 1,2,3,4,5 };
        std::list<int> l{ -1,-2,-3 };
        // std::inserter return a std::insert_iterator which can be used to insert elements
        std::copy(v.begin(), v.end(), std::inserter(l, l.begin()));
    }

    // move_iterator 
    // We usually make a move iterator from another iterator using a helper function
    std::vector<std::string> v1 = { "this", "is", "an", "example" };
    std::vector<std::string> v2;

    // TODO: manage the example:
    //auto bk_it = std::back_inserter(v2);
    //auto mv_it = std::make_move_iterator(bk_it);
    //std::copy(v1.begin(), v1.end(), mv_it);

    // now v1 is empty

    // raw_storage_iterator
    // The output iterator std::raw_storage_iterator makes it possible for standard algorithms
    // to store results in uninitialized memory
}

namespace cpp4 {

// some free function
double cube(double d){
    return d*d*d;
}


void out_two_params(int t1, const std::string& t2) {
    std::cout << "T1 = " << t1 << " T2 = " << t2 << std::endl;
}

} // namespace cpp4 

//5. bind&men_fn(33.5.1 - 2)
void show_func_adapters() {
    
    // bind
    // Given a function and a set of arguments, bind() produces a function object that can be called with
    // the remaining arguments, if any, of the function
    auto cube2 = std::bind(&cpp4::cube, 2);
    double d = cube2();

    // The placeholders are found in the (sub)namespace std::placeholders that is part of <functional>
    // The placeholder mechanism is very flexible
    {
        using namespace placeholders;
        cpp4::out_two_params(2, std::string("hello"));
        auto bnd1 = std::bind(cpp4::out_two_params, _1, _2);
        auto bnd_replace = std::bind(cpp4::out_two_params, _2, _1);
        bnd1(2, std::string("hello"));
        bnd_replace(std::string("bye"), 1);
    }

    // mem_fn
    // The function adapter mem_fn() produces a function object that can be called as a nonmember function
    // auto draw = mem_fn(&Shape::draw);
}

int main() {
    show_new_algorithms();
    show_iterator_traits();
    show_iterator_adapters();
    show_func_adapters();
    return 0;
}
