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
 * @file const_lock.h
 * @brief Scope-based const-locking utility for mutex-like synchronization primitives.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 *
 * <p>
 * This header defines <code>jh::sync::const_lock</code>, a lightweight RAII-style
 * synchronization helper that enforces immutability barriers for shared or exclusive access.
 * It provides a unified locking abstraction compatible with all types satisfying
 * <code>jh::concepts::mutex_like</code>.
 * </p>
 *
 * <p>
 * When constructed, <code>const_lock</code> automatically acquires the appropriate lock:
 * <ul>
 *   <li>If the mutex type satisfies <code>shared_lockable</code>, it uses <code>lock_shared()</code>.</li>
 *   <li>Otherwise, it falls back to exclusive <code>lock()</code>.</li>
 * </ul>
 * Upon destruction, it symmetrically releases the held lock.
 * </p>
 *
 * <p>
 * If the supplied mutex is <code>jh::typed::null_mutex_t</code>,
 * all operations are completely no-op, producing zero runtime overhead.
 * This enables seamless single-threaded or thread-local use cases without conditional compilation.
 * </p>
 *
 * <p>
 * Typical usage:
 * </p>
 *
 * @code
 * std::shared_mutex sm;
 * jh::sync::const_lock guard(sm);  // Acquires shared lock automatically
 *
 * std::mutex m;
 * jh::sync::const_lock exclusive_guard(m);  // Acquires exclusive lock
 *
 * jh::sync::const_lock dummy(jh::typed::null_mutex);  // No-op
 * @endcode
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */
 
#pragma once
#include "jh/conceptual/mutex_like.h"

namespace jh::sync {

    /**
     * @brief Scope-based immutability barrier for <code>mutex_like</code> types.
     *
     * <p>
     * A lightweight RAII guard that acquires either a shared or exclusive lock
     * on construction and releases it on destruction.
     * </p>
     *
     * <ul>
     *   <li>If <code>Mutex</code> satisfies <code>shared_lockable</code>,
     *       <code>lock_shared()</code> / <code>unlock_shared()</code> are used.</li>
     *   <li>Otherwise, <code>lock()</code> / <code>unlock()</code> are used.</li>
     *   <li>If <code>Mutex</code> is <code>jh::typed::null_mutex_t</code>,
     *       all operations are no-op.</li>
     * </ul>
     *
     * <p>
     * Intended for const or read-only critical sections where immutability
     * must be preserved across threads.
     * </p>
     *
     * @note This guard is conceptually a <b>read-protection mechanism</b>.
     *       Performing any write or mutable operation within a <code>const_lock</code>
     *       protected scope constitutes <b>undefined behavior (UB)</b>.
     *       Even if the underlying mutex is exclusive-only (e.g. <code>std::mutex</code>),
     *       the logical semantics remain read-only protection.
     *
     * @tparam Mutex Any type satisfying <code>jh::concepts::mutex_like</code>.
     */
    template <jh::concepts::mutex_like Mutex>
    class const_lock final {
    public:

        /**
         * @brief Constructs the guard and acquires the lock.
         * @param m Reference to a mutex-like object.
         */
        explicit const_lock(Mutex& m) noexcept : mutex_(m) {
                lock_impl();
        }

        const_lock(const const_lock&) = delete;
        const_lock& operator=(const const_lock&) = delete;
        const_lock(const const_lock&&) = delete;
        const_lock& operator=(const const_lock&&) = delete;

        /**
         * @brief Destroys the guard and releases the lock.
         */
        ~const_lock() noexcept {
            unlock_impl();
        }

    private:
        Mutex& mutex_;

        void lock_impl() noexcept {
            if constexpr (jh::concepts::shared_lockable<Mutex>)
            mutex_.lock_shared();
            else
            mutex_.lock();
        }

        void unlock_impl() noexcept {
            if constexpr (jh::concepts::shared_lockable<Mutex>)
            mutex_.unlock_shared();
            else
            mutex_.unlock();
        }
    };

} // namespace jh::sync
