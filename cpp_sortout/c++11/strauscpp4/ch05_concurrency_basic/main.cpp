#include <iostream>
#include <string>
#include <list>
#include <map>
#include <numeric>
#include <thread>
#include <mutex>
#include <future>
#include <condition_variable>
#include <chrono>
#include <algorithm>
#include <forward_list>
#include <iterator>
#include <regex>
#include <random>

using namespace std;

/*

New features:
* Resource management pointers: unique_ptr, shared_ptr, and weak_ptr (5.2.1)
* Concurrency support: thread (5.3.1), mutexes (5.3.4), locks (5.3.4), and condition variables (5.3.4.1)
* Higher-level concurrency support: packaged_thread, future, promise, and async()(5.3.5)
* tuples (5.4.3)
* Regular expressions: regex (5.5)
* Random numbers

1. unique_ptr is moved, shared_ptr is copied
2. Tasks and threads. Passing arguments and returning values.
3. Sharing data, managing deadlocks, performance compare
4. Conditional variable
5. future and promise
6. packaged_task
7. async() call
8. Time, type
9. iterator_traits
10. RegExp
11. Math&Random

*/

namespace cpp4 {

// 1. unique_ptr is moved, shared_ptr is copied
// see example

// 2. Tasks and threads.Passing arguments and returning values.
void thread_func1()
{
    for (int i = 0; i < 10000000; ++i);
}

void thread_func_param(int up_to) 
{
    for (int i = 0; i < up_to; ++i);
}

void thread_func_result(int up_to, int& res) 
{
    int i = 0;
    for (; i < up_to; ++i) {}
    res = i;
}


struct functor {
    void operator()() {}
};

} // namespace cpp4


void show_threads() 
{
    int res{};
    
    // plain function used as a thread
    std::thread t1{ cpp4::thread_func1 };
    t1.join();
    
    // We can easily pass data(or pointers or references to the data) as arguments
    // it uses a thread variadic template constructor 
    std::thread t2{ cpp4::thread_func_param, 10000000 };
    t2.join();

    // if return results by reference, pass it explicitly
    std::thread t3{ cpp4::thread_func_result, 10000000, std::ref(res) };
    t3.join();
    
    // using functor as a thread
    std::thread t4{ cpp4::functor() };
    t4.join();
}


namespace cpp4 {

// 3. Sharing data, managing deadlocks, performance compare
class share_data 
{
public:

    void single_lock() 
    {
        std::unique_lock<std::mutex> l{ m1_ };
        data_ = "single lock";
        std::cout << data_ << std::endl;
    }

    void multiple_lock() 
    {

        // preventing deadlock - postpone locking group of mutex
        std::unique_lock<std::mutex> l1{ m1_, defer_lock };
        std::unique_lock<std::mutex> l2{ m2_, defer_lock };
        std::unique_lock<std::mutex> l3{ m3_, defer_lock };

        // deadlock-safe lock (lock all or nothing)
        std::lock(l1, l2, l3);
        data_ = "multiple lock";
        std::cout << data_ << std::endl;
    }

private:
    std::string data_;
    std::mutex m1_;
    std::mutex m2_;
    std::mutex m3_;
};

void thread_func_share(share_data& d) 
{
    d.single_lock();
    d.multiple_lock();
}

} // namespace cpp4

void show_shared_data() 
{
    cpp4::share_data s;

    // using multiple mutexes
    std::thread t1{ cpp4::thread_func_share, std::ref(s) };
    std::thread t2{ cpp4::thread_func_share, std::ref(s) };

    t1.join();
    t2.join();
    // Locking and unlocking are relatively expensive operations. 
    // On the other hand, modern machines are very good at copying data, 
    // especially compact data, such as vector elements. 
    // So don't choose shared data for communication because of 'efficiency'
    // without thought and preferably not without measurement
}


namespace cpp4 {

// 4. Conditional variable
// In particular, it allows a thread to wait for some condition (often called an event)
class queue 
{
public:

    void enque(int i) 
    {
        std::unique_lock<std::mutex> l{ mutex_ };
        queue_.push_back(i);
        cond_.notify_one();
    }

    int deque() 
    {
        std::unique_lock<std::mutex> l{ mutex_ };
        while (queue_.empty())
            cond_.wait(l);
        int pop_me = queue_.front();
        queue_.pop_front();
        cond_.notify_one();
        return pop_me;
    }

private:
    std::list<int> queue_;
    std::condition_variable cond_;
    std::mutex mutex_;
};

} // namespace cpp4

void show_condition_variable() 
{
    cpp4::queue q;

    std::thread producer{ [&q]() {
         for (size_t i = 0; i < 1000; ++i) {
             q.enque(i);
         }
    } };

    std::thread concumer{ [&q]() {
        for (size_t i = 0; i < 1000; ++i) {
            std::cout << "Dequeue: " << q.deque() << std::endl;
        }
    } };

    producer.join();
    concumer.join();
}



namespace cpp4 {

// 5. future and promise
// 6. packaged_task
// 7. async() call
// The class template std::future provides a mechanism to access the result of asynchronous operations
// An asynchronous operation (created via std::async, std::packaged_task, or std::promise)
// can provide a std::future object to the creator of that asynchronous operation
// It may block if the asynchronous operation has not yet provided a value
template <typename Iterator>
void async_accumulate(Iterator first, Iterator last, std::promise<int> accumulate_promise) 
{
    int sum = std::accumulate(first, last, 0);
    accumulate_promise.set_value(sum);  // Notify future
}


} // namespace cpp4

