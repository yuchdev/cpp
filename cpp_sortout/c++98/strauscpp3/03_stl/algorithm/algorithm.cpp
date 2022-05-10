#include "algorithm.h"
#include "predicates.h"
#include <set>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <cstdlib>
#include <cctype> // std::tolower
#include <cstring>

using namespace std;
typedef vector<int> v_int;
typedef vector<int>::iterator v_iit;

template<typename T, template <typename ELEM, typename = std::allocator<ELEM> > class CONT >
void print_container(const CONT<T>& c)
{
    typename CONT<T>::const_iterator it = c.begin();
    while (it != c.end()) {
        cout << (*it) << ' ';
        ++it;
    }
    cout << endl;
}


/// Pair values extraction functor
/// Extracts 1-st value (key)
template <typename Pair> struct select1st
{
    typedef Pair argument_type;
    typedef typename Pair::first_type result_type;
    const result_type& operator()(const argument_type& p) const
    {
        return p.first;
    }
};

/// Pair values extraction functor
/// Extracts 2-nd value
template <typename Pair> struct select2nd
{
    typedef Pair argument_type;
    typedef typename Pair::second_type result_type;
    const result_type& operator()(const argument_type& p) const
    {
        return p.second;
    }
};

// Non-modifying algorithms
// for_each
// find
// count
// equality and mismatch
// search
void show_non_modif()
{
    int arr[] = { 0,4,1,9,3,7,3,7,3,8 };
    int arr1[] = { 7,3,7,4,5,2,8,8,0,4 };
    vector<int> vi(arr, arr + sizeof(arr) / sizeof(int));
    vector<int> vi1(arr1, arr1 + sizeof(arr1) / sizeof(int));

    // --- find, find_if ---
    v_iit it = find(vi.begin(), vi.end(), 7);

    // Predicate can be anything to which the operation () is applied
    it = find_if(vi.begin(), vi.end(), my_less_predivate(7));

    // The _if suffix is added because of the difficulty of overloading the generic algorithm

    // --- for_each ---
    // Using for_each() as an accumulator with the data-storing functor
    // Note, for_each() is a non-modifying algorithm
    sum_accum<int> s;
    s = for_each(vi.begin(), vi.end(), std::ref(s));
    int sum = s.res;

    // --- find_first_of ---
    // Finds the first element in the sequence,
    // which exists in the second sequence
    it = find_first_of(vi.begin(), vi.end(), vi1.begin(), vi1.end());

    // searches for duplicates and returns an iterator to the first element
    it = adjacent_find(vi1.begin(), vi1.end() /*, pred*/);

    // --- count, count_if ---
    long long c = count(vi.begin(), vi.end(), 7);
    c = count_if(vi.begin(), vi.end(), my_less_predivate(7));

    // --- mismatch ---
    // compares 2 sequences and finds the first mismatch
    // The end of the second sequence is undefined, it is considered to be not smaller than the first one
    pair<v_iit, v_iit> p1 = mismatch(vi.begin(), vi.end(), vi1.begin());

    // --- equal ---
    // just compares the 2 sequences, without specifying the place of mismatch
    bool b = equal(vi.begin(), vi.end(), vi1.begin());

    // --- search, find_end, search_n ---
    int arr2[] = { 9,3,7 };
    size_t arr2_sz = sizeof(arr2) / sizeof(int);

    // search - finds a subsequence in the sequence
    // and returns an iterator on the FIRST matching element
    // E.g. find array arr2 in vector vi
    v_iit it2 = search(vi.begin(), vi.end(), arr2, arr2 + arr2_sz);

    // find_end - finds a subsequence in the sequence
    // and returns an iterator on the first matching element
    // E.g. find array arr2 in vector vi, but returns iterator to the end
    it2 = find_end(vi.begin(), vi.end(), arr2, arr2 + arr2_sz);

    // Finds a sequence of 2 elements equal 8 in vi1
    it2 = search_n(vi1.begin(), vi1.end(), 2, 8);
}

