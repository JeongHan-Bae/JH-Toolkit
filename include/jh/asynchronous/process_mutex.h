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
 * @file process_mutex.h
 * @brief Cross-platform process-wide named mutex with timed try_lock.
 *
 * <h3>Overview</h3>
 * <p>
 * <code>jh::async::process_mutex</code> is a cross-platform, process-wide synchronization primitive
 * identified by a <strong>compile-time string literal</strong>. Each unique literal corresponds to a unique
 * OS-level named semaphore.
 * </p>
 *
 * <h3>Implementation</h3>
 * <ol>
 *   <li><strong>POSIX (Linux &amp; generic UNIX)</strong>:
 *       created/opened via <code>sem_open</code>, synchronized with <code>sem_wait</code>, <code>sem_post</code>.
 *       <ul>
 *         <li>POSIX + Realtime Extension (e.g. Linux/glibc):
 *             timed waiting via <strong>POSIX.1b</strong> API <code>sem_timedwait</code>.</li>
 *         <li>Pure POSIX platforms (e.g. Darwin/macOS, some BSDs):
 *             the SDK does not declare <code>sem_timedwait</code>. Timed waiting is emulated with
 *             <strong>sem_trywait + exponential backoff sleep</strong> to approximate semantics.</li>
 *       </ul>
 *   </li>
 *   <li><strong>Windows / MSYS2</strong>:
 *       implemented via Win32 API (<code>CreateSemaphore</code>, <code>WaitForSingleObject</code>,
 *       <code>ReleaseSemaphore</code>).</li>
 * </ol>
 *
 * <h3>Naming rules</h3>
 * <ul>
 *   <li>Only alphanumeric characters, <code>_</code>, <code>-</code>, and <code>.</code> are allowed.</li>
 *   <li>Name length must be in range <strong>[1, 128]</strong> (engineering constraint for portability).</li>
 *   <li>Prefix is added automatically:
 *       <ul>
 *         <li>POSIX: <code>"/"</code> is prepended internally.</li>
 *         <li>Windows:
 *           <ul>
 *             <li>C++ literal: <code>"Global&bsol;&bsol;name"</code></li>
 *             <li>Runtime object name: <code>Global&bsol;name</code></li>
 *           </ul></li>
 *       </ul>
 *   </li>
 * </ul>
 *
 * <h3>unlink semantics</h3>
 * <ul>
 *   <li><strong>POSIX (Linux &amp; UNIX)</strong>:
 *       <ul>
 *         <li><code>sem_unlink()</code> removes the name from the namespace immediately,
 *             but does <strong>not</strong> destroy existing semaphore objects.</li>
 *         <li>Any process that already opened the semaphore can continue using it.</li>
 *         <li>The actual semaphore is destroyed only after all processes close their descriptors
 *             via <code>sem_close()</code>.</li>
 *         <li>New processes cannot open the same name until all old descriptors are closed.</li>
 *       </ul>
 *   </li>
 *   <li><strong>Windows / MSYS2</strong>:
 *       no <code>unlink()</code> concept. Named semaphores persist while any process holds an open handle and
 *       are destroyed automatically once the last handle closes.</li>
 * </ul>
 *
 * <h4>Permissions policy (POSIX only)</h4>
 * <ul>
 *   <li>Semaphore permission bits control which users can <strong>open</strong> or <strong>unlink</strong>
 *       the semaphore name.</li>
 *   <li>This library enforces a simple policy:
 *     <ul>
 *       <li>If <code>JH_PROCESS_MUTEX_SHARED</code> is <strong>false</strong> (default):
 *           the mode is <code>0644</code> (only the creating user can unlink; others can open read-only).</li>
 *       <li>If <code>JH_PROCESS_MUTEX_SHARED</code> is <strong>true</strong>:
 *           the mode is <code>0666</code> (any user can open and unlink).</li>
 *     </ul>
 *   </li>
 *   <li>This affects only the <em>namespace</em> (open/unlink); locking semantics are unaffected.</li>
 *   <li>On Windows, <code>mode_t</code> does not exist; access control is managed internally
 *       by the Win32 API and is unaffected by this policy.</li>
 * </ul>
 *
 * <h3>Standards note</h3>
 * <ul>
 *   <li><code>sem_open</code>, <code>sem_wait</code>, <code>sem_post</code>, <code>sem_unlink</code> are
 *       part of the <strong>POSIX base standard</strong>.</li>
 *   <li><code>sem_timedwait</code> belongs to the <strong>POSIX Realtime Extension (POSIX.1b)</strong>.
 *       Linux/glibc exposes it widely. On pure POSIX systems (Darwin, BSD), it is not available, so
 *       this library emulates timed waits via backoff loop.</li>
 * </ul>
 */