void show_future_promise() 
{
    std::vector<int> v{ 1,2,3,4,5,6,7,8,9 };

    // future+promise
    std::promise<int> pr;

    // future can get from promise (as well as async and packaged_task)
    std::future<int> ft = pr.get_future();

    // perform computation
    using Iter = std::vector<int>::const_iterator;
    std::thread wrk{ cpp4::async_accumulate<Iter>, v.begin(), v.end(), std::move(pr) };

    // wait until computation finish
    ft.wait();

    // get result
    std::cout << "result = " << ft.get() << std::endl;

    // wait until thread finish
    wrk.join();
}



void show_future() 
{
    // future created via std::async, std::packaged_task, or std::promise

    // 1. packaged_task (only returns value)
    // A packaged_task provides wrapper code to put the return value 
    // or exception from the task into a promise
    std::packaged_task<int()> task{ []() {return 42; } };
    std::future<int> f1 = task.get_future();
    
    // post task to detached thread
    // The move() operations are needed because a packaged_task cannot be copied
    std::thread(std::move(task)).detach();
    f1.wait();
    std::cout << "result = " << f1.get() << std::endl;

    // 2. async
    // Treat a task as a function that may happen to run concurrently with other tasks
    std::future<int> f2 = std::async(std::launch::async, []() { return 8; });
    // TODO: more real example std::async

    // Basically, async() separates the 'call part' of a function call from the 'get the result' part
    // Using async(), you don't have to think about threads and locks

    f2.wait();
    std::cout << "result = " << f2.get() << std::endl;

}


// 8. Time, type
// see examples
void show_time()
{
    // it is usually a good idea to convert a duration into a known unit
    using namespace std::chrono;

    auto t0 = high_resolution_clock::now();
    cout << "Measure how long the string out" << std::endl;
    auto t1 = high_resolution_clock::now();

    cout << duration_cast<milliseconds>(t1-t0).count() << " msec" << std::endl;
}


void show_type_functions() 
{
    // Limits info
    constexpr float min_float = std::numeric_limits<float>::min();
    std::cout << "Min float = " << min_float << std::endl;

    std::cout << "is_arithmetic<int>() = " << is_arithmetic<int>() << std::endl;
    std::cout << "is_pod<string>() = " << is_pod<string>() << std::endl;
    std::cout << "has_virtual_destructor<vector<int>>() = " << has_virtual_destructor<vector<int>>() << std::endl;
}


namespace cpp4 {

// 9. iterator_traits

// How iterator tags work
// These 2 functions are dispatched by iterator_tag
template <typename RndIter>
void sort_helper_(RndIter begin, RndIter end, random_access_iterator_tag) 
{
    std::sort(begin, end);
}

template <typename RndIter>
void sort_helper_(RndIter begin, RndIter end, forward_iterator_tag) 
{
    using ElementType = std::iterator_traits<RndIter>::value_type;
    std::vector<ElementType> v{ begin, end };
    std::sort(v.begin(), v.end());
    std::copy(v.begin(), v.end(), begin);
}

template <typename Container>
void sort_helper(Container& c) 
{
    // fetch iterator type from container
    using IteratorType = typename Container::iterator;
    // fetch iterator tag from iterator
    using IteratorTag = std::iterator_traits<IteratorType>::iterator_category;
    IteratorTag t;
    sort_helper_(c.begin(), c.end(), t);
}

} // namespace cpp4


void show_iterator_traits() 
{
    std::vector<int> v{ 9,8,7,6,5,4,3,2,1 };
    std::forward_list<int> f{ 9,8,7,6,5,4,3,2,1 };

    cpp4::sort_helper(v);
    cpp4::sort_helper(f);
}


// 10. RegExp
// see example
void show_regexp() 
{
    // To express the pattern, I use a raw string literal
    auto r0 = R"(\d{3})";

    std::smatch base_match;

    std::regex re{r0};

    std::string fnames[] = { "999", "666", "11", "xxx" };

    for (auto fname : fnames) {
        bool b = std::regex_match(fname, base_match, re);
        std::cout << r0 << " for " << fname << " = " << b << endl;
    }
}


namespace cpp4 {

// 11. Math&Random
template <typename T, typename Distribution>
class random_generator {
public:
    random_generator(T low, T high) : dist_{ low, high } {}
    int operator()() { return dist_(engine_); }
private:
    std::default_random_engine engine_;
    Distribution dist_;
};

template <typename Generator>
void print_distribution(Generator& g) {
    
    std::map<int, size_t> distr;

    for (size_t i = 0; i < 200; ++i) {
        ++distr[g()];
    }

    // write out a bar graph cout << i << '\t';
    for (auto& mypair : distr) {
        for (int j = 0; j != mypair.second; ++j) cout << '*';
        cout << endl;
    }
}

}

void show_random() 
{
    std::cout << "Linear distribution" << std::endl;
    cpp4::random_generator<int, std::uniform_int_distribution<int>> gen_uniform{ 0, 5 };
    cpp4::print_distribution(gen_uniform);

    std::cout << "Normal distribution" << std::endl;
    cpp4::random_generator<double, std::normal_distribution<double>> gen_normal{ 0.0, 1.0 };
    cpp4::print_distribution(gen_normal);
}

int main() 
{
    show_threads();
    show_shared_data();
    show_condition_variable();
    show_future_promise();
    show_future();
    show_time();
    show_type_functions();
    show_iterator_traits();
    show_regexp();
    show_random();

    return 0;
}
