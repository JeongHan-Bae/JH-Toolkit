#include "jh/macros/platform.h"

#if IS_WINDOWS
#include <windows.h>
#else
#include <csignal>
#include <sys/resource.h>
#endif

int main() {
#if IS_WINDOWS
    ::TerminateProcess(::GetCurrentProcess(), EXCEPTION_STACK_OVERFLOW);
#else
    const rlimit no_core_dump{0, 0};
    ::setrlimit(RLIMIT_CORE, &no_core_dump);
    std::raise(SIGSEGV);
#endif
    return 1;
}
