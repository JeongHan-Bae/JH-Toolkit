// ensure_output.h
//
// This header ensures that console output does not become garbled on Windows
// when printing non-ASCII characters (e.g., UTF-8 text, emoji, etc.).
//
// Windows support in this library is limited to MinGW toolchains
// (MinGW-w64 / MinGW-clang). MSVC is not supported.
//
// This file is unrelated to the actual standard operations demonstrated
// by the library. It exists solely to make debug output readable.
//
// The library does not aim to provide a fully featured or production-ready
// printing system. All stream output within the library is intended
// strictly for debugging purposes.
//
// Any printing in the examples/ directory is for demonstration only.

#pragma once

#include <jh/macros/platform.h>

#if IS_WINDOWS
#include <windows.h>
#include <io.h>      // for _setmode / fileno
#include <fcntl.h>   // for _O_U16TEXT
#include <cstdio>    // for stdout

struct EnsureOutput {
    EnsureOutput() {
        SetConsoleOutputCP(CP_UTF8);

        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE) {
            DWORD dwMode = 0;
            if (GetConsoleMode(hOut, &dwMode)) {
                SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
            }
        }
    }
};
#else

// On POSIX platforms, nothing required
struct EnsureOutput {
    EnsureOutput() noexcept = default;
};

#endif
