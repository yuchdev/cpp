#include <cmath>
#include <iomanip>
#include <iostream>

#include <cfenv>
#include <csetjmp>
#include <csignal>

#if !defined(_MSC_VER)
#if defined(__SSE2__)
#include <xmmintrin.h>
#endif
#endif

// The floating-point environment maintains a series of status flags and specific control modes.
// Specific about the contents of the floating-point environment depend on the implementation,
// but the status flags generally include the floating-point exceptions and their associated information,
// and the control modes include at least the rounding direction.
// Working with floating-point numbers, you may face a number of situations,
// cause "FP error" or "FP exception". It includes, but not limited to:
// Rounding error, Underflow and Overflow errors, Inexact error
// Ways of catch and process these errors are strictly system- and architecture-specific.

void fp_control_noexcept()
{
    std::cout << "Floating point state flags:\n";
    std::cout << " FE_DIVBYZERO = " << FE_DIVBYZERO << '\n';
    std::cout << " FE_INEXACT = " << FE_INEXACT << '\n';
    std::cout << " FE_INVALID = " << FE_INVALID << '\n';
    std::cout << " FE_OVERFLOW = " << FE_OVERFLOW << '\n';
    std::cout << " FE_UNDERFLOW = " << FE_UNDERFLOW << '\n';

    std::cout << "Floating point rounding flags:\n";
    std::cout << " FE_DOWNWARD = " << FE_DOWNWARD << '\n';
    std::cout << " FE_TONEAREST = " << FE_TONEAREST << '\n';
    std::cout << " FE_TOWARDZERO = " << FE_TOWARDZERO << '\n';
    std::cout << " FE_UPWARD = " << FE_UPWARD << '\n';

    std::cout << "Current exceptions flags = " << fetestexcept(FE_ALL_EXCEPT) << '\n';
    std::cout << "Current rounding mode = " << fegetround() << '\n';

    // Clear exception flags
    feclearexcept(FE_ALL_EXCEPT);

    // Create Inexact/Underflow error
    double d1 = 1.0;
    size_t steps{};
    int res{};
    do
    {
        d1 /= 2.0;
        ++steps;
    } while ((res = fetestexcept(FE_ALL_EXCEPT)) == 0);
    std::cout << "Inexact/Underflow Exceptions in " << steps << " steps: " << res << '\n';
    std::cout << "2^-inf: " << d1 << '\n';

    feclearexcept(res);

    // Create Inexact/Overflow error
    double d2 = 1.0;
    steps = 0;
    do
    {
        d2 *= 2.0;
        ++steps;
    } while ((res = fetestexcept(FE_ALL_EXCEPT)) == 0);
    std::cout << "Inexact/Overflow Exceptions in " << steps << " steps: " << res << '\n';
    std::cout << "2^+inf: %g\n"
              << d2 << '\n';

    feclearexcept(res);

    // Create zero division error
    double d3 = 1.0 / d1;
    res = fetestexcept(FE_ALL_EXCEPT);
    std::cout << "Zero Div Exceptions:" << res << '\n';
    std::cout << "1/0:" << d3 << '\n';

    feclearexcept(res);

    // Create Inexact error
    double s = 1.0;
    steps = 0;
    do
    {
        s = (s + 2 / s) * 0.5;
        ++steps;
    } while ((s * s - 2) > 0);
    std::cout << "Inexact Exceptions in " << steps << " steps\n";
    std::cout << "sqrt (2): %g\n"
              << '\n';
}

void fp_control()
{
    // Floating Point Environment is a system-wide variable
    // which value is being set (and never clear!) on floating point exception/error situation,
    // and keep additional information about the error
    // FP is probably set by 'stmxcsr' assembly instruction in special SSE MXCSR register

    // default FP environment (system-dependent!)
    fenv_t fenv = *FE_DFL_ENV;

    // get current FP environment
    int ret = fegetenv(&fenv);
    std::cout << "fegetenv() returned " << ret << '\n'; // 0 is no errors

    // pair function setting fenv_t
    ret = fesetenv(&fenv);
    std::cout << "fesetenv() returned " << ret << '\n'; // 0 is no errors

    // Attempts to clear the floating-point exceptions specified
    feclearexcept(FE_ALL_EXCEPT);
    ret = fegetenv(&fenv);
    std::cout << "After feclearexcept(), fegetenv() returned " << ret << '\n'; //

    // feholdexcept() saves exception flags clears
    double x = sin(0.0); // = 0
    fenv_t fe;
    feholdexcept(&fe);
    x = log(x); // should raise exception, but fenv is clear

    // feupdateenv() function is responsible quite complicated actions
    // It saves in local memory information about exception,
    // set the new environment by fenvp value and raise in that environment saved exception
    feupdateenv(&fe);
    std::cout << "log 0 = " << x << '\n';

    if (!fetestexcept(FE_ALL_EXCEPT))
    {
        std::cout << "no exceptions raised" << '\n';
    }
}

