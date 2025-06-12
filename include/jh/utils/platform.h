/**
 * Copyright 2025 JeongHan-Bae <mastropseudo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file platform.h
 * @author JeongHan-Bae <mastropseudo@gmail.com>
 * @brief Base platform checks and macros for all modules.
 *
 * Enforces 64-bit targets and excludes unsupported compilers (MSVC).
 * Provides compiler, architecture, OS, and endianness detection macros.
 *
 * This header is implicitly included in most internal components to guard
 * against undefined behavior on unsupported platforms or toolchains.
 *
 * Marcos > constexpr: better and leaner for compilation and branch optimization.
 * @note Macro Design Philosophy:
 *
 * This header defines a minimal and precise set of `IS_*` macros
 * to identify platform, compiler, architecture, and endianness traits.
 *
 * All macros are:
 *   - Purely semantic (e.g., `IS_LINUX == 1` means target is Linux)
 *   - Zero-side-effect (no redefinitions, no overrides)
 *   - Safe for co-existence (as long as the same macro has the same meaning)
 *
 * ⚠️ These macros are intentionally simple:
 *   - No namespace pollution
 *   - No extra logic or branching
 *   - No dependency on other libraries
 *
 * ✅ This avoids undefined behavior even when included alongside
 *    other headers or libraries that define the same macros consistently.
 *
 * 📌 Rule: If another library defines the same `IS_XXX` macro
 *         **with the same meaning**, it's harmless.
 */


#pragma once

// Macros are preferred over constexpr here for:
// - Zero runtime cost
// - Preprocessor-level branching (#if)
// - No symbol pollution
// - Better compile-time stripping and leaner binaries

// === Compiler Detection ===

#if defined(__clang__)
  #define IS_CLANG 1
#else
  #define IS_CLANG 0
#endif

#if defined(__GNUC__) && !defined(__clang__)
  #define IS_GCC 1
#else
  #define IS_GCC 0
#endif

#if defined(_MSC_VER) && !defined(__clang__) && !defined(__GNUC__)
  #define IS_MSVC 1
#else
  #define IS_MSVC 0
#endif

#if IS_MSVC
    #pragma message("🚫 MSVC is not supported.")
    #pragma message("MSVC is currently unsupported due to poor and incomplete support for C++17/20.")
    #pragma message("This includes broken handling of concepts, inconsistent SFINAE behavior,")
    #pragma message("and unstable headers/linking across versions.")
    #pragma message("")
    #pragma message("➡️ Recommended alternatives:")
    #pragma message(" - GCC >= 10 (including MinGW-w64 on Windows)")
    #pragma message(" - Clang >= 11 (via MSYS2 or WSL2)")
    #pragma message("")
    #pragma message("❗ If you're on Windows, try:")
    #pragma message(" - MSYS2: https://www.msys2.org/")
    #pragma message(" - WSL2 + Ubuntu: sudo apt install g++")
    #pragma message("")
    #error "MSVC is not supported. Please use GCC or Clang."
#endif

// === Architecture Detection via sizeof ===
#include <cstddef>
static_assert(sizeof(std::size_t) == 8,
              "\U0001F6AB 32-bit targets are not supported.\n"
              "This library requires a 64-bit architecture (e.g., x86_64 or aarch64) for correct behavior and performance.\n"
              "Please switch to a 64-bit toolchain and platform."
);
// Trust actual ABI over preprocessor macros.
// This check prevents false positives from macro spoofing or incomplete platform detection.

// === Architecture Detection ===

// Posix Check
#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
#  define IS_POSIX 1
#else
#  define IS_POSIX 0
#endif

#if defined(__x86_64__) || defined(_M_X64)
#define IS_AMD64 1
#else
#define IS_AMD64 0
#endif

#if defined(__i386__) || defined(_M_IX86)
#define IS_X86 1
#else
#define IS_X86 0
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
#define IS_AARCH64 1
#else
#define IS_AARCH64 0
#endif

// x86 Mix
#define IS_X86_FAMILY (IS_X86 || IS_AMD64)

// Toolchain on Windows
#if defined(_WIN32)
#  define IS_WINDOWS 1

#  if defined(__MINGW32__) || defined(__MINGW64__)
#    define IS_MINGW 1
#  else
#    define IS_MINGW 0
#  endif

#  if defined(__clang__)
#    define IS_CLANG_ON_WINDOWS 1
#  else
#    define IS_CLANG_ON_WINDOWS 0
#  endif

#else
#  define IS_WINDOWS 0
#  define IS_MINGW 0
#  define IS_CLANG_ON_WINDOWS 0
#endif

// **Notice**: IS_MINGW and IS_CLANG_ON_WINDOWS depends on IS_WINDOWS

#if defined(__linux__)
#define IS_LINUX 1
// NOTE:
// RISC-V targets commonly use the Linux toolchain prefix (e.g., riscv64-unknown-linux-gnu),
// so they are correctly identified as Linux platforms.
// No separate `IS_RISCV` is needed — the architecture should be inferred from ISA macros (e.g. __riscv).
// This header is for ***BASIC* platform check
#else
#define IS_LINUX 0
#endif

#if defined(__APPLE__)
#  include <TargetConditionals.h>
#  define IS_APPLE 1
#else
#  define IS_APPLE 0
#endif

#if defined(__FreeBSD__)
# define IS_FREEBSD 1
#else
# define IS_FREEBSD 0
#endif

#if IS_APPLE && defined(TARGET_OS_OSX) && TARGET_OS_OSX
#  define IS_OS_X 1
#else
#  define IS_OS_X 0
#endif

#define IS_MACOS IS_OS_X

#if defined(__EMSCRIPTEN__)
# define IS_WASM 1
#else
# define IS_WASM 0
#endif

// Safe in all modern gcc/clang-based platform, no need of endian headers
#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#  define IS_BIG_ENDIAN 1
#else
#  define IS_BIG_ENDIAN 0
#endif

#define IS_LITTLE_ENDIAN (!IS_BIG_ENDIAN)
