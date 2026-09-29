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
 * @file process_launcher.h
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
 *       <li><code>wait()</code> uses <code>waitpid()</code> to return normal exit codes or report signal termination.</li>
 *     </ul>
 *   </li>
 *   <li><strong>Windows / MSYS2</strong>:
 *     <ul>
 *       <li>Child processes must originate from an <strong>executable image</strong>
 *           (e.g. <code>.exe</code>, <code>.bat</code>, <code>.ps1</code>).</li>
 *       <li><code>CreateProcess()</code> is used for launching.</li>
 *       <li><code>wait()</code> uses <code>WaitForSingleObject()</code> and reads the process exit code.</li>
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
 *           <li>By default (<code>JH_INTERPROCESS_ALLOW_PARENT_PATH == 0</code>): any <code>".."</code> is forbidden.</li>
 *           <li>If <code>JH_INTERPROCESS_ALLOW_PARENT_PATH == 1</code>: leading <code>"../"</code> prefixes are permitted
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
 *   <li><strong>Parent path relaxation</strong> (<code>JH_INTERPROCESS_ALLOW_PARENT_PATH</code>):
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

#ifndef JH_INTERPROCESS_ALLOW_PARENT_PATH
#define JH_INTERPROCESS_ALLOW_PARENT_PATH 0
#endif

#include "jh/macros/platform.h"
#include "jh/metax/expected.h"
#include "jh/metax/t_str.h"
#include "jh/synchronous/ipc/ipc_limits.h"
#include <cstddef>
#include <cstdint>
#include <cerrno>
#include <string>
#include <stdexcept>
#include <system_error>
#include <filesystem>
#include <iostream>   // for std::cerr

#if IS_WINDOWS
#include <windows.h>  // STARTUPINFO, PROCESS_INFORMATION, CreateProcess, WaitForSingleObject, CloseHandle
#elif IS_POSIX
#include <csignal>
#include <fcntl.h>    // fcntl, FD_CLOEXEC
#include <unistd.h>   // close, fork, execl, read, write, _exit
#include <sys/wait.h> // waitpid

#endif


namespace jh::sync::ipc {

#if IS_POSIX
    namespace detail {
        template<class Fork>
        [[nodiscard]] pid_t fork_or_throw(Fork &&fork_operation) {
            const pid_t pid = fork_operation();
            if (pid < 0) {
                const int error = errno;
                throw std::system_error(error, std::generic_category(), "fork");
            }
            return pid;
        }
    }
#endif

