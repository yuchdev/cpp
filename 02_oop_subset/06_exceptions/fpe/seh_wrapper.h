#ifndef	DEFINE_STRUCTREDEXCEPTION_H
#define	DEFINE_STRUCTREDEXCEPTION_H

#if defined(_WIN32) || defined(_WIN64)

#include <exception>
#include <stdexcept>

// SEH declaration here
#include <windows.h>

// This header describes the class that "wraps" the SEH in C++ exceptions

// C++11 noexcept keyword
#define NOTHROW noexcept

// If you want your C++ exception code to catch SEH exceptions, 
// you need to build the code with /EHa.

/***	A Microsoft Windows Strucutred Exception.*/
class structured_exception : public std::runtime_error
{
public:
    /**	*	Default Constructor	*/
    structured_exception(const char* const strMessage) NOTHROW;

    /**	*	Copy Constructor	*/
    structured_exception(const structured_exception& rhs) NOTHROW;

    /**	*	Destructor	*/
    virtual ~structured_exception() NOTHROW;

    /**	*	Assignment operator	*/
    structured_exception& operator = (const structured_exception& rhs) NOTHROW;

    /**	*	Enables the class to convert Windows Structured Exception. 	*/
    static void enableStructuredExceptions() NOTHROW;

private:
    static void handlerStructuredException(unsigned int, EXCEPTION_POINTERS*);
};

// You can expand your own hierarchy, e.g.
// class access_violation : public structured_exception
// class divide_by_zero : public structured_exception
// etc.

#endif // defined(_WIN32) || defined(_WIN64)

#endif	//	DEFINE_STRUCTREDEXCEPTION_H//
