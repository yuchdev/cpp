#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
#include <string>
#include <iostream>
#include <cstdarg>

#if defined __GNUC__
#define cdecl
#define stdcall
#define fastcall
#define cdecl_gcc __attribute__((cdecl))
#define stdcall_gcc __attribute__((stdcall))
#define fastcall_gcc __attribute__((fastcall))
#else
#define cdecl __cdecl
#define stdcall __stdcall
#define fastcall __fastcall
#define cdecl_gcc
#define stdcall_gcc
#define fastcall_gcc
#endif


// __cdecl is standard C++ calling convention
// Arguments are being passed through stack, left to right
// the stack is cleaned up by the caller
// it can do vararg functions
int cdecl cdecl_func(int a, long b) cdecl_gcc
{
    // Save base register into stack
    // 00411440  push        ebp

    // Save surrent top of the stack
    // 00411441  mov         ebp,esp

    // Set the stack frame for local variables
    // Size of the frame is 0xC0h, probably for debugging purposes
    //00411443  sub         esp,0C0h

    // Put registers into stack
    // 00411449  push        ebx
    // 0041144A  push        esi
    // 0041144B  push        edi

    // Some kind of init
    //0041144C  lea         edi,[ebp-0C0h]
    //00411452  mov         ecx,30h
    //00411457  mov         eax,0CCCCCCCCh
    //0041145C  rep stos    dword ptr es:[edi]

    // Performing addition
    // 0041145E  mov         eax,dword ptr [a]
    // 00411461  add         eax,dword ptr [b]
    return a + static_cast<int>(b);

    // Pop registers from the stack in backward order
    //00411464  pop         edi
    //00411465  pop         esi
    //00411466  pop         ebx

    // Put the old stack pointer in place
    // pop the base register and leave
    //00411467  mov         esp,ebp
    //00411469  pop         ebp

    // Stack will be cleaned up by the caller
    // 0041146A  ret
}


// Calling convention used in WINAPI in __stdcall
// Arguments are being passed through stack, left to right
// the stack is cleaned up by the called function
int stdcall stdcall_func(int a, long* b) stdcall_gcc
{
    // Save base register into stack
    //  push        ebp

    // Save surrent top of the stack
    //  mov         ebp,esp

    // Set the stack frame for local variables
    // Size of the frame is 0xC0h, probably for debugging purposes
    //  sub         esp,0C0h

    // Put registers into stack
    //  push        ebx
    //  push        esi
    //  push        edi

    // Some kind of init
    //  lea         edi,[ebp-0C0h]
    //  mov         ecx,30h
    //  mov         eax,0CCCCCCCCh
    //  rep stos    dword ptr es:[edi]

    // Perform assignment
    //	mov         eax,dword ptr [b]
    //	mov         ecx,dword ptr [a]
    //	mov         dword ptr [eax],ecx
    *b = static_cast<long>(a);

    return 0;
    //  xor         eax,eax

    // Pop registers from the stack in backward order
    //	pop         edi
    //	pop         esi
    //	pop         ebx

    // Put the old stack pointer in place
    // pop the base register and leave
    //	mov         esp,ebp
    //	pop         ebp

    // Before exit release 8 bytes, occupied by stack
    // ret         8
}


// "Fast" calling convention
// calling convention specifies that arguments to functions are to be passed in registers, 
// when possible. When all registers are busy, remaining parameters are passed thriugh stack
//  This calling convention only applies to the x86 architecture
// the stack is cleaned up by the called function
int fastcall fastcall_func(char a, short b) fastcall_gcc
{
    return a%b;
}


// If the stack is cleaned up by the caller it can do vararg functions
// Let say 0 means the last passed param
// The Microsoft Visual Studio C/C++ compiler resolves this conflict
// by silently converting the calling convention to __cdecl
int cdecl var_param_func(int a, ...) cdecl_gcc
{
    va_list argptr;
    va_start(argptr, a);
    int i = a;
    int sum = 0;

    while (i != 0)
    {
        sum += i;
        i = va_arg(argptr, int);
    }
    va_end(argptr);
    return sum;
}


// Here we work with the stack directly, accessing it through pointer
// while passing variadic number of params
// The following function calculates average of params passed, zero is a terminator
double f_var(double n, ...)
{
    // Beginning of params pack
    double *p = &n;
    double sum = 0, count = 0;
    // Iterate over stack while stack param is zero
    // Clang-Tidy: Narrowing conversion from 'double' to 'bool'
    while (*p)
    {
        sum += (*p);            
        p++;
        count++;
    }
    return ((sum) ? sum / count : 0);
}


// If we pass arguments by non-const reference, we cant implicitly convert passed argument
// to avoid UB or slicing
// Passed argiment must be non-const either
void update(float& f)
{
    f += 1.0f;
}

void show_unconst_ref()
{
    float f = 1.0f;
    double d = 2.0;
    // error - passing constant argument
    //update(2.0f);
    update(f);
    // error - trying to convert argument
    //update(d);
}


// Function pointer
void error_(const std::string& s){ std::cerr << s << std::endl; }

void show_func_pointer()
{
    // Function pointer could be nullptr, or 0
    void(*p_func)(const std::string&) = nullptr;

    p_func = error_;	// Assign the function to pointer
    p_func = &error_;	// This form is also correct

    p_func("error");	// This call is correct
    (*p_func)("error");	// so as that

    // Let's define new type 'err_func_t' for shorter declaration
    typedef void(*err_func_t)(const std::string&);

    err_func_t p = p_func;
    p("more errors");

    // std lib has function pointers callback: new_handler
}

// ------------------------------
// Concatenation macros
// Let's generate function names, starting from the common prefix 'show'
#define CONCAT_FUNC_NAME(pref,name) pref##_##name()
void show_macro()
{
    CONCAT_FUNC_NAME(show, unconst_ref);	// show_unconst_ref()
    CONCAT_FUNC_NAME(show, func_pointer);	// show_func_pointer()
    // This would be infinite recursion
    //CONCAT_FUNC_NAME(show, macro);
    // #define MAKE_NVP(name) L###name, name -> L"name", name
}


// ----------------------------------
int main()
{
    int a = 109;
    long b = 1090;
    char aa = 56;
    short bb = 10;

    // Call function with cdecl convention
    int c = cdecl_func(a, b);
    //mov         eax,dword ptr [b]
    // put param b into stack
    //push        eax
    //mov         ecx,dword ptr [a]
    // put param a into stack
    //push        ecx
    // call the function cdecl_func()
    //call        cdecl_func (41102Dh)
    // stack cleanup
    //add         esp,8
    // ewrurn value
    //mov         dword ptr [c],eax

    int d = stdcall_func(a, &b);
    // put the address of param b into stack
    //lea         eax,[b]
    //push        eax
    //mov         ecx,dword ptr [a]
    // put the address of param a into stack
    //push        ecx
    // call the function stdcall_func()
    //call        stdcall_func (4110B9h)
    // return without cleanup
    //mov         dword ptr [d],eax


    int e = fastcall_func(aa, bb);
    // both aa and bb are being passed through registers
    //mov         dx,word ptr [bb]
    //mov         cl,byte ptr [aa]
    //call        fastcall_func (411019h)
    //mov         dword ptr [e],eax

    int sm = var_param_func(1, 2, 3, 4, -1);

    double da = 1.0, db = 2.0, dc = 3.0;
    double avg = f_var(3, &da, &db, &dc);

    show_macro();

    return 0;
}

#pragma clang diagnostic pop