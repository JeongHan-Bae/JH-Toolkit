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
 * @brief RAII lock adapters with sequentially-consistent fences at lock boundaries.
 *
 * <p><code>strong_unique_lock</code> and <code>strong_shared_lock</code>
 * acquire their mutex, execute a <code>seq_cst</code> fence, and execute
 * another fence before releasing the mutex. The aliases
 * <code>posix_smtx_&#42;_lock</code> select these adapters on Windows and
 * standard library lock adapters on other platforms.</p>
 *
 * <p>The project has measured a high-contention gap at the interaction
 * boundary between the C++ standard library and Windows runtime
 * implementations. The Windows alias path adds fences as a mitigation for
 * that observed behavior. Pool capacity checks and insertion now use the same
 * mutex; the adapters preserve the underlying lock's ownership rules.
 * A lock plus these fences cannot complete a missing library/runtime
 * synchronization path for every independent atomic operation. See the
 * <code>strong_lock</code> reference for details.</p>
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
     * @brief Windows exclusive-lock alias with fences at lock boundaries.
     *
     * Selects <code>strong_unique_lock</code> on Windows. The extra fences
     * are used as a mitigation for the measured high-contention gap between
     * Windows implementations.
     */
    template<jh::concepts::basic_lockable Mtx>
    using posix_smtx_unique_lock = strong_unique_lock<Mtx>;

    /**
     * @brief Windows shared-lock alias with fences at lock boundaries.
     *
     * Selects <code>strong_shared_lock</code> on Windows. The extra fences
     * are used as a mitigation for the measured high-contention gap between
     * Windows implementations.
     */
    template<jh::concepts::shared_lockable Mtx>
    using posix_smtx_shared_lock = strong_shared_lock<Mtx>;

#else

    /**
     * @brief Standard exclusive-lock alias on non-Windows platforms.
     *
     * Selects <code>std::unique_lock</code> directly.
     */
    template<jh::concepts::basic_lockable Mtx>
    using posix_smtx_unique_lock = std::unique_lock<Mtx>;

    /**
     * @brief Standard shared-lock alias on non-Windows platforms.
     *
     * Selects <code>std::shared_lock</code> directly.
     */
    template<jh::concepts::shared_lockable Mtx>
    using posix_smtx_shared_lock = std::shared_lock<Mtx>;

#endif


} // namespace jh::sync
