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
 * @file process_condition.h (asynchronous)
 * @brief Cross-process condition variable primitive implemented via shared memory or named events.
 *
 * <h3>Overview</h3>
 * <p>
 * <code>jh::async::ipc::process_condition</code> is an inter-process signaling primitive
 * modeled after <code>pthread_cond_t</code> / <code>std::condition_variable_any</code>.
 * It provides a minimal, globally visible synchronization point usable across processes,
 * implemented entirely via OS-named IPC mechanisms.
 * </p>
 *
 * <h3>Platform implementation</h3>
 * <ul>
 *   <li><b>POSIX (Linux / BSD / Darwin)</b>:
 *     <ul>
 *       <li>Backed by <code>shm_open</code> + <code>mmap</code> + <code>pthread_cond_t</code>.</li>
 *       <li>Condition and mutex objects are marked <code>PTHREAD_PROCESS_SHARED</code>.</li>
 *       <li>All processes share the same shared-memory segment; no privilege escalation required.</li>
 *     </ul>
 *   </li>
 *   <li><b>Windows / MSYS2</b>:
 *     <ul>
 *       <li>Backed by <code>CreateEventA</code> and <code>WaitForSingleObject</code>.</li>
 *       <li>Uses <code>Global&bsol;&bsol;</code> namespace for inter-process visibility.</li>
 *       <li><strong>Administrator privilege</strong> is required to create or open
 *           <code>Global&bsol;&bsol;</code> named events.</li>
 *     </ul>
 *   </li>
 * </ul>
 *
 * <h3>Design stance</h3>
 * <p>
 * In the <b>jh-toolkit IPC model</b>, Windows is treated as a <b>second-class citizen</b>:
 * API compatibility and basic semantics are preserved, but exact parity of behavior
 * (particularly in multi-notification semantics) is <b>not</b> guaranteed.
 * </p>
 *
 * <h4>Notification semantics</h4>
 * <ul>
 *   <li><b>POSIX</b>: <code>notify_all(n)</code> signals up to <code>n</code> waiting processes
 *       (default 32). Exceeding waiters remain blocked until the next call.</li>
 *   <li><b>Windows</b>: there is no native multi-waiter broadcast.
 *       <code>notify_all()</code> simulates this behavior by setting the event for
 *       ~1 ms, which is sufficient for most engineering use cases but not strictly equivalent.</li>
 *   <li>For deterministic coordination across multiple listeners, users should
 *       layer a <code>process_counter</code> to implement precise wake control.</li>
 * </ul>
 *
 * <h3>Privilege requirement</h3>
 * <p>
 * On POSIX systems, <code>process_condition</code> requires no special privileges.
 * On Windows, due to <code>Global&bsol;&bsol;</code> namespace policy, creation and access require
 * administrative rights. The same restriction applies to <code>process_counter</code>.
 * </p>
 *
 * <h3>Design guarantees</h3>
 * <ul>
 *   <li><strong>Global visibility</strong>: all processes referencing the same name participate in the same wait-set.</li>
 *   <li><strong>Process-safe</strong>: internally protected by <code>pthread_mutex_t</code> or event handle.</li>
 *   <li><strong>Primitive-level abstraction</strong>: can be freely composed with other IPC primitives
 *       (<code>process_mutex</code>, <code>process_counter</code>) to form higher-level protocols.</li>
 *   <li><strong>Portable API</strong>: interface parity across POSIX and Windows, despite differing kernel behavior.</li>
 * </ul>
 *
 * <h3>Unlink semantics</h3>
 * <ul>
 *   <li><b>POSIX</b>: invokes <code>shm_unlink()</code> on the internal shared-memory segment.</li>
 *   <li><b>Windows</b>: no explicit unlink; event objects are destroyed when the last handle closes.</li>
 *   <li>As with other IPC primitives, the operation is idempotent — redundant calls are no-ops.</li>
 * </ul>
 *
 * <h4>Usage note</h4>
 * <p>
 * <code>process_condition</code> is an <strong>IPC primitive</strong>.
 * It does not guarantee fairness or broadcast consistency across all platforms.
 * Developers are encouraged to compose it with <code>process_mutex</code> or
 * <code>process_counter</code> when building higher-level coordination patterns.
 * </p>
 */

#pragma once
#include "jh/str_template.h"
#include "jh/macros/platform.h"
#include "jh/asynchronous/process_mutex.h"
#include "jh/asynchronous/ipc_limits.h"

#include <chrono>
#include <stdexcept>
#include <string>

#if IS_WINDOWS
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <pthread.h>
#endif

namespace jh::async::ipc {

    using jh::str_template::CStr;

