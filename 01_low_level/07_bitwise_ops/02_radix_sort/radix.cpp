#include <utilities/elapsed.h>

#include <iostream>

/// Radix sort of integer vector
void radix_sort(const std::vector<int>& v)
{
    std::vector<int> result;
    std::vector<int> buckets[10];

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < v.size(); ++j)
        {
            int bucket_index = (v[j] / static_cast<int>(std::pow(10, i))) % 10;
            buckets[bucket_index].push_back(v[j]);
        }

        for (int j = 0; j < 10; ++j)
        {
            result.insert(result.end(), buckets[j].begin(), buckets[j].end());
            buckets[j].clear();
        }
    }
}


int main()
{
    return 0;
}