#ifdef _MSC_VER
void fp_exceptions_ms()
{
    // MS-specific way to trap FP exceptions
    // Set the x86 floating-point control word according to what
    // exceptions you want to trap
    _clearfp(); // Always call _clearfp before setting the control

    // word
    // Because the second parameter in the following call is 0, it
    // only returns the floating-point control word

    unsigned int cw = _controlfp(0, 0); // Get the default control

    // word
    // Set the exception masks off for exceptions that you want to
    // trap. When a mask bit is set, the corresponding floating-point
    // exception is //blocked from being generating.

    cw &= ~(EM_OVERFLOW | EM_ZERODIVIDE | EM_DENORMAL | EM_INVALID);

    // For any bit in the second parameter (mask) that is 1, the
    // corresponding bit in the first parameter is used to update
    // the control word.

    unsigned int cwOriginal = _controlfp(cw, MCW_EM); // Set it.

    // MCW_EM is defined in float.h.
    // Restore the original value when done:
    _controlfp(cwOriginal, MCW_EM);
}
#else
namespace
{

sigjmp_buf g_sigfpe_jmp;
volatile sig_atomic_t g_sigfpe_seen = 0;
volatile sig_atomic_t g_sigfpe_code = 0;

struct SigFpeGuard
{
    struct sigaction oldAct{};
    bool installed{false};

    static void handler(int /*signum*/, siginfo_t* info, void* /*ucontext*/)
    {
        g_sigfpe_seen = 1;
        g_sigfpe_code = info ? info->si_code : 0;

        // Clear flags so we can continue after jumping out.
        feclearexcept(FE_ALL_EXCEPT);

        // Jump out of the faulting instruction to avoid re-trigger loops.
        siglongjmp(g_sigfpe_jmp, 1);
    }

    explicit SigFpeGuard()
    {
        struct sigaction act{};
        act.sa_sigaction = &SigFpeGuard::handler;
        sigemptyset(&act.sa_mask);
        act.sa_flags = SA_SIGINFO;

        if (sigaction(SIGFPE, &act, &oldAct) == 0)
        {
            installed = true;
        }
    }

    ~SigFpeGuard()
    {
        if (installed)
        {
            sigaction(SIGFPE, &oldAct, nullptr);
        }
    }
};

const char* sigfpe_code_name(int code)
{
    switch (code)
    {
    case FPE_INTDIV:
        return "FPE_INTDIV";
    case FPE_INTOVF:
        return "FPE_INTOVF";
    case FPE_FLTDIV:
        return "FPE_FLTDIV";
    case FPE_FLTOVF:
        return "FPE_FLTOVF";
    case FPE_FLTUND:
        return "FPE_FLTUND";
    case FPE_FLTRES:
        return "FPE_FLTRES";
    case FPE_FLTINV:
        return "FPE_FLTINV";
    case FPE_FLTSUB:
        return "FPE_FLTSUB";
    default:
        return "unknown";
    }
}

// Enables traps for a subset of FP exceptions.
// Returns previous trap mask when supported; otherwise 0.
int enable_fp_traps(int excepts)
{
#if defined(__GLIBC__) || defined(__linux__)
    // glibc provides this.
    return feenableexcept(excepts);
#elif defined(__APPLE__)
    // Apple libc doesn't consistently expose feenableexcept.
    // Use SSE MXCSR to unmask exceptions where available.
#if defined(__SSE2__)
    // Bits 7..12 are exception masks (1=masked). Clear to unmask.
    // Map FE_* to MXCSR masks (same bit positions for the standard 5 exceptions).
    const unsigned int maskBits = static_cast<unsigned int>(excepts) << 7U;
    unsigned int mxcsr = _mm_getcsr();
    unsigned int oldMasks = (mxcsr >> 7U) & 0x3FU;
    mxcsr &= ~maskBits;
    _mm_setcsr(mxcsr);
    return static_cast<int>(oldMasks);
#else
    (void)excepts;
    return 0;
#endif
#else
#if defined(FE_NOMASK_ENV)
    (void)excepts;
    fesetenv(FE_NOMASK_ENV);
    return 0;
#else
    (void)excepts;
    return 0;
#endif
#endif
}

void restore_fp_traps(int /*prev*/)
{
    // Best-effort. On Linux we could call fedisableexcept(prev-wanted)
    // but that's non-portable too; for the demo, reset to default env.
    fesetenv(FE_DFL_ENV);
}

} // namespace

void fp_exceptions_ms()
{
    std::cout << "Enabling hardware FP traps and triggering one (non-MSVC path)...\n";

    // Make sure FP exceptions start clear.
    feclearexcept(FE_ALL_EXCEPT);

    // If we trap, we'll jump back to here.
    if (sigsetjmp(g_sigfpe_jmp, 1) != 0)
    {
        std::cout << "Caught SIGFPE (si_code=" << g_sigfpe_code
                  << ", " << sigfpe_code_name(g_sigfpe_code) << ")\n";
        restore_fp_traps(0);
        return;
    }

    SigFpeGuard guard;

    // Unmask a common set.
    constexpr int want = FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW;
    const int prev = enable_fp_traps(want);

    // Force evaluation at runtime.
    volatile double one = 1.0;
    volatile double zero = 0.0;

    // This should raise FE_DIVBYZERO and typically deliver SIGFPE when unmasked.
    volatile double x = one / zero;
    (void)x;

    // If no signal/trap, we can still observe flags.
    const int raised = fetestexcept(FE_ALL_EXCEPT);
    std::cout << "No SIGFPE delivered; fetestexcept(FE_ALL_EXCEPT)=" << raised << "\n";

    restore_fp_traps(prev);
}

#endif // _MSC_VER

int main()
{
    fp_exceptions_ms();
    return 0;
}
