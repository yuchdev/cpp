#include <iostream>
#include <string>
#include <forward_list>
#include <unordered_map>

using namespace std;

/*

New features:
* A singly-linked list: forward_list (4.4.5, 31.4.2)
* Hash containers:unordered_map,unordered_multimap,unordered_set,andunordered_multiset (4.4.5, 31.4.3)
*/

namespace cpp4
{

struct Entry
{
    string name;
    string number;
};

ostream& operator<<(ostream& os, const Entry& e)
{
    return os << '{' << e.name << ':' << e.number << '}';
}

}

void show_forward_list()
{
    std::forward_list<cpp4::Entry> l {
        {"German Skotobaza", "666-666"},
        {"Gelmut Komatoza", "999-999"}
    };

    for (cpp4::Entry& e : l) {
        std::cout << e << std::endl;
    }

}

// http://en.wikipedia.org/wiki/Hash_table
// http://codeforces.ru/blog/entry/4710

typedef std::unordered_map<string, string> stringmap;

stringmap merge(stringmap a, stringmap b)
{
    stringmap temp(a);
    temp.insert(b.begin(), b.end());
    return temp;
}

void show_hash()
{
    // For big maps hash-based containers are faster up to 10 times
    // in case of correctly selected hash-function

    // map requires >, hash requires !=
    // tightly filled tables are slower, so hash-table is able to grow

    // good hash-function must be profiled

    // unordered_map containers are faster than map containers to access 
    // individual elements by their key, although they are generally less efficient 
    // for range iteration through a subset of their elements.

    // Iterators in the container are at least forward iterators.

    // C++ has std::hash standard function in <functional>

    // empty
    stringmap first;

    // init list
    stringmap second({
        { "apple", "red" },
        { "lemon", "yellow" }
        });

    // init list
    stringmap third({
        { "orange", "orange" },
        { "strawberry", "red" }
        });

    // copy
    stringmap fourth(second);

    // move
    stringmap fifth(merge(third, fourth));

    // range init (slow)
    stringmap sixth(fifth.begin(), fifth.end());

    // directly insert (emplace) to map
    sixth.emplace("banana", "yellow");
}

int main()
{

    show_forward_list();
    show_hash();

    return 0;
}
