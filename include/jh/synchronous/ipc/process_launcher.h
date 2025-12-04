/**
 * \verbatim
 * Copyright 2025 JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
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
 * \endverbatim
 */
/**
 * @file process_launcher.h (synchronous/ipc)
 * @brief Cross-platform process launcher aligned with std::thread semantics.
 *
 * <h3>Rationale</h3>
 * <p>
 * This class encapsulates the platform-specific differences between
 * POSIX <code>fork() + execl()</code> and Windows <code>CreateProcess()</code>,
 * exposing a unified <strong>std::thread-like</strong> API.
 * </p>
 *
 * <h3>Platform differences</h3>
 * <ul>
 *   <li><strong>POSIX (Linux &amp; UNIX)</strong>:
 *     <ul>
 *       <li>Any file with execute permission can be launched (binary or script).</li>
 *       <li><code>fork()</code> creates the child, <code>execl()</code> replaces its image.</li>
 *       <li><code>wait()</code> maps to <code>waitpid()</code>.</li>
 *     </ul>
 *   </li>
 *   <li><strong>Windows / MSYS2</strong>:
 *     <ul>
 *       <li>Child processes must originate from an <strong>executable image</strong>
 *           (e.g. <code>.exe</code>, <code>.bat</code>, <code>.ps1</code>).</li>
 *       <li><code>CreateProcess()</code> is used for launching.</li>
 *       <li><code>wait()</code> maps to <code>WaitForSingleObject()</code>.</li>
 *     </ul>
 *   </li>
 * </ul>
 *
 * <h3>Binary flag</h3>
 * <p>
 * The template parameter <code>IsBinary</code> exists to simplify build workflows
 * (especially for CMake-generated executables):
 * </p>
 * <ul>
 *   <li>If <strong>true</strong>:
 *     <ul>
 *       <li>On Windows, <code>".exe"</code> is appended automatically
 *           (so <code>"writer"</code> &rarr; <code>"writer.exe"</code>).</li>
 *       <li>On POSIX, the path is used directly (no extension manipulation).</li>
 *     </ul>
 *   </li>
 *   <li>If <strong>false</strong>:
 *     <ul>
 *       <li>The string is used as-is (Windows: may be <code>.bat</code>, <code>.ps1</code>;
 *           POSIX: may be script with shebang + execute permission).</li>
 *     </ul>
 *   </li>
 * </ul>
 *
 * <h3>Path rules</h3>
 * <ul>
 *   <li>The template string must be a <strong>POSIX-style relative path</strong>:
 *     <ul>
 *       <li>No leading <code>'/'</code> (absolute paths forbidden).</li>
 *       <li><code>"./"</code> segments are meaningless and rejected.</li>
 *       <li><code>".."</code> handling:
 *         <ul>
 *           <li>By default (<code>JH_ALLOW_PARENT_PATH == 0</code>): any <code>".."</code> is forbidden.</li>
 *           <li>If <code>JH_ALLOW_PARENT_PATH == 1</code>: leading <code>"../"</code> prefixes are permitted
 *               (one or more), but:
 *             <ul>
 *               <li>The entire path cannot consist only of <code>"../"</code> segments.</li>
 *               <li>Once non-empty content has been appended, no further <code>".."</code> segments are allowed.</li>
 *             </ul>
 *           </li>
 *         </ul>
 *       </li>
 *       <li>Allowed characters: <code>[A-Za-z0-9_.-/]</code>.</li>
 *       <li>Length must be within <strong>[1, 128]</strong>.</li>
 *     </ul>
 *   </li>
 *   <li>No need to prefix with <code>"./"</code> or use <code>'\\'</code>:
 *       paths are interpreted directly by the filesystem and resolved
 *       relative to the current working directory.</li>
 * </ul>
 * <h4>Path policy</h4>
 * <ul>
 *   <li><strong>Strict validation at compile time</strong>:
 *     <ul>
 *       <li>Illegal characters are rejected.</li>
 *       <li><code>"./"</code> or mid-path <code>".."</code> segments are forbidden.</li>
 *       <li>Absolute paths (<code>"/foo/bar"</code>) are forbidden by design.</li>
 *     </ul>
 *   </li>
 *   <li><strong>Parent path relaxation</strong> (<code>JH_ALLOW_PARENT_PATH</code>):
 *     <ul>
 *       <li>Disabled (default = 0): any <code>".."</code> usage is an error.</li>
 *       <li>Enabled (= 1): only leading <code>"../"</code> prefixes are permitted;
 *           they must be followed by a valid non-empty subpath.</li>
 *     </ul>
 *   </li>
 *   <li><strong>Cross-platform normalization</strong>:
 *     <ul>
 *       <li>POSIX: used as-is, relative to <code>cwd</code>.</li>
 *       <li>Windows: forward slashes (<code>'/'</code>) are translated automatically;
 *           backslashes are not required.</li>
 *     </ul>
 *   </li>
 *   <li><strong>Security note</strong>:
 *     <ul>
 *       <li>Forbidding <code>..</code> in the middle of paths prevents
 *           directory traversal vulnerabilities.</li>
 *       <li>Restricting to relative paths avoids accidental execution of
 *           system binaries outside the project tree.</li>
 *     </ul>
 *   </li>
 * </ul>
 *
 * <h3>Semantics</h3>
 * <ul>
 *   <li>Strictly aligned with <code>std::thread</code>:
 *     <ul>
 *       <li>A <strong>handle</strong> must be explicitly <code>wait()</code>-ed.</li>
 *       <li>If destroyed without waiting, <code>std::terminate()</code> is invoked.</li>
 *       <li>No <em>kill</em> or <em>stop</em> operations are provided.</li>
 *     </ul>
 *   </li>
 *   <li>The number and identity of launchers are fixed at <strong>compile time</strong>
 *       by the template string parameter.</li>
 *   <li>The class itself is an <strong>empty static interface</strong>:
 *     <ul>
 *       <li>Cannot be instantiated.</li>
 *       <li>Provides only <code>start()</code> for launching.</li>
 *     </ul>
 *   </li>
 * </ul>
 *
 * <h4>Handle semantics &amp; security rationale</h4>
 * <ul>
 *   <li>Each call to <code>start()</code> returns a unique, move-only
 *       <code>handle</code> representing one running process.</li>
 *   <li>The handle enforces <strong>strict ownership</strong>:
 *     <ul>
 *       <li>It must be <code>wait()</code>-ed before destruction.</li>
 *       <li>Destruction of an active handle triggers <code>std::terminate()</code>.</li>
 *       <li>Move transfers ownership; the source becomes invalid and cannot
 *           be moved or waited again.</li>
 *       <li>Assigning into an active handle also terminates the program.</li>
 *     </ul>
 *   </li>
 *   <li>Each <code>process_launcher&lt;Path, IsBinary&gt;</code> has its own
 *       nested handle type, <strong>bound to its executable at compile time</strong>.</li>
 *   <li>This binding is a deliberate design for both type safety and security:
 *     <ul>
 *       <li>Prevents handle reuse or hijacking across launchers.</li>
 *       <li>Disallows runtime path injection or substitution attacks.</li>
 *       <li>Guarantees the executable path is compile-time verified.</li>
 *     </ul>
 *   </li>
 *   <li>A unified generic handle is intentionally avoided because it would:
 *     <ul>
 *       <li>Break compile-time validation guarantees,</li>
 *       <li>Enable runtime path injection,</li>
 *       <li>And expand the attack surface for untrusted binaries.</li>
 *     </ul>
 *   </li>
 *   <li>By keeping the handle type bound to its launcher, the system enforces
 *       a static trust boundary: only pre-validated, known executables can run.</li>
 * </ul>
 */