    /**
     * @brief Describes why a launched process did not produce a normal exit value.
     *
     * On POSIX, a signal termination stores the signal number as the enum value.
     * The core-dump flag is combined with that number when the system reports one.
     * Named signal values are provided when the platform defines the corresponding
     * signal; other signal numbers, including real-time signals, are preserved too.
     */
    enum class process_exit_error : std::uint32_t {
#if IS_POSIX
        /// @brief POSIX signal number SIGHUP.
        signal_hangup = SIGHUP,
        /// @brief POSIX signal number SIGINT.
        signal_interrupt = SIGINT,
        /// @brief POSIX signal number SIGQUIT.
        signal_quit = SIGQUIT,
        /// @brief POSIX signal number SIGILL.
        signal_illegal_instruction = SIGILL,
        /// @brief POSIX signal number SIGABRT.
        signal_abort = SIGABRT,
        /// @brief POSIX signal number SIGFPE.
        signal_floating_point = SIGFPE,
        /// @brief POSIX signal number SIGKILL.
        signal_killed = SIGKILL,
        /// @brief POSIX signal number SIGSEGV.
        signal_segmentation_fault = SIGSEGV,
        /// @brief POSIX signal number SIGPIPE.
        signal_broken_pipe = SIGPIPE,
        /// @brief POSIX signal number SIGALRM.
        signal_alarm = SIGALRM,
        /// @brief POSIX signal number SIGTERM.
        signal_terminate = SIGTERM,
        /// @brief POSIX signal number SIGUSR1.
        signal_user_1 = SIGUSR1,
        /// @brief POSIX signal number SIGUSR2.
        signal_user_2 = SIGUSR2,
        /// @brief POSIX signal number SIGCHLD.
        signal_child = SIGCHLD,
        /// @brief POSIX signal number SIGCONT.
        signal_continue = SIGCONT,
        /// @brief POSIX signal number SIGSTOP.
        signal_stop = SIGSTOP,
        /// @brief POSIX signal number SIGTSTP.
        signal_terminal_stop = SIGTSTP,
        /// @brief POSIX signal number SIGTTIN.
        signal_background_terminal_input = SIGTTIN,
        /// @brief POSIX signal number SIGTTOU.
        signal_background_terminal_output = SIGTTOU,
#ifdef SIGBUS
        /// @brief POSIX signal number SIGBUS.
        signal_bus_error = SIGBUS,
#endif
#ifdef SIGTRAP
        /// @brief Platform signal number SIGTRAP.
        signal_trap = SIGTRAP,
#endif
#ifdef SIGURG
        /// @brief Platform signal number SIGURG.
        signal_urgent_socket = SIGURG,
#endif
#ifdef SIGXCPU
        /// @brief Platform signal number SIGXCPU.
        signal_cpu_limit = SIGXCPU,
#endif
#ifdef SIGXFSZ
        /// @brief Platform signal number SIGXFSZ.
        signal_file_size_limit = SIGXFSZ,
#endif
#ifdef SIGVTALRM
        /// @brief Platform signal number SIGVTALRM.
        signal_virtual_alarm = SIGVTALRM,
#endif
#ifdef SIGPROF
        /// @brief Platform signal number SIGPROF.
        signal_profiling_alarm = SIGPROF,
#endif
#ifdef SIGWINCH
        /// @brief Platform signal number SIGWINCH.
        signal_window_change = SIGWINCH,
#endif
#ifdef SIGPOLL
        /// @brief Platform signal number SIGPOLL.
        signal_poll = SIGPOLL,
#endif
#ifdef SIGSYS
        /// @brief Platform signal number SIGSYS.
        signal_bad_system_call = SIGSYS,
#endif
#ifdef SIGEMT
        /// @brief Platform signal number SIGEMT.
        signal_emulator_trap = SIGEMT,
#endif
#ifdef SIGINFO
        /// @brief Platform signal number SIGINFO.
        signal_information = SIGINFO,
#endif
#ifdef SIGIOT
        /// @brief Platform signal number SIGIOT.
        signal_iot = SIGIOT,
#endif
#ifdef SIGIO
        /// @brief Platform signal number SIGIO.
        signal_io = SIGIO,
#endif
#ifdef SIGPWR
        /// @brief Platform signal number SIGPWR.
        signal_power_failure = SIGPWR,
#endif
#ifdef SIGSTKFLT
        /// @brief Platform signal number SIGSTKFLT.
        signal_stack_fault = SIGSTKFLT,
#endif
#endif
        /// @brief The child stopped without returning a normal process exit code.
        abnormal_termination = 0x7FFFFFFEu,
        /// @brief The operating system could not provide a child process status.
        wait_failed = 0x7FFFFFFFu
    };

#if IS_POSIX
    /// @brief Bit set in a POSIX process error when the wait status reports a core dump.
    inline constexpr std::uint32_t process_exit_core_dump_mask = 0x80000000u;

    /**
     * @brief Return whether an error encodes a POSIX terminating signal.
     * @param error Process error returned by <code>handle::wait()</code>.
     * @return <code>true</code> when the error stores a signal number.
     */
    [[nodiscard]] inline constexpr bool process_exit_is_signal(const process_exit_error error) noexcept {
        const auto raw = static_cast<std::uint32_t>(error);
        const auto signal = raw & ~process_exit_core_dump_mask;
        return signal != 0 && signal < static_cast<std::uint32_t>(process_exit_error::abnormal_termination);
    }

    /**
     * @brief Return the POSIX signal number encoded in a signal termination error.
     * @param error Process error returned by <code>handle::wait()</code>.
     * @return The terminating signal number. Call only when
     *         <code>process_exit_is_signal(error)</code> is true.
     */
    [[nodiscard]] inline constexpr std::uint32_t process_exit_signal_number(
        const process_exit_error error
    ) noexcept {
        return static_cast<std::uint32_t>(error) & ~process_exit_core_dump_mask;
    }