#pragma once

#ifndef JH_PROCESS_MUTEX_SHARED
#define JH_PROCESS_MUTEX_SHARED false
#endif

#include "jh/str_template.h"
#include "jh/utils/platform.h"

#include <algorithm>  // for std::min
#include <string>
#include <stdexcept>
#include <chrono>
#include <cstdint>
#include <thread>

#if IS_WINDOWS
#include <windows.h>
#else
#include <semaphore.h>
#include <fcntl.h>
#include <cerrno>
#endif

#if !IS_WINDOWS
#include <sys/stat.h>

namespace jh::async::detail {
    static constexpr mode_t process_mutex_permissions =
            JH_PROCESS_MUTEX_SHARED ? static_cast<mode_t>(0666) : static_cast<mode_t>(0644);
}
#endif


namespace jh::async {

    namespace detail {
        /// Check if a character is valid in a mutex name (alnum, '_', '-', '.').
        constexpr bool is_mutex_char(char c) noexcept {
            return (c >= 'A' && c <= 'Z') ||
                   (c >= 'a' && c <= 'z') ||
                   (c >= '0' && c <= '9') ||
                   c == '_' || c == '-' || c == '.';
        }

        /// Validate compile-time string as a legal mutex name.
        template <jh::str_template::CStr S>
        consteval bool valid_mutex_name() {
            if (S.size() < 1) return false;
            if (S.size() > 128) return false;
            for (std::uint64_t i = 0; i < S.size(); ++i) {
                if (!is_mutex_char(S.val()[i])) return false;
            }
            return true;
        }

    } // namespace detail

    /**
     * @brief Cross-platform named process-wide mutex.
     *
     * @tparam S Bare name string (letters, digits, dot, dash, underscore).
     * @tparam HighPriv If true, exposes <code>unlink()</code> for POSIX.
     */
    template <jh::str_template::CStr S, bool HighPriv = false>
    requires (detail::valid_mutex_name<S>())
    class process_mutex final{
    private:
#if IS_WINDOWS
        HANDLE handle_{}; ///< OS handle for Windows
        static constexpr auto full_name_ = jh::str_template::cstr{"Global\\"} + S;
#else
        sem_t* sem_{}; ///< POSIX semaphore handle
        static constexpr auto full_name_ = jh::str_template::cstr{"/"} + S;
#endif

    public:
        /// @brief Get OS-visible name.
        static constexpr const char* name() noexcept { return full_name_.val(); }

        /// @brief Singleton instance.
        static process_mutex& instance() {
            static process_mutex inst;
            return inst;
        }

        /// @brief Deleted copy constructor.
        process_mutex(const process_mutex&) = delete;
        /// @brief Deleted copy assignment.
        process_mutex& operator=(const process_mutex&) = delete;
        /// @brief Deleted move constructor.
        process_mutex(process_mutex&&) = delete;
        /// @brief Deleted move assignment.
        process_mutex& operator=(process_mutex&&) = delete;

        /// @brief Acquire the lock (blocking).
        void lock() {
#if IS_WINDOWS
            DWORD r = WaitForSingleObject(handle_, INFINITE);
            if (r != WAIT_OBJECT_0) {
                throw std::runtime_error("WaitForSingleObject(INFINITE) failed for " + std::string{name()});
            }
#else
            if (sem_wait(sem_) == -1) {
                throw std::runtime_error("sem_wait failed for " + std::string{name()});
            }
#endif
        }

