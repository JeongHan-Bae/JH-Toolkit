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
 * @file shared_process_mutex.h (asynchronous)
 * @brief Cross-process shared (read/write) timed mutex built from process primitives.
 *
 * <h3>Overview</h3>
 * <p>
 * <code>jh::async::ipc::shared_process_mutex</code> is a fully process-visible synchronization primitive
 * providing both shared and exclusive locking semantics, similar to
 * <code>std::shared_timed_mutex</code>, but implemented entirely from process-named OS primitives.
 * It enables multiple participants — threads, coroutines, or processes — to coordinate
 * access to a shared resource without requiring shared memory.
 * </p>
 *
 * <h3>Component composition</h3>
 * <ul>
 *   <li><code>process_mutex&lt;S + ".exc"&gt;</code> — exclusive access control, preventing new readers during write phases.</li>
 *   <li><code>process_condition&lt;S + ".cond"&gt;</code> — global condition variable used to wake writers or upgraders when readers exit.</li>
 *   <li><code>process_counter&lt;S + ".cnt"&gt;</code> — global atomic counter tracking the number of active readers system-wide.</li>
 *   <li><code>process_mutex&lt;S + ".pri"&gt;</code> — preemption mutex used exclusively by <strong>upgraders</strong>.
 *       It allows a participant upgrading from shared to exclusive mode to
 *       <strong>preempt all waiting writers</strong> and maintain <strong>upgrade continuity</strong>.
 *       Once <code>.pri</code> is held, no other process may enter exclusive mode until
 *       the upgrade completes. This lock does not enforce fairness — it ensures transactional upgrade atomicity.</li>
 * </ul>
 *
 * <h3>Dependency</h3>
 * <p>
 * The implementation is composed of three fundamental process-level synchronization primitives:
 * <ul>
 *   <li><code>#include "jh/asynchronous/process_mutex.h"</code> — built upon <b>named semaphore</b>.</li>
 *   <li><code>#include "jh/asynchronous/process_counter.h"</code> — built upon <b>shared memory</b>.</li>
 *   <li><code>#include "jh/asynchronous/process_condition.h"</code> — built upon <b>shared memory</b>.</li>
 * </ul>
 * </p>
 *
 * <h3>Platform compatibility</h3>
 * <p>
 * On POSIX-compliant systems, all primitives are implemented using <b>native named IPC mechanisms</b>
 * (POSIX semaphores and shared memory segments) and require no special privilege.
 * </p>
 * <p>
 * On <b>Windows</b>, due to the discrepancy between POSIX and Win32 IPC naming and visibility rules:
 * <ul>
 *   <li><b>Semaphores</b> must be created under the <code>Local&bsol;&bsol;</code> namespace to be visible
 *       within the same session (used by <code>process_mutex</code>).</li>
 *   <li><b>Shared memory objects</b> (<code>process_counter</code> and <code>process_condition</code>)
 *       must be created under the <code>Global&bsol;&bsol;</code> namespace to allow inter-process linkage.</li>
 * </ul>
 * </p>
 *
 * <h4>Important note (Windows privilege requirement)</h4>
 * <p>
 * Because access to <code>Global&bsol;&bsol;</code> objects requires administrative rights on Windows,
 * both <code>process_counter</code> and <code>process_condition</code> must be initialized
 * under elevated privilege. Consequently, any component that depends on them —
 * including <code>shared_process_mutex</code> — must be executed as an <b>Administrator</b>.
 * </p>
 * <p>
 * This restriction does <strong>not</strong> apply to POSIX systems.
 * </p>
 *
 * <h4>Testing and validation</h4>
 * <p>
 * On Windows, we built the library under the <b>URT64</b> toolchain and verified execution
 * in the <b>native PowerShell environment</b>. The IPC primitives were confirmed to function
 * correctly across processes in the same session.
 * </p>
 * <p>
 * Note that <b>MSYS2 (URT64)</b> environments typically run under <em>root-equivalent privilege</em>,
 * meaning CI environments may fail to detect privilege-related issues.
 * </p>
 * <p>
 * As of the current implementation:
 * <ul>
 *   <li><code>process_mutex</code> (semaphore-based) — operates correctly under <b>normal privilege</b>.</li>
 *   <li><code>process_counter</code> and <code>process_condition</code> (shared-memory-based) — require
 *       <b>administrator privilege</b> for creation and access.</li>
 * </ul>
 * Consequently, <code>shared_process_mutex</code> on Windows functions correctly only
 * when executed under administrative rights.
 * </p>
 *
 * <h4>Design stance</h4>
 * <p>
 * In the <b>jh-toolkit IPC model</b>, Windows is treated as a <b>second-class citizen</b>:
 * parity of semantics with POSIX is ensured, but the privilege requirement remains unavoidable.
 * Therefore, when using <code>shared_process_mutex</code> for inter-process read/write coordination
 * on Windows, administrative rights are mandatory.
 * </p>
 *
 * <h3>Design guarantees</h3>
 * <ul>
 *   <li><strong>Global visibility</strong>: all cooperating processes and threads share the same OS-named primitives.</li>
 *   <li><strong>Compile-time fixed identity</strong>: the template parameter <code>CStr S</code> uniquely defines the synchronization group.</li>
 *   <li><strong>Exclusive upgrade continuity</strong>: once an upgrade begins, it completes without interference from writers.</li>
 *   <li><strong>Deterministic semantics</strong>: fairness is not guaranteed; consistency and isolation are prioritized.</li>
 * </ul>
 *
 * <h4>Upgrade semantics</h4>
 * <p>
 * The upgrade operation may be initiated by <strong>any participant</strong> holding a shared lock —
 * including threads, coroutines, or separate processes bound to the same named primitives.
 * Because upgrade must occur in a <strong>continuous global scope</strong>,
 * the upgrader cannot yield to a waiting writer without breaking its semantic integrity.
 * </p>
 * <p>
 * Therefore, if <code>.exc</code> cannot be immediately acquired, the upgrader seizes <code>.pri</code>
 * to <strong>preempt all writers</strong> that were already waiting. Once <code>.pri</code> is held,
 * the upgrader waits for all other readers to exit and then transitions into exclusive mode.
 * Any concurrent upgrader attempting the same will be treated as a global protocol violation
 * and cause forced unlink and termination.
 * </p>
 */

