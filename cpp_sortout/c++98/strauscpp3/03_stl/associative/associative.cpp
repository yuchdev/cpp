#include <iostream>
#include <map>
#include <set>

// map
// It is convenient to define alias for map declarations
typedef map<int, string> is_map;
typedef map<int, string>::iterator is_map_it;

typedef map<string, int, no_case<string> > si_nocase_map;
typedef map<string, int, no_case<string> >::iterator si_nocase_map_it;

template <typename TMap, typename TKey, typename TVal>
void _insert_to_map(TMap& m1, const TKey& k, const TVal& s)
{

    // An associative array requires that for a key 
    // an operation "less than" exists.

    // The returned pair contains an iterator on the inserted string
    // and a logical flag indicating successful insertion
    pair< typename TMap::iterator, bool > p = m1.insert(make_pair(k, s));
    if (p.second == true) {
        cout << "Insert success" << endl;
    }
    else {
        cout << "Such key is already defined" << endl;
    }
}

void show_map()
{
    string s1("Vin Diesel");
    string s2("Chuck Norris");
    is_map m1;

    // map requires the `operator<()` defined for the element
    // unordered(hash) map requires `operator!=()`

    // map keys (and set elements) cannot be changed directly
    // you must remove the old one and insert a new one
    _insert_to_map<is_map, int, string>(m1, 1, s1);
    _insert_to_map<is_map, int, string>(m1, 1, s1);

    // Map with the no_case predicate
    si_nocase_map ncm1;
    _insert_to_map<si_nocase_map, string, int>(ncm1, s1, 1);
    _insert_to_map<si_nocase_map, string, int>(ncm1, "vin diesel", 2);
    _insert_to_map<si_nocase_map, string, int>(ncm1, "Chuck Norris", 3);
    _insert_to_map<si_nocase_map, string, int>(ncm1, "chuck norris", 4);

    pair<is_map_it, bool> p = m1.insert(make_pair(2, "Chuck Norris"));
    p = m1.insert(make_pair(2, "chuck norris"));

    // A sorting predicate can be passed as a constructor parameter
    // This allows us to set the predicate in runtime
    si_nocase_map pred;
    map<string, int, si_nocase_map> mm(pred);

    // When inserting an element, you can specify after which element it should be inserted
    // With large data volumes this can be proven useful
    m1.insert(p.first, make_pair(3, "Bruce Willis"));

    // iteration is performed in sorting order
    print_map(m1);

    // the map type is always pair<>
    // pair initializer allows type cast
    pair<char, int> p1('a', 1);
    pair<char, double> p2(p1);

    // index reference
    string s = m1[2];

    // When the key by operation [] can not be found,
    // create a new index with a default value
    s = m1[3];

    // create a new index by the value
    m1[4] = "Bruce Willis";

    // If an element of a pair does not have a default constructor,
    // it must always be initialized explicitly

    // iterator search by the key
    map<int, string>::iterator it = m1.find(2);

    // the first element with a key greater than 1 && less than 2
    it = m1.upper_bound(1);
    it = m1.lower_bound(2);

    // check if nothing is found
    if (it == m1.end()) {
        m1[2] = string("Chuck Norris");
    }

    //Items can be deleted
    // - one at a time
    m1.erase(m1.find(1));

    // - ranges [)
    m1.erase(m1.find(2), m1.find(3));
    // after erase() is called, the iterators pointing to them are invaludated

    size_t sz = m1.size();
    size_t msz = m1.max_size();
    // map and set provide a strong exception guarantee,
    // except for the insertion of a multitude elements (some of them could be insert, some are not)
}

// set
void show_set()
{
    // Container with unique elements
    // Can be represented as a map with keys only
    // You can use it as an analogue of unique()

    // Insert from a vector with repeating elements	
    set<string> st;

}

// multiset, multimap
void show_multimap()
{
    typedef multimap<int, string> is_mmap;
    typedef multimap<int, string>::iterator is_mmap_it;

    is_mmap m2;

    m2.insert(make_pair(1, "Chuck Norris"));
    m2.insert(make_pair(1, "Vin Diesel"));
    m2.insert(make_pair(2, ""));
    m2.insert(make_pair(2, "Jason Statham"));

    // First element with key 2
    is_mmap_it p1 = m2.lower_bound(2);

    // following the last element with key 1
    p1 = m2.upper_bound(1);

    // a pair of iterators wrapping all elements with key 2
    pair<is_mmap_it, is_mmap_it> pm = m2.equal_range(2);

    // number of elements with key 2
    size_t sz = m2.count(2);
}

void show_multiset()
{
    // A multitude of repetitive elements. Such a pile
    multiset<string> ms;
}
