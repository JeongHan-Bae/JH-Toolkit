/**
 * @copyright
 * Copyright 2025 JeongHan-Bae &lt;mastropseudo\@gmail.com&gt;
 * <br>
 * Licensed under the Apache License, Version 2.0 (the "License"); <br>
 * you may not use this file except in compliance with the License.<br>
 * You may obtain a copy of the License at<br>
 * <br>
 *     http://www.apache.org/licenses/LICENSE-2.0<br>
 * <br>
 * Unless required by applicable law or agreed to in writing, software<br>
 * distributed under the License is distributed on an "AS IS" BASIS,<br>
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.<br>
 * See the License for the specific language governing permissions and<br>
 * limitations under the License.<br>
 * <br>
 * Full license: <a href="https://github.com/JeongHan-Bae/JH-Toolkit?tab=Apache-2.0-1-ov-file#readme">GitHub</a>
 */
/**
 * @file ipc_limits.h
 * @brief Compile-time validation utilities for IPC object naming and POSIX-style path safety.
 *
 * <h3>Overview</h3>
 * <p>
 * This header defines <code>consteval</code> (compile-time evaluated) utilities
 * to enforce platform-aware constraints for inter-process communication (IPC) primitives
 * implemented under <code>jh::sync::ipc</code>.
 * </p>
 *
 * <p>
 * It validates:
 * <ul>
 *   <li>IPC object names (used by semaphores, shared memory, conditions, etc.)</li>
 *   <li>POSIX-style relative paths (for safe file-based or namespace-based IPC)</li>
 * </ul>
 * All validation occurs entirely at <b>compile time</b> &mdash; no runtime overhead is introduced.
 * </p>
 *
 * <h3>Platform-specific limits</h3>
 * <ul>
 *   <li><b>FreeBSD / Darwin (macOS)</b>:
 *       maximum = 30 (strict BSD POSIX limit, 31 bytes including leading '/').</li>
 *   <li><b>Linux / Windows / WASM</b>:
 *       extended limit = 128 (safe portable maximum across modern systems).</li>
 *   <li><b>If <code>JH_FORCE_SHORT_SEM_NAME</code> is defined as 1,</b>
 *       the name length is treated as "same as BSD" (30) regardless of platform.
 *       This is used if a non-detected BSD-like environment is targeted or developing cross-platform code
 *       on non-BSD systems.</li>
 * </ul>
 * <p>
 * The limit is automatically selected at compile time using <code>jh/macros/platform.h</code>.
 * </p>
 *
 * <h4>Design philosophy</h4>
 * <p>
 * These checks are performed via <code>consteval</code> functions, ensuring:
 * <ul>
 *   <li>Invalid IPC names or paths cause <b>compile-time errors</b>.</li>
 *   <li>No runtime validation or branching is ever emitted.</li>
 *   <li>Validation logic is portable and deterministic across compilers.</li>
 * </ul>
 * This design provides early failure detection for illegal resource names
 * and guarantees identical IPC namespace semantics across all supported platforms.
 * </p>
 */

#pragma once

#include "jh/metax/t_str.h"
#include "jh/macros/platform.h"
#include <cstdint>

#ifndef JH_ALLOW_PARENT_PATH
#define JH_ALLOW_PARENT_PATH 0
#endif

#ifndef JH_FORCE_SHORT_SEM_NAME
#define JH_FORCE_SHORT_SEM_NAME 0
#endif

/**
 * @brief Compile-time constraint and validation utilities for
 * <code>jh::sync::ipc</code>.
 *
 * This namespace provides <code>consteval</code>-based helpers used to
 * validate IPC object names and POSIX-style relative paths at compile time.
 * It is an internal support layer and not intended for direct runtime use.
 *
 * @see jh::sync::ipc
 */
namespace jh::sync::ipc::limits {