#pragma once

#include "jh/str_template.h"
#include "jh/asynchronous/process_mutex.h"
#include "jh/asynchronous/process_counter.h"
#include "jh/asynchronous/process_condition.h"
#include "jh/asynchronous/ipc_limits.h"

#include <chrono>
#include <stdexcept>
#include <iostream>
#include <thread>

namespace jh::async::ipc {

    using jh::str_template::CStr;

    template <CStr S, bool HighPriv = false>
    requires (limits::valid_object_name<S, limits::max_name_length - 8>())
    class shared_process_mutex;

    /**
     * @brief Cross-process shared/exclusive timed mutex with optional upgrade support.
     *
     * <h4>Overview</h4>
     * <p>
     * <code>jh::async::ipc::shared_process_mutex</code> is a process-visible synchronization primitive
     * providing shared, exclusive, and (optionally) upgradeable locking semantics,
     * similar to <code>std::shared_timed_mutex</code> but implemented from
     * process-level IPC primitives.
     * </p>
     *
     * <h4>Composition</h4>
     * <ul>
     *   <li><code>process_mutex&lt;S + ".exc"&gt;</code> — exclusive access control.</li>
     *   <li><code>process_condition&lt;S + ".cond"&gt;</code> — wake writers or upgraders when readers exit.</li>
     *   <li><code>process_counter&lt;S + ".cnt"&gt;</code> — global reader count.</li>
     *   <li><code>process_mutex&lt;S + ".pri"&gt;</code> — preemption lock for upgrade continuity.</li>
     * </ul>
     *
     * <h4>Behavior</h4>
     * <ul>
     *   <li>Implements all <code>std::shared_timed_mutex</code> interfaces (<code>lock</code>,
     *       <code>try_lock*</code>, <code>lock_shared</code>, <code>try_lock_shared*</code>).</li>
     *   <li>The <b>HighPriv</b> variant adds <code>upgrade_lock()</code> and <code>unlink()</code>.</li>
     *   <li>Thread- and coroutine-local reentrancy is supported; repeated calls to
     *       <code>lock()</code> or <code>lock_shared()</code> in the same execution context are idempotent.</li>
     *   <li>Lock ownership is tracked via <code>thread_local</code> flags; release operations are
     *       similarly idempotent within the same participant.</li>
     * </ul>
     *
     * <h4>Upgrade semantics</h4>
     * <ul>
     *   <li>Upgrade is <b>continuous and exclusive</b>: once started, it cannot yield or be interrupted.</li>
     *   <li>Only one upgrader may exist system-wide. Concurrent upgrades are
     *       <strong>undefined behavior (UB)</strong>.</li>
     *   <li>During upgrade, the participant seizes <code>.pri</code> to preempt writers
     *       and preserve transactional consistency.</li>
     *   <li>If two participants may attempt upgrade, it is recommended to protect
     *       the upgrade path with an additional mutex and use <code>try_lock()</code> to
     *       ensure a single active upgrader.</li>
     * </ul>
     *
     * <h4>Design notes</h4>
     * <ul>
     *   <li>This class models the behavior of <code>std::shared_timed_mutex</code>,
     *       but is implemented from process-wide IPC primitives.</li>
     *   <li>It is an <strong>engineering-level primitive</strong>: deterministic, portable,
     *       and designed for correctness rather than fairness.</li>
     *   <li>Within a single thread or coroutine context, all lock operations are
     *       <strong>idempotent</strong>. Repeated acquisitions or releases are safe and
     *       treated as no-ops. This enables reentrant patterns for coroutine and task frameworks.</li>
     *   <li>Across threads or processes, reentrancy is not propagated — holding a lock in
     *       one thread does not imply ownership in another.</li>
     *   <li><code>notify_one()</code> is used intentionally: only writers wait on the
     *       condition variable, and at most one writer should proceed when readers complete.</li>
     *   <li>Fully compatible with RAII wrappers (<code>std::shared_lock</code>,
     *       <code>std::unique_lock</code>, <code>std::lock_guard</code>).</li>
     * </ul>
     *
     * <h4>Privilege and cleanup</h4>
     * <ul>
     *   <li><b>Normal variant</b> (<code>HighPriv == false</code>): shared/exclusive locks only.</li>
     *   <li><b>High-privileged variant</b> (<code>HighPriv == true</code>): adds upgrade support and <code>unlink()</code>.</li>
     *   <li>Unlink removes all associated IPC objects: <code>.exc</code>, <code>.cond</code>, <code>.cnt</code>, and <code>.pri</code>.</li>
     * </ul>
     */
    template <CStr S, bool HighPriv>
    requires (limits::valid_object_name<S, limits::max_name_length - 8>())
    class shared_process_mutex final {
    private:
        using exc_t  = process_mutex<S + CStr{".exc"}, HighPriv>;
        using cond_t = process_condition<S + CStr{".cond"}, HighPriv>;
        using cnt_t  = process_counter<S + CStr{".cnt"}, HighPriv>;
        using pri_t = process_mutex<S + CStr{".pri"}, HighPriv>;
        thread_local static bool has_shared_lock_;
        thread_local static bool has_exclusive_lock_;
        thread_local static bool has_prior_lock_;