// Modifying algorithms
// copy, copy_if, copy_backward
// transform
// replace, replace_if, replace_copy, replace_copy_if
// remove,remove_copy,remove_copy_if
// fill,fill_n
// generate,generate_n
// unique,unique_copy
// reverse,reverse_copy
// rotate,rotate_copy
// random_shuffle
// swap,iter_swap,swap_ranges
void show_modif()
{

    int arr[] = { 0,4,1,9,3,7,3,7,3,8 };
    int arr1[] = { 7,3,7,4,5,2,8,8,0,4 };
    vector<int> vi(arr, arr + sizeof(arr) / sizeof(int));
    vector<int> vi1(arr1, arr1 + sizeof(arr1) / sizeof(int));

    // Modifying algorithms do not necessarily write changes
    // into the input sequence

    // Instead of inserting and changing, elements are overwritten elsewhere in the container,
    // and then deleted/modified manually.

    // This is done because the algorithm knows nothing about the container being modified,
    // and is unable, for example, to change its size parameter

    // --- copy, copy_backward ---
    // Copies anywhere, not necessarily to the coneiner. For example, to the stream
    copy(vi.begin(), vi.end(), ostream_iterator<int>(cout));
    cout << endl;

    // The use of back_inserter prevents from writing beyond the container,
    // and allows you to conveniently write at the end
    vector<int> v_copy;
    copy(vi.begin(), vi.end(), back_inserter(v_copy));

    // Input and output sequences may overlap
    // if the beginning of the OUTPUT sequence is inside the INPUT sequence
    // it is necessary to use copy_backward, which is copying from the end
    // For example, copy all elements into the same container with an offset of 2
    //copy_backward( v_copy.begin(), v_copy.end(), v_copy.begin() + 2 );

    // 
    copy_if(vi.begin(), vi.end(), back_inserter(v_copy), less<int>(7));

    // --- transform ---
    // Creates an output sequence by applying the operation to an input sequence,
    // or two input sequences
    // Does not accept more than 2 sequences
    // It can be used to clear polymorphic container, change case_sens, etc.

    vector<int> vcolors;
    vector<circle> vshape1;
    vshape1.push_back(circle(10));
    vshape1.push_back(circle(20));
    vshape1.push_back(circle(30));

    // Form a container of colors from the container of shapes
    transform(vshape1.begin(), vshape1.end(), back_inserter(vcolors), mem_fun_ref(&shape::color));

    // An overload accepting two sequences
    // Demonstration of multiplication of two vectors and writing to a third
    vector<int> res;
    transform(vi.begin(), vi.end(), vi1.begin(),
        back_inserter(res), multiplies<int>());

    // --- unique,unique_copy ---
    // unique moves all non-unique elements to the end of the sequence,
    // and returns an iterator to the end of the duplicates sequence
    vi.clear();
    int arr2[] = { 0,4,1,1,1,7,3,3,3,8,6,2,9,4,4,7 };
    vector<int> vi2(arr2, arr2 + sizeof(arr2) / sizeof(int));
    sort(vi2.begin(), vi2.end());
    v_iit vit1 = unique(vi2.begin(), vi2.end());

    // remove them manually
    vi2.assign(arr2, arr2 + sizeof(arr2) / sizeof(int));
    sort(vi2.begin(), vi2.end());

    // unique_copy forms a new sequence, does not touch the old one
    unique_copy(vi2.begin(), vi2.end(), back_inserter(vi));

    // --- replace, replace_if, replace_copy, replace_copy_if ---
    // The family of these algorithms replaces the specified values in the target sequence

    // Replace 7 with 8
    replace(vi.begin(), vi.end(), 7, 8);

    // Replace 4 with 6 in vi, and insert into vector vi2 (does not touch the old one)
    vi2.clear();
    replace_copy(vi.begin(), vi.end(), back_inserter(vi2), 4, 6);

    // Replace all <= 2 with 3
    replace_if(vi.begin(), vi.end(), bind2nd(less_equal<int>(), 2), 3);

    // Replace all <= 2 with 3 in vi1 and paste into the vi vector (does not touch the old one)
    replace_copy_if(vi1.begin(), vi1.end()
        , back_inserter(vi)
        , bind2nd(less_equal<int>(), 2)
        , 3);

    // --- remove,remove_copy,remove_copy_if ---
    // The family removes elements (in or out of sequence), based on a value or predicate.
    // remove simply writes elements to be deleted to the end of the sequence (remove them manually later)
    print_container(vi);
    v_iit it = remove(vi.begin(), vi.end(), 5);
    print_container(vi);


    // remove_if uses predicate
    it = remove_if(vi.begin(), vi.end(), bind2nd(less<int>(), 4));
    print_container(vi);

    // remove_copy_if - the element is placed on the output,
    // if it does not satisfy the predicate condition
    vector<int> vi3;
    remove_copy_if(vi1.begin(), vi1.end(), back_inserter(vi3), bind2nd(less<int>(), 2));
    print_container(vi3);

    // --- fill,fill_n ---
    // The family of functions fills the container with values

    // Assign 5 to the first 2 elements
    fill(vi3.begin(), vi3.begin() + 2, 5);

    // Same with fill_n
    fill_n(vi3.begin(), 2, 6);

    // --- generate,generate_n ---
    // A family of functions generating container values from a function
    // Generation of random numbers
    generate(vi3.begin(), vi3.end(), rand);

    // Output 20 random numbers
    generate_n(ostream_iterator<int>(cout), 20, rand);

    // --- reverse,reverse_copy ---
    // reverse the order of the sequence
    reverse(vi.begin(), vi.end());

    // reverse_copy produces a new sequence in reverse order
    vector<int> vi5;
    reverse_copy(vi.begin(), vi.end(), back_inserter(vi5));

    // --- rotate,rotate_copy ---
    // rotate cyclically shifts the elements until the second argument of the function
    // appear the first argument of the sequence
    int arr3[] = { 7,2,89,3,8,6,2,6 };
    size_t sz3 = sizeof(arr3) / sizeof(int);

    // E.g. a cyclic shift of the array by 3 to the left
    rotate(arr3, arr3 + 3, arr3 + sz3);

    // rotate_copy
    // Copying an array shifted by 5 into vi5
    vi5.clear();
    rotate_copy(arr3, arr3 + 5, arr3 + sz3, back_inserter(vi5));

    // --- random_shuffle ---
    // Shuffling the container
    random_shuffle(arr3, arr3 + sz3/* , rand_gen()*/);

    // --- swap,iter_swap,swap_ranges ---
    // swaps two objects
    swap(vi, vi1);

    // iter_swap
    // swap replaces 2 elements, not necessarily in the same container
    swap(*vi.begin(), *vi1.begin());

    // swap_ranges
    // swaps two ranges of equal size, e.g.
    // first 2 elements vi1 and vi2
    swap_ranges(vi1.begin(), vi1.begin() + 2, vi2.begin());
}

