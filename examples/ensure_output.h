// ensure_output.h
#pragma once

#include "jh/macros/platform.h"

#if IS_WINDOWS
#include <windows.h>
  #include <io.h>      // for _setmode / fileno
  #include <fcntl.h>   // for _O_U16TEXT
  #include <cstdio>    // for stdout

  struct EnsureOutput {
      EnsureOutput() {
          // On MinGW/Clang/GCC toolchains, fileno() is correct
          _setmode(fileno(stdout), _O_U16TEXT);

          // Ensure narrow-output (printf, cout) uses UTF-8
          SetConsoleOutputCP(CP_UTF8);

          // Enable ANSI escape sequences (color, emoji, etc.)
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