        /// @brief Try acquire without blocking.
        bool try_lock() {
#if IS_WINDOWS
            DWORD r = WaitForSingleObject(handle_, 0);
            if (r == WAIT_OBJECT_0) return true;
            if (r == WAIT_TIMEOUT)  return false;
            throw std::runtime_error("WaitForSingleObject(0) failed for " + std::string{name()});
#else
            int rc = sem_trywait(sem_);
            if (rc == 0) return true;
            if (errno == EAGAIN) return false;
            throw std::runtime_error("sem_trywait failed for " + std::string{name()});
#endif
        }

        /**
         * @brief Attempt to acquire the lock, waiting for a maximum duration.
         *
         * <details>
         * <p>
         * On Windows, this maps to <code>WaitForSingleObject</code> with a bounded timeout.
         * On POSIX systems with the <strong>Realtime Extension (POSIX.1b)</strong> (e.g. Linux/glibc),
         * this maps to <code>sem_timedwait</code>.
         * On pure POSIX systems without the extension (e.g. Darwin, BSD), timed waiting is emulated with
         * <strong>sem_trywait + exponential backoff sleep</strong>, which approximates the same semantics
         * while avoiding busy spinning.
         * </p>
         * </details>
         *
         * @tparam Rep Representation type of duration.
         * @tparam Period Period type of duration.
         * @param d Duration to wait.
         * @return <strong>true</strong> if lock acquired, <strong>false</strong> if timed out.
         * @throw std::runtime_error if the underlying system call fails unexpectedly.
         *
         * @note See @c try_lock_until for details.
         */
        template <class Rep, class Period>
        bool try_lock_for(const std::chrono::duration<Rep, Period>& d) {
            using namespace std::chrono;
            if (d <= d.zero()) return try_lock();
#if IS_WINDOWS
            constexpr auto max_ms = milliseconds{0xFFFFFFFEu};
            auto ms = duration_cast<milliseconds>(d);
            if (ms > max_ms) ms = max_ms;
            DWORD r = WaitForSingleObject(handle_, static_cast<DWORD>(ms.count()));
            if (r == WAIT_OBJECT_0) return true;
            if (r == WAIT_TIMEOUT)  return false;
            throw std::runtime_error("WaitForSingleObject(ms) failed for " + std::string{name()});
#else
            return try_lock_until(system_clock::now() + d);
#endif
        }

        /**
         * @brief Attempt to acquire the lock until an absolute time point.
         *
         * <details>
         * <p>
         * Windows uses <code>WaitForSingleObject</code> with a computed relative timeout.
         * POSIX with the <strong>Realtime Extension (POSIX.1b)</strong> (e.g. Linux/glibc) uses
         * <code>sem_timedwait</code> with an absolute <code>timespec</code>.
         * Pure POSIX systems without the extension (e.g. Darwin, BSD) emulate timed waiting
         * via <strong>sem_trywait + exponential backoff sleep</strong>, preserving observable
         * semantics (success/timeout) without excessive CPU usage.
         * </p>
         * </details>
         *
         * @tparam Clock Clock type.
         * @tparam Duration Duration type.
         * @param tp Absolute time point at which the attempt should time out.
         * @return <strong>true</strong> if lock acquired, <strong>false</strong> if timed out.
         * @throw std::runtime_error if the underlying system call fails unexpectedly.
         *
         * @note Backoff is doubled each iteration, capped at 5 ms, to balance responsiveness and CPU usage.
         */
        template <class Clock, class Duration>
        bool try_lock_until(const std::chrono::time_point<Clock, Duration>& tp) {
            using namespace std::chrono;
            if (tp <= Clock::now()) return try_lock();
#if IS_WINDOWS
            auto rel = tp - Clock::now();
            return try_lock_for(rel);
#elif HAS_POSIX_1B
            auto sys_tp = time_point_cast<system_clock::duration>(
                tp - Clock::now() + system_clock::now()
            );
            auto secs = time_point_cast<seconds>(sys_tp);
            auto nsec = duration_cast<nanoseconds>(sys_tp - secs);

            timespec ts{};
            ts.tv_sec  = static_cast<time_t>(secs.time_since_epoch().count());
            ts.tv_nsec = static_cast<long>(nsec.count());

            if (sem_timedwait(sem_, &ts) == 0) return true;
            if (errno == ETIMEDOUT) return false;
            throw std::runtime_error("sem_timedwait failed for " + std::string{name()});
#else
            auto backoff = 100us;
            while (Clock::now() < tp) {
                if (sem_trywait(sem_) == 0) return true;
                if (errno != EAGAIN) {
                    throw std::runtime_error("sem_trywait failed for " + std::string{name()});
                }
                std::this_thread::sleep_for(backoff);
                backoff = std::min(backoff * 2, duration_cast<microseconds>(5ms));
            }
            return false;
#endif
        }