#pragma once

#ifndef JH_ALLOW_PARENT_PATH
#define JH_ALLOW_PARENT_PATH 0
#endif

#include "jh/macros/platform.h"
#include "jh/metax/t_str.h"
#include "jh/synchronous/ipc/ipc_limits.h"
#include <string>
#include <stdexcept>
#include <filesystem>
#include <iostream>   // for std::cerr

#if IS_WINDOWS
#include <windows.h>  // STARTUPINFO, PROCESS_INFORMATION, CreateProcess, WaitForSingleObject, CloseHandle
#elif IS_POSIX
#include <unistd.h>   // fork, execl, _exit
#include <sys/wait.h> // waitpid
#endif


namespace jh::sync::ipc {

    /**
     * @brief Cross-platform process launcher.
     *
     * @tparam Path Executable path (compile-time string literal).
     *           Must be a <strong>relative path</strong> following POSIX rules:
     *           <ul>
     *             <li>Allowed characters: <code>[A-Za-z0-9_.-/]</code>.</li>
     *             <li>Must not begin with <code>'/'</code> (absolute paths forbidden).</li>
     *             <li><code>"./"</code> segments are disallowed.</li>
     *             <li><code>".."</code> handling:
     *               <ul>
     *                 <li>Default (<code>JH_ALLOW_PARENT_PATH == 0</code>): any <code>".."</code> is rejected.</li>
     *                 <li>Relaxed (<code>JH_ALLOW_PARENT_PATH == 1</code>): only leading <code>"../"</code>
     *                     prefixes are allowed, and the path must contain additional content afterwards.</li>
     *                 <li>Once non-empty content has been appended, <code>".."</code> is forbidden.</li>
     *               </ul>
     *             </li>
     *             <li>Length must be in range <strong>[1, 128]</strong>.</li>
     *           </ul>
     *
     * @tparam IsBinary Distinguishes binary executables from scripts (Windows only).
     *           <ul>
     *             <li>If <strong>true</strong>: <code>".exe"</code> is appended automatically
     *                 on Windows (e.g. <code>"writer"</code> &rarr; <code>"writer.exe"</code>).</li>
     *             <li>If <strong>false</strong>: the path is used as-is
     *                 (e.g. <code>"script.ps1"</code>, <code>"runner.bat"</code>).</li>
     *             <li>On POSIX systems, this parameter has no effect: the given string
     *                 is used directly (binary or script).</li>
     *           </ul>
     *
     * <h4>Path policy</h4>
     * <ul>
     *   <li><strong>Strict validation at compile time</strong>:
     *     <ul>
     *       <li>Illegal characters are rejected.</li>
     *       <li><code>"./"</code> or mid-path <code>".."</code> segments are forbidden.</li>
     *       <li>Absolute paths (<code>"/foo/bar"</code>) are forbidden.</li>
     *     </ul>
     *   </li>
     *   <li><strong>Parent path relaxation</strong> (<code>JH_ALLOW_PARENT_PATH</code>):
     *     <ul>
     *       <li>Disabled (default = 0): any <code>".."</code> usage is invalid.</li>
     *       <li>Enabled (= 1): leading <code>"../"</code> prefixes are allowed,
     *           but the path cannot consist only of them.</li>
     *     </ul>
     *   </li>
     *   <li><strong>Cross-platform normalization</strong>:
     *     <ul>
     *       <li>POSIX: path used as-is, relative to <code>cwd</code>.</li>
     *       <li>Windows: forward slashes (<code>'/'</code>) are translated automatically,
     *           backslashes are unnecessary.</li>
     *     </ul>
     *   </li>
     * </ul>
     *
     * <p>
     * Each instantiation corresponds to a specific executable determined
     * at <strong>compile time</strong>. The type is unique per string literal
     * and parameter combination.
     * </p>
     */
    template<jh::meta::TStr Path, bool IsBinary = true>
    requires (limits::valid_relative_path<Path>())
    class process_launcher final {
    public:
        process_launcher() = delete;                                    ///< Not constructible.
        process_launcher(const process_launcher &) = delete;            ///< Not copyable.
        process_launcher &operator=(const process_launcher &) = delete; ///< Not assignable.