    /**
     * @brief Cross-process condition variable primitive (POSIX / Win32).
     *
     * <h4>Overview</h4>
     * <p>
     * <code>jh::async::ipc::process_condition</code> provides a minimal inter-process signaling
     * mechanism modeled after <code>pthread_cond_t</code>. It is a true IPC primitive implemented
     * with OS-level facilities and usable across multiple processes.
     * </p>
     *
     * <h4>Platform behavior</h4>
     * <ul>
     *   <li><b>POSIX</b>: uses <code>pthread_cond_t</code> with <code>PTHREAD_PROCESS_SHARED</code>
     *       stored in shared memory.</li>
     *   <li><b>Windows</b>: uses named <code>Event</code> objects in the
     *       <code>Global&bsol;&bsol;</code> namespace. Administrator privilege is required.</li>
     * </ul>
     *
     * <h4>Notification model</h4>
     * <ul>
     *   <li><b>POSIX</b>: <code>notify_all(n)</code> wakes up to <code>n</code> waiting processes
     *       (default 32).</li>
     *   <li><b>Windows</b>: there is no true broadcast; <code>notify_all()</code> simulates it
     *       by holding the event open for about 1 ms.</li>
     * </ul>
     *
     * <h4>Design stance</h4>
     * <p>
     * Windows is treated as a <b>second-class IPC target</b>: API compatibility is preserved,
     * but strict semantic equivalence is not guaranteed. The primitive is intended for
     * composition with <code>process_mutex</code> or <code>process_counter</code> in
     * higher-level synchronization constructs.
     * </p>
     *
     * <h4>Privilege and unlink</h4>
     * <ul>
     *   <li><b>POSIX</b>: no special privilege required; supports explicit <code>unlink()</code>.</li>
     *   <li><b>Windows</b>: requires Administrator rights; no unlink concept.</li>
     * </ul>
     *
     * @tparam S Bare name string (letters, digits, dot, dash, underscore).
     * @tparam HighPriv If true, exposes <code>unlink()</code> (POSIX only).
     */
    template <CStr S, bool HighPriv = false>
    requires (limits::valid_object_name<S, limits::max_name_length>())
    class process_condition final {
    private:
#if IS_WINDOWS
        static constexpr auto shm_name_  = jh::str_template::cstr{"Global\\"} + S;
        HANDLE event_ = nullptr;
#else
        static constexpr auto shm_name_ = jh::str_template::cstr{"/"} + S;
        static constexpr mode_t shm_mode = JH_PROCESS_MUTEX_SHARED ? 0666 : 0644;

        struct cond_data {
            bool initialized;
            pthread_mutex_t mutex;
            pthread_cond_t cond;
        };

        int fd_ = -1;
        cond_data* data_ = nullptr;
#endif

        process_condition() {
#if IS_WINDOWS
            event_ = ::CreateEventA(
                nullptr,
                TRUE,
                FALSE,
                shm_name_.val());
            if (!event_)
                throw std::runtime_error("process_condition: CreateEventA failed (errno=" +
                                         std::to_string(::GetLastError()) + ")");
#else
            // 1. open shared memory
            fd_ = ::shm_open(shm_name_.val(), O_CREAT | O_RDWR, shm_mode);
            if (fd_ == -1)
                throw std::runtime_error("process_condition: shm_open failed (errno=" + std::to_string(errno) + ")");

            // 2. global init guard
            auto& init_guard = process_mutex<S>::instance();
            std::lock_guard lock(init_guard);

            // 3. ensure size
            struct stat st{};
            if (::fstat(fd_, &st) == -1)
                throw std::runtime_error("process_condition: fstat failed (errno=" + std::to_string(errno) + ")");
            if (st.st_size < sizeof(cond_data))
                if (::ftruncate(fd_, sizeof(cond_data)) == -1)
                    throw std::runtime_error("process_condition: ftruncate failed (errno=" + std::to_string(errno) + ")");

            // 4. mmap
            void* ptr = ::mmap(nullptr, sizeof(cond_data), PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
            if (ptr == MAP_FAILED)
                throw std::runtime_error("process_condition: mmap failed (errno=" + std::to_string(errno) + ")");
            data_ = static_cast<cond_data*>(ptr);
            ::close(fd_);

            // 5. initialize once
            if (!data_->initialized) {
                pthread_mutexattr_t mattr;
                pthread_condattr_t cattr;

                pthread_mutexattr_init(&mattr);
                pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);

                pthread_condattr_init(&cattr);
                pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);

                pthread_mutex_init(&data_->mutex, &mattr);
                pthread_cond_init(&data_->cond, &cattr);
                data_->initialized = true;

                pthread_mutexattr_destroy(&mattr);
                pthread_condattr_destroy(&cattr);
            }
#endif
        }

        ~process_condition() noexcept {
#if IS_WINDOWS
            if (event_) ::CloseHandle(event_);
#else
            if (data_) ::munmap(data_, sizeof(cond_data));
#endif
        }

    public:
        static process_condition& instance() {
            static process_condition inst;
            return inst;
        }

