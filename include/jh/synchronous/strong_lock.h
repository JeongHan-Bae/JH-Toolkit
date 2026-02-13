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
 * @file strong_lock.h
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 * @brief Strong ordering adapters for read-write mutex-like synchronization primitives.
 *
 * <p>
 * On POSIX platforms (e.g., Linux or Darwin),
 * <code>std::shared_mutex</code> is commonly implemented on top of
 * <code>pthread_rwlock</code> or futex-based primitives.
 * In practice:
 * </p>
 *
 * <ul>
 *   <li><code>std::unique_lock&lt;std::shared_mutex&gt;</code>
 *       behaves similarly to <code>pthread_rwlock_wrlock()</code> /
 *       <code>pthread_rwlock_unlock()</code>.</li>
 *   <li><code>std::shared_lock&lt;std::shared_mutex&gt;</code>
 *       behaves similarly to <code>pthread_rwlock_rdlock()</code> /
 *       <code>pthread_rwlock_unlock()</code>.</li>
 * </ul>
 *
 * <p>
 * However, ISO C++ does not strictly guarantee identical global ordering
 * semantics compared to POSIX implementations.
 * In particular, acquire-release semantics alone may be insufficient
 * when locks and atomics are mixed under high concurrency.
 * </p>
 * <p>
 * Even when atomics use <code>memory_order_seq_cst</code>,
 * observable reordering may still occur under combinations of:
 * high concurrency, debug builds, and intrusive test frameworks.
 * The strong variants aim to reduce such cross-domain ordering anomalies.
 * </p>
 * <p>
 * This header introduces two strengthening adapters:
 * </p>
 *
 * <ul>
 *   <li><code>strong_unique_lock</code> requires
 *       <code>jh::concepts::basic_lockable</code>.</li>
 *   <li><code>strong_shared_lock</code> requires
 *       <code>jh::concepts::shared_lockable</code>.</li>
 * </ul>
 *
 * <ul>
 *   <li>
 *     <code>strong_unique_lock</code> and <code>strong_shared_lock</code><br>
 *     They enforce a global ordering barrier via
 *     <code>std::atomic_thread_fence(std::memory_order_seq_cst)</code>
 *     immediately after acquisition and immediately before release.
 *   </li>
 *   <li>
 *     <code>posix_smtx_&#42;_lock</code><br>
 *     On Windows, aliases the strong variants to approximate POSIX-style
 *     behavior. On POSIX systems, aliases the standard library lock types.
 *   </li>
 * </ul>
 *
 * @note
 * This facility does not address lifecycle inconsistencies
 * introduced by test frameworks that intercept or wrap
 * <code>std::thread</code> handles.
 * Such issues originate from thread management semantics,
 * not from <code>std::shared_mutex</code> itself.
 *
 * @note
 * The primary target platform remains POSIX.
 * The Windows strengthening path exists to provide a bounded,
 * practical approximation for cross-platform engineering scenarios.
 *
 * @version <pre>1.4.1</pre>
 * @date <pre>2026</pre>
 */

#pragma once

#include <shared_mutex>
#include <atomic>
#include <jh/conceptual/mutex_like.h>
#include "jh/macros/platform.h"

namespace jh::sync {

    /**
     * @brief Strong exclusive lock adapter.
     *
     * Requires only exclusive locking capability.
     * A sequentially-consistent fence is inserted
     * after acquisition and before release.
     */
    template<jh::concepts::basic_lockable Mutex>
    class strong_unique_lock final {
    public:
        explicit strong_unique_lock(Mutex &m)
                : m_(&m), owns_(true) {
            m_->lock();
            std::atomic_thread_fence(std::memory_order_seq_cst);
        }

        ~strong_unique_lock() {
            if (owns_) {
                std::atomic_thread_fence(std::memory_order_seq_cst);
                m_->unlock();
            }
        }

        strong_unique_lock(const strong_unique_lock &) = delete;

        strong_unique_lock &operator=(const strong_unique_lock &) = delete;

    private:
        Mutex *m_;
        bool owns_;
    };