    // BSD-derived systems have strict 31-byte limit (including '/')
#if IS_DARWIN || IS_FREEBSD || JH_FORCE_SHORT_SEM_NAME
    inline constexpr std::uint64_t max_name_length = 30;
#else
    // Linux, Windows, WASM: more permissive; keep it conservative but practical
    inline constexpr std::uint64_t max_name_length = 128;
#endif

    namespace detail {
        /// Check if a character is valid in an IPC object name.
        consteval bool is_valid_name_char(char c) noexcept {
            return (c >= 'A' && c <= 'Z') ||
                   (c >= 'a' && c <= 'z') ||
                   (c >= '0' && c <= '9') ||
                   c == '_' || c == '-' || c == '.';
        }
        /// Check if a character is valid in a POSIX relative path.
        consteval bool is_path_char(char c) noexcept {
            return (c >= 'A' && c <= 'Z') ||
                   (c >= 'a' && c <= 'z') ||
                   (c >= '0' && c <= '9') ||
                   c == '_' || c == '-' || c == '.' || c == '/';
        }
    } // namespace detail

    /**
     * @brief Validate compile-time IPC object name (for semaphores, shared memory, etc.).
     *
     * <h4>Rules</h4>
     * <ul>
     *   <li>Length must be in range <b>[1, MaxLen]</b>, default = 30 (BSD limit).</li>
     *   <li>Allowed characters: <code>[A-Za-z0-9_.-]</code>.</li>
     *   <li>No leading '/' (automatically added by the OS namespace).</li>
     * </ul>
     *
     * <h4>Template parameters</h4>
     * <ul>
     *   <li><code>S</code> &mdash; compile-time string representing base name.</li>
     *   <li><code>MaxLen</code> &mdash; optional override for system-specific maximum name length.</li>
     * </ul>
     *
     * @return <code>true</code> if the name is valid, otherwise <code>false</code>.
     */
    template<jh::meta::TStr S, std::uint64_t MaxLen = max_name_length>
    consteval bool valid_object_name() {
        if (S.size() < 1) return false;
        if (S.size() > MaxLen) return false;
        for (std::uint64_t i = 0; i < S.size(); ++i)
            if (!detail::is_valid_name_char(S.val()[i]))
                return false;
        return true;
    }

    /**
     * @brief Compile-time validation for POSIX-style relative paths.
     *
     * <h4>Rules</h4>
     * <ul>
     *   <li>Length in range [1, 128].</li>
     *   <li>Absolute paths forbidden (no leading <code>'/'</code>).</li>
     *   <li>No <code>"./"</code> segments.</li>
     *   <li><code>".."</code> segments:
     *     <ul>
     *       <li>When <code>JH_ALLOW_PARENT_PATH == 0</code> &rarr; forbidden.</li>
     *       <li>When <code>JH_ALLOW_PARENT_PATH == 1</code> &rarr; leading <code>"../"</code>
     *       allowed but cannot occupy entire path, and no <code>".."</code> after content begins.</li>
     *     </ul>
     *   </li>
     *   <li>Allowed characters: <code>[A-Za-z0-9_.-/]</code>.</li>
     * </ul>
     */
    template<jh::meta::TStr S>
    consteval bool valid_relative_path() {
        if (S.size() < 1) return false;
        if (S.size() > 128) return false;
        if (S.val()[0] == '/') return false;   // absolute path forbidden

        std::uint64_t i = 0;

#if JH_ALLOW_PARENT_PATH
        // Allow leading "../" segments
        while (i + 2 < S.size() &&
               S.val()[i] == '.' &&
               S.val()[i + 1] == '.' &&
               S.val()[i + 2] == '/')
        {
            i += 3;
        }
        if (i == S.size()) return false; // path cannot be only ../
#endif

        for (; i < S.size(); ++i) {
            if (!detail::is_path_char(S.val()[i]))
                return false;

            // reject ".." appearing mid-path
            if (S.val()[i] == '.' && i + 1 < S.size() && S.val()[i + 1] == '.')
                return false;
        }

        return true;
    }

} // namespace jh::sync::ipc::limits
