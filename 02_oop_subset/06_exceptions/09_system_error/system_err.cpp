#include <iostream>
#include <exception>
#include <stdexcept>
#include <string>
#include <thread>
#include <future>
#include <system_error>
#include <cstdio>

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#else
#include <errno.h>
#endif

using namespace std;

/*

New features:
* Copying and rethrowing exceptions (30.4.1.2)
* Error reporting using error codes: system_error (30.4.3)

Examples:
1. Error propagation basic
2. Error propagation in multithreading
3. packaged task propagation
4. system_error example with file
5. system_error + error_category
6. system_error + error_condition
*/

// 1. Error propagation basic
namespace cpp
{


// pass exception_ptr by value
void propagate_exception(std::exception_ptr ex)
{

    // check whether exception is exists
    if (ex) {
        std::rethrow_exception(ex);
    }
}

void wrap_exception()
{
    // std::exception_ptr is a nullable pointer-like type that manages an exception object 
    // which has been thrown and captured with std::current_exception
    std::exception_ptr ex_ptr;

    // std::make_exception_ptr also creates ritht from exception object

    try {
        std::string s;
        const char c = s.at(1); // throw here
    }
    catch (...) {
        // hold thrown exception in ptr
        ex_ptr = std::current_exception();
    }

    cpp::propagate_exception(ex_ptr);
}

} // namespace cpp

