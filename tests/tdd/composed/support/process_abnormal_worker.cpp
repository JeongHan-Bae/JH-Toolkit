#include "jh/macros/platform.h"

#if IS_WINDOWS
#include <windows.h>
#else
#include <csignal>
#endif

int main() {
#if IS_WINDOWS
    ::TerminateProcess(::GetCurrentProcess(), EXCEPTION_STACK_OVERFLOW);
#else
    std::raise(SIGKILL);
#endif
    return 1;
}
