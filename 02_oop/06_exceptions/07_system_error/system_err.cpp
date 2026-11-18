#include <cassert>
#include <clocale>
#include <cstdio>
#include <exception>
#include <future>
#include <iostream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

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
1. Error propagation basic -- exception_ptr preserves the ORIGINAL
   dynamic type across the capture boundary, not just "some exception"
2. Error propagation in multithreading -- same proof, across a
   std::promise/std::future pair
3. packaged task propagation -- same proof again, via std::packaged_task
4. system_error example with file
5. system_error + error_category -- including actually exercising the
   custom http_error category end to end (constructed below but never
   once instantiated by the original version of this example)
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

// std::exception_ptr is a nullable pointer-like type that manages an
// exception object which has been thrown and captured with
// std::current_exception (std::make_exception_ptr builds one directly
// from an exception object instead, without an active throw/catch).
std::exception_ptr capture_out_of_range()
{
    std::exception_ptr ex_ptr;

    try {
        std::string s;
        const char c = s.at(1); // throw here
        (void)c;
    }
    catch (...) {
        // hold thrown exception in ptr
        ex_ptr = std::current_exception();
    }

    return ex_ptr;
}

} // namespace cpp

void show_exception_propagation()
{
    std::exception_ptr ex = cpp::capture_out_of_range();
    assert(ex != nullptr);

    bool caught_as_out_of_range = false;
    try {
        cpp::propagate_exception(ex);
    }
    // Proof, not just a comment: exception_ptr/rethrow_exception carry
    // the exception's ORIGINAL dynamic type through the capture, not
    // merely "some std::exception" -- catching only the base here
    // would not have distinguished the two.
    catch (const std::out_of_range& e) {
        caught_as_out_of_range = true;
        std::cout << e.what() << std::endl;
    }
    catch (const std::exception&) {
    }
    assert(caught_as_out_of_range);
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

    // instead of future current exception is thrown -- and, same proof
    // as above, the exact out_of_range type survives the trip through
    // promise::set_exception()/future::get(), across the thread boundary
    bool caught_as_out_of_range = false;
    try {
        auto f = cpp::global_promise.get_future();
        int result = f.get();
        (void)result;
    }
    catch (const std::out_of_range& e) {
        caught_as_out_of_range = true;
        std::cout << e.what() << std::endl;
    }
    catch (const std::exception&) {
    }

    t.join();
    assert(caught_as_out_of_range);
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

    bool caught_as_out_of_range = false;
    try {
        // process exception from get()
        int res = f.get();
        (void)res;
    }
    catch (const std::out_of_range& e) {
        caught_as_out_of_range = true;
        std::cout << e.what() << std::endl;
    }
    catch (const std::exception&) {
    }
    t.join();
    assert(caught_as_out_of_range);
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
    bool caught = false;
    try {
        cpp::call_system_specific();
    }
    catch (const std::system_error& e) {
        caught = true;
        std::cout << "code(): " << e.code() << std::endl;
        std::cout << "what(): " << e.what() << std::endl;
        std::cout << "code().value(): " << e.code().value() << std::endl;
        std::cout << "code().message(): " << e.code().message() << std::endl;

        // EIO's numeric value and the fact it's std::system_category()
        // are both portable to assert on POSIX; the human-readable
        // message() text is not (it's libc-specific wording), so that
        // part stays a printed observation rather than an assert.
        assert(e.code().value() == EIO);
        assert(e.code().category() == std::system_category());
    }
    assert(caught);
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
    std::error_condition default_error_condition(int err) const noexcept override
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
    // This equivalence check does work: std::system_category()'s own
    // default_error_condition() maps the platform EACCES/ERROR_ACCESS_DENIED
    // value it was assigned to std::errc::permission_denied, so the
    // comparison below succeeds.
    bool matched = (ec == std::errc::permission_denied);
    assert(matched);
    if (matched) {
        std::cout << "error code:" << ec.default_error_condition().message() << std::endl;
    }
}

// The http_category_impl/http_error machinery above is fully built but
// the original version of this example never actually constructed a
// single std::error_code FROM it -- theory (a custom error_category)
// with no code exercising it at all. This closes that gap end to end:
// the is_error_code_enum<http_error> specialization, the implicit
// conversion it enables, the category's own name()/message(), and the
// default_error_condition() mapping onto std::errc.
void show_custom_http_error_category()
{
    // is_error_code_enum<http_error> is what makes this implicit
    // conversion compile at all -- http_error now behaves as a
    // first-class error_code source, the same relationship std::errc
    // has with std::generic_category().
    std::error_code forbidden = cpp::http_error::forbidden;

    assert(std::string(forbidden.category().name()) == "http");
    assert(forbidden.value() == static_cast<int>(cpp::http_error::forbidden));
    assert(forbidden.message() == "Forbidden");

    // default_error_condition() is where http_error::forbidden claims
    // equivalence with the portable std::errc::permission_denied --
    // this is the mapping the category exists to provide, not merely
    // code that happens to compile.
    assert(forbidden == std::errc::permission_denied);

    std::error_code gateway_timeout = cpp::http_error::gateway_timeout;
    // gateway_timeout has no explicit case in default_error_condition(),
    // so it falls through to the default branch: a condition under
    // http_category() itself, equivalent to nothing in std::errc.
    assert(!(gateway_timeout == std::errc::permission_denied));
    assert(gateway_timeout.default_error_condition().category() == cpp::http_category());

    std::cout << "http forbidden: " << forbidden.message()
               << " (equivalent to " << forbidden.default_error_condition().message() << ")"
               << std::endl;
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
    bool matched = (ec == std::errc::file_exists);
    assert(matched);
    if (matched) {
        std::cout << "error code:" << ec.default_error_condition().message() << std::endl;
    }
}


int main()
{

    // correct system messages
    std::setlocale(LC_ALL, "");

    // propagation
    show_exception_propagation();
    show_multithreading_propagation();
    show_packaged_task_propagation();

    // system_error
    show_system_error();
    show_custom_error_condition();
    show_custom_http_error_category();
    show_error_condition();

    return 0;
}