// --------------- Sort ---------------
//  sort,stable_sort
//  parital_sort,parital_stable_sort
//  nth_element, binary_search
//  merge,inplace_merge
//  lower_bound, upper_bound
//  partition,stable_partition
void show_sort()
{
    int arr[] = { 0,4,1,9,3,7,3,7,3,8 };
    int arr1[] = { 7,3,7,4,5,2,8,8,0,4 };
    vector<int> vi(arr, arr + sizeof(arr) / sizeof(int));
    vector<int> vi1(arr1, arr1 + sizeof(arr1) / sizeof(int));

    // --- sort,stable_sort ---
    // Require iterators with random access
    sort(vi.begin(), vi.end());

    // stable_sort keeps the order of the equial elements
    stable_sort(vi1.begin(), vi1.end());

    vi.assign(arr, arr + sizeof(arr) / sizeof(int));
    vi1.assign(arr1, arr1 + sizeof(arr1) / sizeof(int));

    // --- parital_sort,partial_sort_copy ---
    // the order is split into two parts, and the sorting continues
    // until the first part is sorted
    partial_sort(vi.begin(), vi.begin() + 3, vi.end());

    // the sequence is split into two parts, copied into the output sequence
    // and the sorting continues until the first part is sorted
    partial_sort_copy(vi.begin(), vi.end(), vi1.begin(), vi1.begin() + 3);

    vi.assign(arr, arr + sizeof(arr) / sizeof(int));
    vi1.assign(arr1, arr1 + sizeof(arr1) / sizeof(int));
    // --- nth_element ---
    // Sorting until the n-th element takes its place
    // Useful when calculating medians, percentiles, etc.
    // For example, the third element
    nth_element(vi.begin(), vi.begin() + 3, vi.end());

    // --- binary_search, lower_bound, upper_bound ---
    // binary_search - search in a sorted sequence
    sort(vi.begin(), vi.end());
    bool b = binary_search(vi.begin(), vi.end(), 1);

    // lower_bound and upper_bound - return
    // upper and lower boundary of the same elements
    // sorted sequence
    v_iit it = lower_bound(vi.begin(), vi.end(), 7);
    it = upper_bound(vi.begin(), vi.end(), 7);

    // --- merge,inplace_merge ---
    // Family of functions for merging sorted sequences

    // merge
    // Merging of two sequences
    sort(vi1.begin(), vi1.end());
    vector<int> res;
    merge(vi.begin(), vi.end(), vi1.begin(), vi1.end(), back_inserter(res));

    // inplace_merge
    // Merge two sorted parts of the same sequence
    res.clear();
    // not to reallocate memory
    res.reserve(100);

    copy(vi.begin(), vi.end(), back_inserter(res));
    copy(vi1.begin(), vi1.end(), back_inserter(res));
    inplace_merge(res.begin(), res.begin() + vi.size(), res.end());

    // --- partition,stable_partition ---
    // The container partition; placing all elements that 
    // satisfy the predicate before those that do not satisfy it
}


