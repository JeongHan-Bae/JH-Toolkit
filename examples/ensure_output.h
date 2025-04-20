// ensure_output.h
#pragma once

#include "../include/jh/utils/platform.h"

#if IS_WINDOWS
  #include <windows.h>
  #include <io.h>
  #include <fcntl.h>

  struct EnsureOutput {
      EnsureOutput() {
          // UTF-16 wide output (wcout)
          _setmode(_fileno(stdout), _O_U16TEXT);
          // Optional: set code page to UTF-8
          SetConsoleOutputCP(CP_UTF8);

          // Enable virtual terminal processing (for emoji, colors, etc.)
          HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
          DWORD dwMode = 0;
          GetConsoleMode(hOut, &dwMode);
          SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
      }
  };
#endif