        /**
         * @brief Process handle representing a single launched instance.
         *
         * <h4>Semantics</h4>
         * <ul>
         *   <li>Must be explicitly <code>wait()</code>-ed before destruction.</li>
         *   <li>If destroyed while still active, the program terminates.</li>
         *   <li>Non-copyable, but <strong>movable</strong> with strict rules:</li>
         *   <ul>
         *     <li>Move transfers exclusive ownership.</li>
         *     <li>The source becomes invalid and cannot be reused or waited.</li>
         *     <li>Assigning into an active handle triggers <code>std::terminate()</code>.</li>
         *   </ul>
         *   <li>These rules mirror <code>std::thread</code> semantics and ensure
         *       safe, deterministic process lifetime management.</li>
         * </ul>
         *
         * <h4>Security binding</h4>
         * <ul>
         *   <li>This handle type is <strong>tightly bound</strong> to its
         *       <code>process_launcher&lt;Path, IsBinary&gt;</code> template.</li>
         *   <li>Each launcher produces a unique handle type tied to a specific
         *       compile-time verified executable path.</li>
         *   <li>This prevents handle hijacking, cross-launcher misuse, and
         *       runtime path injection attacks.</li>
         * </ul>
         */
        struct handle final {
        public:
            handle(const handle &) = delete;