// --------------- Heap ---------------
// make_heap, sort_heap
// push_heap, pop_heap
void show_heap()
{

    // the priority_queue is based on the heap data structure

    vector<int> v({ 0, 2, 5, 1, 5, 3, 8, 3, 6, 5 });
    print_container(v);

    // turn a vector into a heap (heapify)
    make_heap(v.begin(), v.end());
    print_container(v);

    // add an item to the heap
    // always adds the last item
    v.push_back(3);
    push_heap(v.begin(), v.end());
    print_container(v);

    // remove an element from the heap
    // the last element is also taken out
    pop_heap(v.begin(), v.end());
    v.pop_back();
    print_container(v);

    // sort the heap (after sorting, the sequence is no longer a heap)
    sort_heap(v.begin(), v.end());
    print_container(v);
}


// --------------- Min|Max ---------------
// min, max, max_element, min_element
// lexigraphical_compare
bool no_case(char c1, char c2)
{
    return std::tolower(c1) < std::tolower(c2);
}

void show_min_max()
{
    // --- min, max, max_element, min_element ---
    double m = max(1.2, 2.5);
    const char* a = "aaa";
    const char* b = "bbb";
    string s(min(a, b));
    // max/min elements from a pair or from a container
    // no comments

    const char* A = "AAA";

    // lexigraphical_compare
    // true if the first range compares lexicographically less than than the second.
    // false otherwise(including when all the elements of both ranges are equivalent).
    bool diff = lexicographical_compare(a, a + strlen(a), b, b + strlen(b));
    diff = lexicographical_compare(a, a + strlen(a), A, A + strlen(A), no_case);
}

// ------------------- Set -------------------

// --- includes ---
// --- set_union, set_intersection ---
// --- set_difference, set_symmetric_difference ---
void show_set()
{
    // Set operations are meaningful only for sorted sequences
    // especially effective with set and multiset containers
    int arr[] = { 0,4,1,9,3,7,3,7,3,8 };
    int arr1[] = { 7,3,7,4,5,2,8,8,0,4 };

    set<int> s(arr, arr + sizeof(arr) / sizeof(int));
    multiset<int> ms(arr1, arr1 + sizeof(arr1) / sizeof(int));
    set<int> s1(ms.begin(), ms.end());

    // --- includes ---
    // checks if all members of the first sequence are
    // are also members of the second sequence
    bool b = includes(s.begin(), s.end(), ms.begin(), ms.end());

    // --- set_union, set_intersection ---
    // Merge and intersect, respectively
    vector<int> union_out;
    vector<int> intersection_out;
    set_union(s.begin(), s.end(), s1.begin(), s1.end(), back_inserter(union_out)); //(??? wtf)
    set_intersection(s.begin(), s.end(), ms.begin(), ms.end(), back_inserter(intersection_out));

    // --- set_difference, set_symmetric_difference ---
    vector<int> diff_out;
    vector<int> symm_out;

    vector<int> v1(arr, arr + sizeof(arr) / sizeof(int));
    vector<int> v2(arr1, arr + sizeof(arr1) / sizeof(int));
    sort(v1.begin(), v1.end());
    sort(v2.begin(), v2.end());

    // set_difference; a sequence of elements included in the first, but not included in the second
    set_difference(v1.begin(), v1.end(), v2.begin(), v2.end(), back_inserter(diff_out));

    // set_symmetric_difference; a sequence of elements included in only one of the sequences
    set_symmetric_difference(v1.begin(), v1.end(), v2.begin(), v2.end(), back_inserter(symm_out));
}

// --------------- Permutations ---------------
// next_permutation, prev_permutation
void show_permutations()
{

    // all possible permutations of the sequence
    char v[] = "abcd";
    cout << endl << "start permutations:" << endl << v << '\t';
    while (next_permutation(v, v + 4)) {
        cout << v << '\t';
    }
}

// --------------- C algorithms ---------------
// qsort, bsearch
int c_compare(const void* arg1, const void* arg2)
{
    /* Compare by subtracting (ret>0, ret==0, ret<0)*/
    return (*static_cast<const int*>(arg1)) - (*static_cast<const int*>(arg2));
}

void c_algorithms()
{
    int arr[] = { 0,4,1,9,3,7,3,7,3,8 };
    size_t sz = sizeof(arr) / sizeof(int);
    qsort(arr, sz, sizeof(int), c_compare);
    int key = 1;
    int* i = (int*) bsearch((const void*) &key, (int*) arr, sz, sizeof(int), c_compare);
}
