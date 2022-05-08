#include <iostream>
#include <thread>
#include <cassert>
#include <csignal>
#include <atomic>

using namespace std;

/*

New features:

Examples:
1. Instruction Reordering (41.2.2)
2. Fences
3. volatile
*/

// 1. Instruction Reordering (41.2.2)
struct th1 
{
    int x;
    bool x_init;
    
    // For this piece of code there is no stated reason to assign to x before assigning to x_init
    // The optimizer(or the hardware instruction scheduler) may decide to speed up the program by executing x_init = true first
    void operator()() 
    {
        // no use of x_init in initialize() x_init = true;
        x = initialize(); 
        x_init = true;
    }

    int initialize()
    {
        return 42;
    }
};

struct th2
{
    th1& th1_ref;
    th2(th1& t1) :th1_ref(t1) {}

    void operator()()
    {

        int y;
        // Now we have a problem: thread 2 may never wait and thus will assign an uninitialized x to y
        while (!th1_ref.x_init)
            std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
        y = th1_ref.x;
    }
};


void show_instructions_reordering()
{
    th1 t1_;
    th2 t2_(t1_);

    std::thread t1(t1_);
    std::thread t2(t2_);

    t1.join();
    t2.join();
}

// 2. Fences
std::atomic<int> a = 0;
std::atomic<int> b = 0;

extern "C" void handler(int) {
    if (1 == a.load(std::memory_order_relaxed)) {
        std::atomic_signal_fence(std::memory_order_acquire);
        assert(1 == b.load(std::memory_order_relaxed));
    }

    std::exit(0);
}

void show_fences()
{
    // atomic_signal_fence only perform synchronization between a signal handler and other code running on the same thread
    // That means that it does not perform synchronization between two different threads
    // (i.e. between code and signal handler code, which is async, but still in the same thread)
    std::signal(SIGTERM, &handler);

    b.store(1, std::memory_order_relaxed);
    std::atomic_signal_fence(std::memory_order_release);
    a.store(1, std::memory_order_relaxed);
}

// 3. volatile
void show_volatile() 
{
    volatile long clock_register = 0; // updated by the hardware clock

    // A volatile specifier basically tells the compiler not to optimize away apparently redundant reads and
    // writes. For example:
    auto t1{ clock_register };
    // ... no use of clock_register here ... 
    auto t2 {clock_register};

    // Had clock_register not been volatile, the compiler would have been perfectly entitled to eliminate
    // one of the reads and assume t1 == t2
}

int main() {

    show_instructions_reordering();
    show_fences();
    show_volatile();

    return 0;
}
