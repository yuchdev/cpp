#include <iostream>
#include <list>
#include <set>
#include <vector>
#include <cstdlib>
#include <iterator>
#include <memory>
#include "checked_iterator.h"

using namespace std;

// In C++,  modifying a temporary object is forbidden
// That's why some iterators with automatic lifetime cannot be modified.
// For example, sort(++a, b) - will compile if a is pointer,  win't if a is object
void show_iterator_traits()
{
    // An iterator is an abstraction (something that behaves like an iterator is an iterator)
    // Most often it is a pointer or object

    // Types of iterators:
    // * Output
    // * Input
    // * Forward
    // * Bidirectional
    // * Random-access
    int a[] = { 0,1,9,3,5,4,2,7,6,5 };

    // vector has Random Access iterators
    vector<int> v1(a, a + sizeof(a) / sizeof(int));

    // list has Bidirectional
    list<int> l1(a, a + sizeof(a) / sizeof(int));

    // Distance between iterators
    // vector
    typedef vector<int>::iterator::difference_type vect_int_dist;
    vect_int_dist vdist = distance(v1.begin(), v1.end());

    // swap values by iterator (first with the last)
    vector<int>::iterator lst = v1.end() - 1;
    iter_swap(v1.begin(), lst);

    // list
    typedef list<int>::iterator::difference_type list_int_dist;
    list_int_dist ldist = distance(l1.begin(), l1.end());
    cout << "Distance = " << ldist << endl;

    // in such algorithms, the "iterator trait" object is also passed implicitly
    // it allows to choose an overload that corresponds to the type of iterator
    // For example, for RA it is just a difference,
    // for BI it is a "running" from one iterator to another

    // Similarly with the `advance()` operation (analog to += for list)
    // += 5
    const vect_int_dist run_v = 5;
    vector<int>::iterator v_b = v1.begin();
    advance(v_b, run_v);

    // running through the list 5 elements (analog of operator[] for std::list)
    const list_int_dist run_l = 5;
    list<int>::iterator l_b = l1.begin();
    advance(l_b, run_l);

    // also insertion iterators and inverse iterators exist
    // as well as stream iterators 
    // With their help, it is possible, for example, to duplicate every line, read every third line, etc.
}

void show_checked_iterator()
{

    // Connectors with verification 
    Checked_container< vector<int> > v1;
    Checked_container< vector<int> > v2(10);

    Checked_container< list<int> > l1;
    Checked_container< list<int> > l2(10);

    // Iterators with verification 
    try {
        // initialize v2.begin()
        Checked_container< vector<int> >::iterator it(v2);
        while (true) it++;
    }
    catch (const std::exception& e) {
        cout << "exception: " << e.what() << endl;
    }

}

void show_set_iterator()
{
    set<int> s;

    // old way
    associative_insert_iterator< set<int> > aint(s);

    *aint = 1;
    ++aint;
    *aint = 2;
    ++aint;
    *aint = 3;
    ++aint;

    // new way
    associative_inserter(s) = 4;
    associative_inserter(s) = 5;

    int vals[] = { 6,7,8,9 };
    copy(vals, vals + sizeof(vals) / sizeof(int), associative_inserter(s));
}

int main()
{

    show_iterator_traits();
    show_checked_iterator();
    show_set_iterator();
    show_custom_allocator();
    show_c_memory();

    return 0;
}
