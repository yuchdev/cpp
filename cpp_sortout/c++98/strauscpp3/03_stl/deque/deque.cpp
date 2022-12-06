#include <deque>

void show_vector_by_val()
{
    vector<int> v;
    size_t s = sizeof(v);
    v.push_back(1);
    // the vector size does not increase,
    // because all elements are in the dynamic memory
    s = sizeof(v);
    v.push_back(2);
    s = sizeof(v);
    v.push_back(3);
    v.push_back(4);
    s = sizeof(v);
    gimme_vector_by_val(v);
}

void gimme_vector_by_val(vector<int> v)
{
    size_t s = sizeof v;
    bool b = v.empty();
}

void show_deque()
{
    int arr[10] = { 1,6,3,5,7,6,4,2,4 };

    // Combines properties of a vector and a list
    // Exception guarantees are the same as vector
    deque<int> d;
    d.insert(d.begin(), arr, arr + sizeof(arr) / sizeof(int));
    int a = d.at(1);

}

