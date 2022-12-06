#include <list>

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

template<typename TKey, typename TVal>
void print_map(const map<TKey, TVal>& m)
{
    typename map<TKey, TVal>::const_iterator it = m.begin();
    while (it != m.end()) {
        cout << (*it).first << "=/t" << (*it).second;
        ++it;
    }
    cout << endl;
}

// list has no random iterators, as well as reserve() and capacity()
void show_list()
{
    int arr[10] = { 1,6,3,5,7,6,4,2,4 };
    vector<int> vi1(10, 1);

    list<int> l;
    list<int> l1;
    l.assign(arr, arr + sizeof(arr) / sizeof(int));
    l1.assign(vi1.begin(), vi1.end());
    print_container(l);
    print_container(l1);

    // Use the built-in `list::sort` function instead of the algorithm
    // since the algorithm requires random iterators
    l.sort();
    print_container(l);

    list<string> ls;
    list<string> ug;
    ls.push_back("Vin Diesel");
    ls.push_back("Chuck Norris");
    ls.push_back("Jason Statham");
    ug.push_back("Bruce Willis");
    print_container(ls);

    // Operation "delete and paste"
    // Can accept data from lists
    // Cheap at a constant complexity, no copying of elements
    // With the splice function iterators must point to the same list and be valid

    // where to insert, what list to insert
    ls.splice(ls.begin(), ug);
    print_container(ls);

    // where to insert, from which list, which element
    ug.splice(ug.begin(), ls, ls.begin());
    print_container(ug);

    // where to insert, from which list, from which to which element 
    // (find the first element with the first letter 'c' and insert from the beginning to this element)
    list<string>::iterator p = find_if(ls.begin(), ls.end(), initial('c'));
    ug.splice(ug.begin(), ls, ls.begin(), p);
    print_container(ug);

    // Error because not a random iterator
    // ug.splice(ug.begin(), ls, ls.begin(), ls.begin()+2 );

    // Operations with front element
    ug.push_front("gotyachke");

    // if changing containers is planned,
    // it is better to prefer operations from the end,
    // then the list can be replaced, say, by a vector

    // exchanging lists is a cheap operation without copying
    ug.swap(ls);

    // merge sorted lists with preserving order 
    l.merge(l1);

    // merging unsorted lists cause debug assertion
    l1.assign(10, 2);
    list<int> l3(arr, arr + sizeof(arr) / sizeof(int));
    // l1.merge(l3);

    // Use built-in deletion
    l.remove(1);

    // delete by predicate
    l.remove_if(is_odd<int>());

    ls.push_back("chuck norris");
    ls.push_back("Chuck Norris");
    ls.push_back("Bruce Willis");

    // Sorting by predicate no_case
    ls.sort(no_case<string>());

    // remove consecutive doubles,
    // satisfying the predicate
    ls.unique(initial2('o'));
    ls.unique(initial2('e'));

    // delete all consecutive doubles
    l.unique();

    // reverse the list
    ls.reverse();

    // 
    string first = ls.front();
    string last = ls.back();

    // Lists are the most reliable in terms of exceptions,
    // only sort() provides a basic guarantee,
    // all other operations offers a strong one.
    // Predicates must not throw exceptions.	
    // List iterators are always valid.
}