            handle &operator=(const handle &) = delete;

            handle(handle &&other) noexcept {
                move_from(std::move(other));
            }

            handle &operator=(handle &&other) noexcept {
                if (this == &other)
                    return *this;

                // if this handle is still active, error out
                if (!waited_) {
                    std::cerr << "Error: assigning into active process handle\n";
                    std::terminate();
                }

                move_from(std::move(other));
                return *this;
            }

            /**
             * @brief Destructor enforces <code>std::thread</code>-like semantics.
             *
             * <ul>
             *   <li>If <code>wait()</code> has not been called, the program is terminated.</li>
             *   <li>Otherwise, underlying OS handles are released safely.</li>
             * </ul>
             */
            ~handle() {
                if (!waited_) {
                    std::cerr << "Error: process handle destroyed without wait()\n";
                    std::terminate();
                }
#if IS_WINDOWS
                if (pi_.hProcess) CloseHandle(pi_.hProcess);
                if (pi_.hThread)  CloseHandle(pi_.hThread);
#endif
            }

            /**
             * @brief Wait for the launched process to finish.
             *
             * <p>
             * Blocks until the child process terminates.
             * Multiple calls are idempotent.
             * </p>
             */
            void wait() {
                if (waited_) return;
#if IS_WINDOWS
                WaitForSingleObject(pi_.hProcess, INFINITE);
#elif IS_POSIX
                int status;
                waitpid(pid_, &status, 0);
#endif
                waited_ = true;
            }

        private:
            friend class process_launcher;

            explicit handle(
#if IS_WINDOWS
                    PROCESS_INFORMATION pi
#else
                    pid_t pid
#endif
            )
#if IS_WINDOWS
            : pi_(pi)
#else
                    : pid_(pid)
#endif
            {}

            bool waited_{false};

#if IS_WINDOWS
            PROCESS_INFORMATION pi_{};
#elif IS_POSIX
            pid_t pid_{};
#endif

            void move_from(handle &&other) noexcept {
                // avoid moving from invalid or already-waited handles
                if (other.waited_) {
                    std::cerr << "Error: moving from an invalid or waited handle\n";
                    std::terminate();
                }
#if IS_WINDOWS
                // transfer ownership of handles
                pi_ = other.pi_;
                // void out source handles
                other.pi_.hProcess = nullptr;
                other.pi_.hThread  = nullptr;
#elif IS_POSIX
                pid_ = other.pid_;
                other.pid_ = -1;
#endif
                // mark source as waited
                waited_ = false;
                other.waited_ = true;
            }
        };

        /**
         * @brief Launch the target process.
         *
         * <p>
         * On success, returns a <strong>handle</strong> which must be
         * explicitly <code>wait()</code>-ed.
         * </p>
         *
         * @throw std::runtime_error if process creation fails.
         */
        static handle start() {
#if IS_WINDOWS
            STARTUPINFO si{};
            si.cb = sizeof(si);
            PROCESS_INFORMATION pi{};

            // Ensure consistent semantics with POSIX: always launch from current directory
            std::filesystem::path exe = std::filesystem::path(".") / Path.val();
            if constexpr (IsBinary) {
                        exe.concat(".exe");
            }

            // Windows requires a mutable command line buffer for lpCommandLine
            std::string cmdline = exe.string();

            if (!CreateProcess(
                        exe.string().c_str(),   // lpApplicationName
                        cmdline.data(),         // lpCommandLine (argv[0])
                        nullptr,                // lpProcessAttributes
                        nullptr,                // lpThreadAttributes
                        FALSE,                  // bInheritHandles
                        0,                      // dwCreationFlags
                        nullptr,                // lpEnvironment
                        nullptr,                // lpCurrentDirectory
                        &si, &pi))
            {
                        DWORD err = GetLastError();
                        throw std::runtime_error(
                                    "CreateProcess failed for " + exe.string() +
                                    " (error=" + std::to_string(err) + ")"
                        );
            }
            return handle{pi};
#elif IS_POSIX
            std::string exe = "./" + std::string(Path.val());
            pid_t pid = fork();
            if (pid == 0) {
                execl(exe.c_str(), exe.c_str(), nullptr);
                // exec failed: safely terminate only the child process (avoid atexit/DTOR)
                _exit(1);
            }
            return handle{pid};
#endif
        }
    };

} // namespace jh::sync::ipc