        process_condition(const process_condition&) = delete;
        process_condition& operator=(const process_condition&) = delete;

        /**
         * @brief Wait until a signal or broadcast occurs.
         *
         * <h4>Semantics</h4>
         * Blocks the current process until another participant calls
         * <code>notify_one()</code> or <code>notify_all()</code>.
         * Spurious wakeups may occur.
         */
        void wait_signal() noexcept {
#if IS_WINDOWS
            ::WaitForSingleObject(event_, INFINITE);
            // auto-reset manually
            ::ResetEvent(event_);
#else
            pthread_mutex_lock(&data_->mutex);
            pthread_cond_wait(&data_->cond, &data_->mutex);
            pthread_mutex_unlock(&data_->mutex);
#endif
        }

        /**
         * @brief Wait until signaled or a timeout expires.
         *
         * <h4>Semantics</h4>
         * Suspends execution until the specified absolute time point
         * or until another process issues a notification.
         *
         * @tparam Clock    Clock type used to measure time (e.g. <code>std::chrono::steady_clock</code>).
         * @tparam Duration Duration type representing the time resolution.
         * @param tp Absolute time point until which the caller should wait.
         *
         * @return <code>true</code> if signaled before timeout, otherwise <code>false</code>.
         */
        template <typename Clock, typename Duration>
        bool wait_until(const std::chrono::time_point<Clock, Duration>& tp) noexcept {
#if IS_WINDOWS
            using namespace std::chrono;
            auto rel = duration_cast<milliseconds>(tp - Clock::now());
            DWORD timeout = (rel.count() > 0) ? static_cast<DWORD>(rel.count()) : 0;
            DWORD r = ::WaitForSingleObject(event_, timeout);
            bool ok = (r == WAIT_OBJECT_0);
            if (ok) ::ResetEvent(event_);
            return ok;
#else
            using namespace std::chrono;
            auto secs = time_point_cast<seconds>(tp);
            auto nsec = duration_cast<nanoseconds>(tp - secs);
            timespec ts{};
            ts.tv_sec  = static_cast<time_t>(secs.time_since_epoch().count());
            ts.tv_nsec = static_cast<long>(nsec.count());

            pthread_mutex_lock(&data_->mutex);
            int rc = pthread_cond_timedwait(&data_->cond, &data_->mutex, &ts);
            pthread_mutex_unlock(&data_->mutex);
            return (rc == 0);
#endif
        }

        /**
         * @brief Wake a single waiting process.
         *
         * <h4>Semantics</h4>
         * Releases exactly one participant blocked in <code>wait_signal()</code>
         * or <code>wait_until()</code>.
         */
        void notify_one() noexcept {
#if IS_WINDOWS
            ::SetEvent(event_);
#else
            pthread_mutex_lock(&data_->mutex);
            pthread_cond_signal(&data_->cond);
            pthread_mutex_unlock(&data_->mutex);
#endif
        }

#if IS_WINDOWS
        /**
         * @brief Wake multiple waiting processes (Windows implementation).
         *
         * <h4>Semantics</h4>
         * Simulates broadcast by setting the named event for ~1&nbsp;ms,
         * allowing multiple waiting participants to resume.
         * This is an engineering approximation — not a guaranteed broadcast.
         */
        void notify_all(int) noexcept {
            ::SetEvent(event_);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            ::ResetEvent(event_);
        }

        /**
         * @brief Equivalent to <code>notify_all(32)</code>.
         *
         * <h4>Semantics</h4>
         * Maintains API parity with POSIX; count argument is ignored.
         */
        void notify_all() noexcept {
            notify_all(32);
        }
#else
        /**
         * @brief Wake multiple waiting processes (POSIX implementation).
         *
         * <h4>Semantics</h4>
         * Signals up to <code>count</code> waiting participants
         * (<code>pthread_cond_signal</code> loop, default 32).
         * Exceeding waiters remain blocked until the next call.
         *
         * @param count Number of waiting processes to wake (default 32).
         */
        void notify_all(int count = 32) noexcept {
            pthread_mutex_lock(&data_->mutex);
            for (int i = 0; i < count; ++i)
                pthread_cond_signal(&data_->cond);
            pthread_mutex_unlock(&data_->mutex);
        }
#endif
        /**
         * @brief Remove the shared-memory backing (POSIX only).
         *
         * <h4>Semantics</h4>
         * Invokes <code>shm_unlink()</code> for the condition's shared segment.
         * Safe and idempotent; ignored on Windows.
         */
        static void unlink() requires(HighPriv) {
#if IS_WINDOWS
            // no unlink semantics on Windows
#else
            ::shm_unlink(shm_name_.val());
#endif
        }

        /// Disabled if HighPriv == false. Non-privileged variants cannot call unlink().
        static void unlink() requires(!HighPriv) = delete;
    };

} // namespace jh::async::ipc