    /**
     * @brief Strong shared (reader) lock adapter.
     *
     * Requires shared locking capability.
     * A sequentially-consistent fence is inserted
     * after acquisition and before release.
     */
    template<jh::concepts::shared_lockable Mutex>
    class strong_shared_lock final {
    public:
        explicit strong_shared_lock(Mutex &m)
                : m_(&m), owns_(true) {
            m_->lock_shared();
            std::atomic_thread_fence(std::memory_order_seq_cst);
        }

        ~strong_shared_lock() {
            if (owns_) {
                std::atomic_thread_fence(std::memory_order_seq_cst);
                m_->unlock_shared();
            }
        }

        strong_shared_lock(const strong_shared_lock &) = delete;

        strong_shared_lock &operator=(const strong_shared_lock &) = delete;

    private:
        Mutex *m_;
        bool owns_;
    };


#if IS_WINDOWS

    /**
     * @brief POSIX-style strong exclusive lock alias (Windows approximation).
     *
     * On Windows platforms this aliases strong_unique_lock in order to
     * approximate the stronger global ordering behavior commonly observed
     * on POSIX systems.
     *
     * This strengthening operates strictly at the ISO C++ memory model level
     * by inserting sequentially-consistent fences around lock boundaries.
     *
     * <p>It does NOT:</p>
     * <ul>
     *   <li>Guarantee prevention of CPU-level reordering,</li>
     *   <li>Provide a true hardware-enforced global total order,</li>
     *   <li>Fully replicate POSIX rwlock implementation semantics.</li>
     * </ul>
     *
     * <p>The effect is a bounded, practical mitigation that may:</p>
     * <ul>
     *   <li>Reduce cross-domain lock/atomic ordering anomalies,</li>
     *   <li>Delay or raise the observable contention threshold on Windows,</li>
     *   <li>Improve behavioral convergence under high concurrency.</li>
     * </ul>
     *
     * It should be regarded as a language-level strengthening adapter,
     * not a formal cross-platform equivalence guarantee.
     *
     * @note
     * On POSIX systems, this alias uses std::unique_lock directly.
     */
    template<jh::concepts::basic_lockable Mtx>
    using posix_smtx_unique_lock = strong_unique_lock<Mtx>;

    /**
     * @brief POSIX-style strong shared lock alias (Windows approximation).
     *
     * On Windows platforms this aliases strong_shared_lock to approximate
     * POSIX-style shared mutex ordering characteristics.
     *
     * The inserted seq_cst fences strengthen ordering only within the
     * C++ abstract machine.
     *
     * <p>They do NOT:</p>
     * <ul>
     *   <li>Eliminate hardware-level reordering effects,</li>
     *   <li>Establish a strict global total order equivalent to
     *       typical pthread_rwlock behavior.</li>
     * </ul>
     *
     * This adapter provides a pragmatic mitigation strategy rather than
     * strict semantic equivalence.
     *
     * @note
     * On POSIX systems, this alias uses std::shared_lock directly.
     */
    template<jh::concepts::shared_lockable Mtx>
    using posix_smtx_shared_lock = strong_shared_lock<Mtx>;

#else

    /**
     * @brief POSIX native exclusive lock alias.
     *
     * On POSIX systems this aliases std::unique_lock directly,
     * relying on the underlying pthread-based implementation.
     *
     * No additional language-level strengthening is applied.
     *
     * @note
     * On Windows, this alias uses strong_unique_lock to approximate
     * POSIX-style behavior.
     */
    template<jh::concepts::basic_lockable Mtx>
    using posix_smtx_unique_lock = std::unique_lock<Mtx>;

    /**
     * @brief POSIX native shared lock alias.
     *
     * On POSIX systems this aliases std::shared_lock directly,
     * relying on the native pthread-backed shared mutex semantics.
     *
     * No additional language-level strengthening is applied.
     *
     * @note
     * On Windows, this alias uses strong_shared_lock to approximate
     * POSIX-style behavior.
     */
    template<jh::concepts::shared_lockable Mtx>
    using posix_smtx_shared_lock = std::shared_lock<Mtx>;

#endif


} // namespace jh::sync