        exc_t&  excl_;    ///< exclusive writer lock
        cond_t& cond_;    ///< condition variable
        cnt_t&  readers_; ///< reader counter
        pri_t&  prior_;   ///< priority mutex to preempt writers during upgrade

    private:
        shared_process_mutex()
                : excl_(exc_t::instance())
                , cond_(cond_t::instance())
                , readers_(cnt_t::instance())
                , prior_(pri_t::instance())
        {}

        [[nodiscard]] inline bool is_writer() const noexcept {
            return has_exclusive_lock_ || has_prior_lock_;
        }
        
    public:
        ~shared_process_mutex() = default;

        shared_process_mutex(const shared_process_mutex&) = delete;
        shared_process_mutex& operator=(const shared_process_mutex&) = delete;

        /**
         * @brief Access the process-wide singleton instance of this mutex.
         * @return A reference to the global synchronization object.
         */
        static shared_process_mutex& instance() {
            static shared_process_mutex inst;
            return inst;
        }

        /**
         * @brief Acquire exclusive access (blocking).
         * <p>
         * Sequence:
         * <ol>
         *   <li>Acquire <code>.exc</code> to block new readers.</li>
         *   <li>Wait until <code>readers_ == 0</code>.</li>
         *   <li>Acquire <code>.pri</code> to ensure no upgrader interferes.</li>
         * </ol>
         * The participant then has full exclusive access across all processes.
         * </p>
         */
        void lock() {
            if (is_writer()) return;
            excl_.lock();

            while (readers_.load() > 0)
                cond_.wait_signal();

            prior_.lock();

            has_exclusive_lock_ = true;
            has_prior_lock_ = true;
        }