        /// @brief Release the lock.
        void unlock() {
#if IS_WINDOWS
            if (!ReleaseSemaphore(handle_, 1, nullptr)) {
                throw std::runtime_error("ReleaseSemaphore failed for " + std::string{name()});
            }
#else
            if (sem_post(sem_) == -1) {
                throw std::runtime_error("sem_post failed for " + std::string{name()});
            }
#endif
        }

        /**
         * @brief Remove the semaphore name from the namespace (POSIX only).
         *
         * <h4>Semantics</h4>
         * <ul>
         *   <li>On POSIX systems, this calls <code>sem_unlink()</code> with the
         *       internally constructed name.</li>
         *   <li>If the semaphore exists and is successfully unlinked:
         *       <ul>
         *         <li>The name is removed immediately from the namespace.</li>
         *         <li>Existing open handles (in this or other processes) remain valid
         *             until they are closed via <code>sem_close()</code>.</li>
         *         <li>The semaphore object is destroyed only when the last handle closes.</li>
         *       </ul></li>
         *   <li>If the semaphore name does not exist (<code>errno == ENOENT</code>):
         *       the call is silently ignored. No exception is thrown.</li>
         *   <li>If <code>sem_unlink()</code> fails for any other reason
         *       (e.g. permissions, resource errors), an exception is thrown.</li>
         * </ul>
         *
         * <h4>Idempotency</h4>
         * <p>
         * This operation is explicitly <strong>idempotent</strong>: calling
         * <code>unlink()</code> multiple times is safe. Once the semaphore is removed,
         * subsequent calls are treated as no-ops.
         * </p>
         *
         * <h4>Windows</h4>
         * <p>
         * On Windows / MSYS2, there is no unlink concept. Named semaphores are automatically
         * destroyed by the OS when the last handle is closed.
         * </p>
         */
        static void unlink() requires (HighPriv) {
#if IS_WINDOWS
            // no unlink
#else
            if (::sem_unlink(name()) == -1) {
                if (errno == ENOENT) {
                    // semaphore does not exist, safe to ignore
                    return;
                }
                throw std::runtime_error(
                        "sem_unlink failed for " + std::string{name()} +
                        " (errno=" + std::to_string(errno) + ")"
                );
            }
#endif
        }

        /// Disabled if HighPriv == false. Non-privileged mutexes cannot call unlink().
        static void unlink() requires (!HighPriv) = delete;

    private:
        /// Private constructor: initialize handle.
        process_mutex() {
#if IS_WINDOWS
            handle_ = CreateSemaphoreA(nullptr, 1, 1, name());
            if (!handle_) {
                throw std::runtime_error("CreateSemaphoreA failed for " + std::string{name()});
            }
#else
            sem_ = sem_open(name(), O_CREAT, detail::process_mutex_permissions, 1);
            if (sem_ == SEM_FAILED) {
                throw std::runtime_error("sem_open failed for " + std::string{name()});
            }
#endif
        }

        /// Private destructor: close handle.
        ~process_mutex() noexcept {
#if IS_WINDOWS
            if (handle_) CloseHandle(handle_);
#else
            if (sem_) sem_close(sem_);
#endif
        }
    };

} // namespace jh::async