void show_exception_propagation()
{

    try {
        cpp::wrap_exception();
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}


// 2. Error propagation in multithreading
namespace cpp
{

// bad example
std::promise<int> global_promise;

void thread_propagate_exception()
{

    std::vector<int> v;
    int hhh {};

    try {
        hhh = v.at(1);
        global_promise.set_value(hhh);
    }
    catch (...) {
        std::exception_ptr ex_ptr = std::current_exception();
        global_promise.set_exception(ex_ptr);  // does set_exception may throw?
    }
}

int thread_throw_exception()
{
    std::vector<int> v;
    int hhh {};

    hhh = v.at(1);
    return hhh;
}

} // namespace cpp


void show_multithreading_propagation()
{

    std::thread t { cpp::thread_propagate_exception };

    // instead of future current exception is thrown
    try {
        auto f = cpp::global_promise.get_future();
        int result = f.get();
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    t.join();
}

// 3. packaged task propagation
void show_packaged_task_propagation()
{

    // An std::packaged_task has an associated std::future object that will hold the exception(or the result of the task)
    // You can retrieve that future by calling the get_future() member function of std::packaged_task.

    // This means that it is enough to throw an exception inside the function associated 
    // with the packaged task in order for that exception to be caught by the task's future
    std::packaged_task<int()> task { cpp::thread_throw_exception };

    std::future<int> f = task.get_future();

    std::thread t { std::move(task) };

    try {
        // process exception from get()
        int res = f.get();
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    t.join();
}


//4. system_error example with file
namespace cpp
{

void call_system_specific()
{
    FILE* pf = fopen("myfile.txt", "r");
    if (!pf) {
        // in general value(int or enum) + category (string or class)
        throw std::system_error(EIO, std::system_category());
    }
    else {
        // ... do smth
    }
}

} // namespace cpp
void show_system_error()
{
    try {
        cpp::call_system_specific();
    }
    catch (const std::system_error& e) {
        std::cout << "code(): " << e.code() << std::endl;
        std::cout << "what(): " << e.what() << std::endl;
        std::cout << "code().value(): " << e.code().value() << std::endl;
        std::cout << "code().message(): " << e.code().message() << std::endl;
    }
}


//5. system_error + error_category
// defining own error_category
namespace cpp
{

// let we have errors for HTTP protocol
enum class http_error
{
    continue_request = 100,
    switching_protocols = 101,
    ok = 200,
    // other codes...
    forbidden = 403,
    gateway_timeout = 504,
    version_not_supported = 505
};

// An error_code object consists of both an error value and a category
// The error category determines whether a value of 100 means http_error::continue_request, 
// mapped to std::errc::network_down (ENETDOWN on Linux), or something else entirely
class http_category_impl : public std::error_category
{
public:

    // need overload these functions:

    // string name of category
    virtual const char* name() const noexcept override
    {
        return "http";
    }

    // pretty-print message
    virtual std::string message(int err) const override
    {
        cpp::http_error ev { static_cast<cpp::http_error>(err) };
        switch (ev) {
        case cpp::http_error::continue_request:
            return "Continue";
        case http_error::switching_protocols:
            return "Switching protocols";
        case http_error::ok:
            return "OK";
            // ...
        case http_error::forbidden:
            return "Forbidden";
        case http_error::gateway_timeout:
            return "Gateway time-out";
        case http_error::version_not_supported:
            return "HTTP version not supported";
        default:
            return "Unknown HTTP error";
        }
    }

    // The error_category::default_error_condition() virtual function lets you define
    // an error_condition that is equivalent to a given error code
    std::error_condition http_category_impl::default_error_condition(int err) const noexcept override
    {

        cpp::http_error ev { static_cast<cpp::http_error>(err) };
        switch (ev) {
        case http_error::forbidden:
            return std::errc::permission_denied;
        default:
            return std::error_condition(err, *this);
        }
    }

};

// This function must always return a reference to the same object
const std::error_category& http_category()
{
    static cpp::http_category_impl cat;
    return cat;
}

// construct system-dependent an error_code from the enum
std::error_code make_error_code(cpp::http_error e)
{

    // create from system-dependent int code + category 
    // in our case int code is really independent as HTTP works the same way everywhere
    return std::error_code { static_cast<int>(e), http_category() };
}

// you should also provide the equivalent function for construction of an error_condition
std::error_condition make_error_condition(cpp::http_error e)
{
    return std::error_condition { static_cast<int>(e), http_category() };
}

// some test function for custom error condition demo
void some_http_handler(std::error_code& ec)
{

    // let we always return 403
    ec.assign(static_cast<int>(std::errc::permission_denied), std::system_category());
}

} // namespace cpp

// For the http_error enumerators to be usable as error_code constants, 
// enable the conversion constructor using the is_error_code_enum type trait
namespace std
{

template <>
struct is_error_code_enum<cpp::http_error> : public true_type {};

}

void show_custom_error_condition()
{

    std::error_code ec;
    cpp::some_http_handler(ec);
    // TODO: does not work?
    if (ec == std::errc::permission_denied) {
        std::cout << "error code:" << ec.default_error_condition().message() << std::endl;
    }
}

//6. system_error + error_condition
namespace cpp
{

// error_code object will contain the OS-specific error code
// This follows the tradition (used by errno and GetLastError) of having 0 indicate success and non-zero indicate failure

// If you're only interested in whether the operation succeeded or failed, you can use the fact that error_code is convertible-to-bool
// If you're calling error_code::value() then you're doing it wrong - it is system-specific!
// you should be comparing your OS - specific error code ec against an error_condition object

/*
// Example:
std::error_code ec;
create_directory("/some/path", ec);
if (ec == std::errc::file_exists)

// This works because the library implementor has defined the equivalence between the
sytem-specific error codes EEXIST or ERROR_ALREADY_EXISTS and the error condition std::errc::file_exists

*/

// function stub
// ec is system-dependent error code value
void create_directory(const std::string& path, std::error_code& ec)
{
    // some stuff...
    // let is be portable implementation and directory already exists

#if defined(_WIN32) || defined(_WIN64)
    ec.assign(ERROR_ALREADY_EXISTS, std::system_category());
#else
    ec.assign(EEXIST, std::system_category());
#endif

}

} // namespace cpp

void show_error_condition()
{
    std::error_code ec {};
    cpp::create_directory("/path/not_exists", ec);

    // If you're calling error_code::value() then you're doing it wrong
    //if (ec.value() == EEXIST) // No! system-specific

    // right way - check error_code against error_condition - checks for equivalence
    // actually call 
    // operator==(const error_code&, const error_condition&)
    // Step 1: determine whether the enum is an error code or condition
    // Step 2: associate the value with an error category
    // Step 3: implicitly converts from errc to error_condition
    if (ec == std::errc::file_exists) {
        std::cout << "error code:" << ec.default_error_condition().message() << std::endl;
    }
}


int main()
{

    // correct system messages
    setlocale(0, "");

    // propagation
    show_exception_propagation();
    show_multithreading_propagation();
    show_packaged_task_propagation();

    // system_error
    show_system_error();
    show_custom_error_condition();
    show_error_condition();

    return 0;
}