        /**
         * @brief Try to acquire exclusive access immediately.
         * @return <code>true</code> if successful, <code>false</code> otherwise.
         */
        bool try_lock() {
            if (is_writer()) return true; // already have exclusive lock
            if (!excl_.try_lock()) return false;
            if (readers_.load() == 0) {
                if (!prior_.try_lock()) {
                    excl_.unlock();
                    return false;
                }
                has_exclusive_lock_ = true;
                has_prior_lock_ = true;
                return true;
            }
            excl_.unlock();

            return false;
        }

        /**
         * @brief Attempt to acquire exclusive access for a limited duration.
         */
        template <typename Rep, typename Period>
        bool try_lock_for(const std::chrono::duration<Rep, Period>& d) {
            using namespace std::chrono;
            return try_lock_until(steady_clock::now() + d);
        }

        /**
         * @brief Attempt to acquire exclusive access until a specific time point.
         */
        template <typename Clock, typename Duration>
        bool try_lock_until(const std::chrono::time_point<Clock, Duration>& tp) {
            if (is_writer())
                return true; // already have exclusive lock
            if (!excl_.try_lock_until(tp))
                return false;
            while (readers_.load() > 0) {
                if (!cond_.wait_until(tp)) {
                    excl_.unlock();
                    return false;
                }
            }
            if (!prior_.try_lock_until(tp)) {
                excl_.unlock();
                return false;
            }
            has_exclusive_lock_ = true;
            has_prior_lock_ = true;
            return true;
        }

        /**
         * @brief Release exclusive access.
         * <p>
         * Steps:
         * <ol>
         *   <li>Release <code>.pri</code>.</li>
         *   <li>Signal <code>.cond</code> to wake blocked writers or upgraders.</li>
         *   <li>If the lock was not obtained via upgrade preemption, release <code>.exc</code>.</li>
         * </ol>
         * </p>
         */
        void unlock() {
            if (!is_writer()) return;

            if (has_prior_lock_) {
                prior_.unlock();
                has_prior_lock_ = false;
            }

            if (has_exclusive_lock_) {
                has_exclusive_lock_ = false;
                excl_.unlock();
            }
        }

        /**
         * @brief Acquire shared access.
         * <p>
         * Locks <code>.exc</code> briefly to safely increment <code>readers_</code>,
         * ensuring no writer is entering concurrently.
         * </p>
         */
        void lock_shared() {
            if (has_shared_lock_) return;
            excl_.lock();
            readers_.fetch_add(1);
            excl_.unlock();
            has_shared_lock_ = true;
        }

        /**
         * @brief Attempt to acquire shared access immediately.
         */
        bool try_lock_shared() {
            if (has_shared_lock_) return true;
            if (!excl_.try_lock()) return false;
            readers_.fetch_add(1);
            excl_.unlock();
            has_shared_lock_ = true;
            return true;
        }

        /**
         * @brief Attempt to acquire shared access for a limited duration.
         */
        template <typename Rep, typename Period>
        bool try_lock_shared_for(const std::chrono::duration<Rep, Period>& d) {
            using namespace std::chrono;
            return try_lock_shared_until(steady_clock::now() + d);
        }

