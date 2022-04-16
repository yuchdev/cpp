#include "structured_exception.h"

#if defined(_WIN32) || defined(_WIN64)

void structured_exception::enableStructuredExceptions() NOTHROW {
    _set_se_translator(structured_exception::handlerStructuredException);
}

void structured_exception::handlerStructuredException(unsigned int, EXCEPTION_POINTERS* ep) {
    switch(ep->ExceptionRecord->ExceptionCode)	
    {	
    case EXCEPTION_ACCESS_VIOLATION:
        if(ep->ExceptionRecord->ExceptionInformation[0] == 0){
            throw structured_exception("Memory Access Violation: The thread tried to read from a virtual address that it does not have appropriate access to.");		
        }		
        else {
            throw structured_exception("Memory Access Violation: The thread tried to write to a virtual address that it does not have appropriate access to.");
        }		
        break;	
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        throw structured_exception("Int Divide By Zero: The thread tried to divide an integer value by an integer divisor of zero.");
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
	case STATUS_FLOAT_MULTIPLE_TRAPS:
		// When generating SSE code you sometimes see STATUS_FLOAT_MULTIPLE_TRAPS 
		// instead of STATUS_FLOAT_DIVIDE_BY_ZERO. 
		// This is slightly less helpful, but the root cause should be straightforward to determine.
        throw structured_exception("Float Divide By Zero: The thread tried to divide a floating-point value by a floating-point divisor of zero.");
    default:
        throw structured_exception("Unknown Windows Structured Exception");	
    }		
} 

structured_exception::structured_exception(const char* const strMessage) NOTHROW : std::runtime_error(strMessage){}

structured_exception::structured_exception(const structured_exception& rhs) NOTHROW : std::runtime_error(rhs){}

structured_exception::~structured_exception() NOTHROW{}

structured_exception& structured_exception::operator = (const structured_exception& rhs) NOTHROW {	
    if(this == &rhs)	{		
        return *this;	
    }	
    std::runtime_error::operator = (rhs);
    return *this;
}
#endif
