#include <iostream>
#include <thread>
#include <mutex>
#include <future>
#include <chrono>

using namespace std;

/*

New features:
* Thread-local storage: thread_local (42.2.8)
* Concurrency support: thread (5.3.1, 42.2), mutexes (5.3.4, 42.3.1), locks (5.3.4, 42.3.2), and condition variables (5.3.4.1, 42.3.4)
* Higher-levelconcurrency support: packaged_thread, future, promise, and async() (5.3.5, 42.4)

Examples:
1. Native handles (tread, mutex etc)
2. Thread ID
3. Mutes std::system_error (42.3.1.2)
4. Future wait for all/for any

*/

//1. Native handles(tread, mutex etc)
//2. Thread ID
void show_threads()
{
    std::thread t([] () { return 42; });

    // could be mixed with POSIX/WinAPI
    std::thread::native_handle_type handle = t.native_handle();

    // ID
    if (t.get_id() == std::thread::id {}) {
        std::cout << "t not joinable\n";
    }
    else {
        std::cout << "t ID = " << t.get_id() << '\n';
    }
    t.join();
}

//3. Mutes std::system_error(42.3.1.2)
void show_mutex_error()
{
    std::mutex mtx;
    try {
        mtx.lock();
        mtx.lock(); // try to lock a second time 
    }
    catch (system_error& e) {
        mtx.unlock();
        cout << e.what() << '\n';
        cout << e.code() << '\n';
    }
}

//4. Future wait for all / for any
// I can easily implement a version of wait_for_all()
template<typename T>
vector<T> wait_for_all(vector<future<T>>& vf)
{
    vector<T> res;
    for (auto& fu : vf) {
        res.push_back(fu.get());
    }
    return res;
}

// return index of ready future
// if no future is ready, wait for d before trying again 
template<typename T>
int wait_for_any(vector<future<T>>& vf, std::chrono::steady_clock::duration d)
{
    while (true) {
        for (int i = 0; i != vf.size(); ++i) {

            if (!vf[i].valid())
                continue;

            switch (vf[i].wait_for(seconds { 0 })) {
            case future_status::ready:
                return i;
            case future_status::timeout:
                break;
            case future_status::deferred:
                // I decided to consider a deferred task (42.4.6) an error for my uses
                // (i.e. not async, just calculate on get())
                throw runtime_error("wait_for_all(): deferred future");
            }
        }
        this_thread::sleep_for(d);
    }
}

int main()
{
    show_threads();
    show_mutex_error();
    return 0;
}