        /**
         * @brief Attempt to acquire shared access until a time point.
         */
        template <typename Clock, typename Duration>
        bool try_lock_shared_until(const std::chrono::time_point<Clock, Duration>& tp) {
            if (has_shared_lock_) return true;
            if (!excl_.try_lock_until(tp))
                return false;
            readers_.fetch_add(1);
            excl_.unlock();
            has_shared_lock_ = true;
            return true;
        }

        /**
         * @brief Release shared access.
         * <p>
         * Decrements <code>readers_</code>; if this was the last reader (<tt>old == 1</tt>),
         * signals <code>.cond</code> to wake one waiting writer or upgrader.
         * </p>
         */
        void unlock_shared() {
            if (!has_shared_lock_) return;
            has_shared_lock_ = false;
            auto old = readers_.fetch_sub(1);
            if (old == 1)
                cond_.notify_one();
        }

        /**
         * @brief Upgrade from shared to exclusive mode (system-wide).
         * <p>
         * Steps:
         * <ol>
         *   <li>Requires the participant to hold a shared lock.</li>
         *   <li>Attempts to acquire <code>.exc</code> (blocks new readers).</li>
         *   <li>If <code>.exc</code> is held by another writer, acquires <code>.pri</code>
         *       to preempt that writer and maintain upgrade continuity.</li>
         *   <li>If <code>.pri</code> cannot be acquired, another upgrader is active — treated as fatal violation.</li>
         *   <li>Waits until all other readers exit (<tt>readers_ == 1</tt>).</li>
         *   <li>Decrements its reader count and transitions into exclusive mode.</li>
         * </ol>
         * This operation preserves global upgrade atomicity and ensures consistency across processes.
         * </p>
         */
        void upgrade_lock() requires (HighPriv) {
            if (!has_shared_lock_)
                throw std::logic_error("Cannot upgrade without shared lock");
            if (is_writer())
                return;

            bool got_excl = excl_.try_lock();
            has_prior_lock_ = !got_excl;

            if (!got_excl) {
                if (!prior_.try_lock()) {
                    std::cerr << "[FATAL] concurrent upgrade detected in shared_process_mutex<" << S.val() << ">\n";
                    try { unlink(); } catch (...) {}
                    std::terminate();
                }
            }

            has_shared_lock_ = false;

            using namespace std::chrono;
            auto backoff = 100us;

            while (true) {
                int r = readers_.load();
                if (r == 1)
                    break;

                std::this_thread::sleep_for(backoff);
                backoff = std::min(backoff * 2, duration_cast<microseconds>(5ms));
            }

            readers_.fetch_sub(1);
            cond_.notify_one(); // wake a waiting writer if any, it will block on .pri
            has_exclusive_lock_ = got_excl;
        }

        /**
         * @brief Disabled for non-privileged variants.
         */
        void upgrade_lock() requires (!HighPriv) = delete;

        /**
         * @brief Remove all associated process primitives.
         * <p>
         * Removes the following objects from the OS namespace:
         * <ul>
         *   <li><code>process_mutex&lt;S + ".exc"&gt;</code></li>
         *   <li><code>process_condition&lt;S + ".cond"&gt;</code></li>
         *   <li><code>process_counter&lt;S + ".cnt"&gt;</code></li>
         * </ul>
         * </p>
         */
        static void unlink() requires (HighPriv) {
            exc_t::unlink();
            cond_t::unlink();
            cnt_t::unlink();
        }

        /**
         * @brief Disabled for non-privileged variants.
         */
        static void unlink() requires (!HighPriv) = delete;
    };

    template <CStr S, bool HighPriv>
    requires (limits::valid_object_name<S, limits::max_name_length - 8>())
    thread_local bool shared_process_mutex<S, HighPriv>::has_shared_lock_ = false;

    template <CStr S, bool HighPriv>
    requires (limits::valid_object_name<S, limits::max_name_length - 8>())
    thread_local bool shared_process_mutex<S, HighPriv>::has_exclusive_lock_ = false;

    template <CStr S, bool HighPriv>
    requires (limits::valid_object_name<S, limits::max_name_length - 8>())
    thread_local bool shared_process_mutex<S, HighPriv>::has_prior_lock_ = false;

} // namespace jh::async::ipc