    /**
     * @brief Return whether the POSIX system reported a core dump for the signal termination.
     * @param error Process error returned by <code>handle::wait()</code>.
     * @return <code>true</code> if the signal termination generated a core dump.
     */
    [[nodiscard]] inline constexpr bool process_exit_has_core_dump(const process_exit_error error) noexcept {
        return process_exit_is_signal(error) &&
               (static_cast<std::uint32_t>(error) & process_exit_core_dump_mask) != 0;
    }
#endif

    /**
     * @brief Normal process exit code or an abnormal termination or wait error.
     *
     * The success type is a fixed-width unsigned carrier for every supported
     * platform. POSIX <code>waitpid()</code> exposes only the low 8 bits of a
     * normal exit status, while Windows <code>GetExitCodeProcess()</code> returns
     * a 32-bit unsigned <code>DWORD</code>.
     */
    using process_exit_result = jh::meta::expected<std::uint32_t, process_exit_error>;

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
     *                 <li>Default (<code>JH_INTERPROCESS_ALLOW_PARENT_PATH == 0</code>): any <code>".."</code> is rejected.</li>
     *                 <li>Relaxed (<code>JH_INTERPROCESS_ALLOW_PARENT_PATH == 1</code>): only leading <code>"../"</code>
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
     *   <li><strong>Parent path relaxation</strong> (<code>JH_INTERPROCESS_ALLOW_PARENT_PATH</code>):
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
    template<jh::meta::TStr Path, bool IsBinary = true> requires (limits::valid_relative_path<Path>())
    class process_launcher final {
#if IS_WINDOWS
    private:
        static constexpr bool is_abnormal_exit_code(const DWORD code) noexcept {
            switch (code) {
                case EXCEPTION_ACCESS_VIOLATION:
                case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
                case EXCEPTION_ILLEGAL_INSTRUCTION:
                case EXCEPTION_INT_DIVIDE_BY_ZERO:
                case EXCEPTION_NONCONTINUABLE_EXCEPTION:
                case EXCEPTION_PRIV_INSTRUCTION:
                case EXCEPTION_STACK_OVERFLOW:
                case 0xC000013Au:
                    return true;
                default:
                    return false;
            }
        }
#endif

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
             * @brief Wait for the process and return its normal exit code.
             *
             * A normal exit is a value even when the code is nonzero. Abnormal
             * termination or a wait-system-call failure is returned as an error.
             * Repeated waits return the cached result.
             * On POSIX, signal termination is detected with <code>waitpid()</code>.
             * A normal POSIX exit value is limited by <code>WEXITSTATUS</code> to
             * the low 8 bits; Windows returns the full 32-bit process exit code.
             * On Windows, known structured-exception exit codes are classified as abnormal;
             * other platform exit codes are returned as normal values.
             *
             * @return The process exit code, or a process exit error.
             */
            [[nodiscard]] process_exit_result wait() {
                if (waited_) return exit_result_;
#if IS_WINDOWS
                const DWORD wait_status = WaitForSingleObject(pi_.hProcess, INFINITE);
                if (wait_status != WAIT_OBJECT_0) {
                    waited_ = true;
                    exit_result_ = jh::meta::unexpected{process_exit_error::wait_failed};
                    return exit_result_;
                }

                DWORD exit_code{};
                if (!GetExitCodeProcess(pi_.hProcess, &exit_code)) {
                    waited_ = true;
                    exit_result_ = jh::meta::unexpected{process_exit_error::wait_failed};
                    return exit_result_;
                }

                waited_ = true;
                if (process_launcher::is_abnormal_exit_code(exit_code)) {
                    exit_result_ = jh::meta::unexpected{process_exit_error::abnormal_termination};
                } else {
                    exit_result_ = static_cast<std::uint32_t>(exit_code);
                }
                return exit_result_;
#elif IS_POSIX
                int status{};
                pid_t wait_status{};
                do {
                    wait_status = waitpid(pid_, &status, 0);
                } while (wait_status == -1 && errno == EINTR);

                if (wait_status == -1) {
                    waited_ = true;
                    exit_result_ = jh::meta::unexpected{process_exit_error::wait_failed};
                    return exit_result_;
                }

                waited_ = true;
                if (WIFEXITED(status)) {
                    exit_result_ = static_cast<std::uint32_t>(WEXITSTATUS(status));
                } else if (WIFSIGNALED(status)) {
                    std::uint32_t error = static_cast<std::uint32_t>(WTERMSIG(status));
#ifdef WCOREDUMP
                    if (WCOREDUMP(status)) error |= process_exit_core_dump_mask;
#endif
                    exit_result_ = jh::meta::unexpected{static_cast<process_exit_error>(error)};
                } else {
                    exit_result_ = jh::meta::unexpected{process_exit_error::abnormal_termination};
                }
                return exit_result_;
#endif
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
            process_exit_result exit_result_{};

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
                other.exit_result_ = jh::meta::unexpected{process_exit_error::wait_failed};
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
         * On POSIX, both <code>fork()</code> and <code>exec()</code> failures
         * are reported synchronously before a handle is returned.
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
            auto exe = jh::meta::TStr{"./"} + Path;
            int exec_pipe[2]{};
            if (::pipe(exec_pipe) == -1) {
                const int error = errno;
                throw std::system_error(error, std::generic_category(), "pipe");
            }

            for (const int fd : exec_pipe) {
                const int descriptor_flags = ::fcntl(fd, F_GETFD);
                if (descriptor_flags == -1 ||
                    ::fcntl(fd, F_SETFD, descriptor_flags | FD_CLOEXEC) == -1) {
                    const int error = errno;
                    ::close(exec_pipe[0]);
                    ::close(exec_pipe[1]);
                    throw std::system_error(error, std::generic_category(), "fcntl(FD_CLOEXEC)");
                }
            }

            pid_t pid = detail::fork_or_throw([] { return fork(); });

            if (pid == 0) {
                ::close(exec_pipe[0]);
                ::execl(exe.val(), exe.val(), static_cast<char *>(nullptr));

                const int error = errno;
                const auto *error_bytes = reinterpret_cast<const char *>(&error);
                std::size_t bytes_remaining = sizeof(error);
                while (bytes_remaining > 0) {
                    const ssize_t bytes_written = ::write(exec_pipe[1], error_bytes, bytes_remaining);
                    if (bytes_written == -1 && errno == EINTR) continue;
                    if (bytes_written <= 0) break;
                    error_bytes += bytes_written;
                    bytes_remaining -= static_cast<std::size_t>(bytes_written);
                }
                _exit(1);
            }

            ::close(exec_pipe[1]);

            int exec_error{};
            auto *error_bytes = reinterpret_cast<char *>(&exec_error);
            std::size_t bytes_read{};
            int read_error{};
            while (bytes_read < sizeof(exec_error)) {
                const ssize_t count = ::read(
                    exec_pipe[0], error_bytes + bytes_read, sizeof(exec_error) - bytes_read);
                if (count == -1 && errno == EINTR) continue;
                if (count == -1) {
                    read_error = errno;
                    break;
                }
                if (count == 0) break;
                bytes_read += static_cast<std::size_t>(count);
            }
            ::close(exec_pipe[0]);

            const auto reap_child = [pid](const bool terminate) noexcept {
                if (terminate) static_cast<void>(::kill(pid, SIGKILL));
                int status{};
                pid_t waited{};
                do {
                    waited = ::waitpid(pid, &status, 0);
                } while (waited == -1 && errno == EINTR);
            };

            if (read_error != 0) {
                reap_child(true);
                throw std::system_error(read_error, std::generic_category(), "read exec status");
            }
            if (bytes_read == sizeof(exec_error)) {
                reap_child(false);
                throw std::system_error(exec_error, std::generic_category(), "exec " + std::string{exe.val()});
            }
            if (bytes_read != 0) {
                reap_child(true);
                throw std::system_error(EIO, std::generic_category(), "incomplete exec status");
            }

            return handle{pid};
#endif
        }
    };

} // namespace jh::sync::ipc
