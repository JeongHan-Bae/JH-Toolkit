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
 * @file null_mutex.h
 * @brief Zero-cost semantic placeholder for <code>mutex_like</code> synchronization.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 *
 * <p>
 * Defines <code>jh::typed::null_mutex_t</code> &mdash; a fully concept-compatible,
 * zero-overhead dummy mutex used to satisfy <code>jh::concepts::mutex_like</code>
 * and related locking concepts.
 * </p>
 *
 * <p>
 * This type provides all standard locking interfaces (<code>lock()</code>,
 * <code>unlock()</code>, <code>try_lock()</code>, <code>lock_shared()</code>, etc.)
 * as pure no-ops. All <code>try_*</code> functions always return <code>true</code>.
 * </p>
 *
 * <p>
 * This type exists only for generic compatibility with <code>mutex_like</code> concepts.
 * It carries no runtime state and performs no synchronization.
 * </p>
 *
 * <p>
 * You should never create separate instances of this type.
 * Always use the global constant <code>jh::typed::null_mutex</code> instead.
 * </p>
 *
 * <p><b>Typical usage:</b></p>
 * @code
 *
 * auto s = jh::safe_from("text", typed::null_mutex);
 * // returns std::shared_ptr&lt;jh::immutable_str&gt;
 * jh::sync::const_lock guard(typed::null_mutex);  // No-op
 * @endcode
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once
#include <chrono>
#include "jh/conceptual/mutex_like.h"

namespace jh::typed {

    /**
     * @brief Semantic placeholder for <code>mutex_like</code> types.
     *
     * <p>
     * A zero-cost, fully concept-compatible dummy mutex.
     * All locking functions are implemented as no-ops, and all <code>try_*</code>
     * operations always return <code>true</code>.
     * </p>
     *
     * <p>
     * This type serves as a semantic stand-in for real synchronization primitives
     * in contexts where locking is structurally required but semantically unnecessary &mdash;
     * for example, when thread safety is guaranteed by design or when the data is
     * confined to a single thread.
     * </p>
     *
     * <p><b>Concept compatibility:</b></p>
     * <ul>
     *   <li><code>jh::concepts::mutex_like</code></li>
     *   <li><code>jh::concepts::timed_mutex_like</code></li>
     *   <li><code>jh::concepts::rw_mutex_like</code></li>
     *   <li><code>jh::concepts::reentrance_capable_mutex</code></li>
     * </ul>
     *
     * <p>
     * It is analogous in spirit to <code>std::nullptr_t</code>:
     * the type exists for semantic consistency, but creating distinct instances
     * is meaningless. Use the provided global constant
     * <code>jh::typed::null_mutex</code> instead.
     * </p>
     *
     * @see jh::typed::null_mutex
     * @see jh::concepts::mutex_like
     * @see jh::sync::const_lock
     */
    struct null_mutex_t final {

        /**
         * @brief Marker tag indicating that this mutex supports idempotent reentrance.
         */
        using is_reentrant_tag [[maybe_unused]] = std::true_type;

        // Exclusive locks
        [[maybe_unused]] void lock() noexcept {}
        [[maybe_unused]] void unlock() noexcept {}
        [[maybe_unused]] static bool try_lock() noexcept { return true; }

        template <typename Rep, typename Period>
        [[maybe_unused]] bool try_lock_for(const std::chrono::duration<Rep, Period>&) noexcept { return true; }

        template <typename Clock, typename Duration>
        [[maybe_unused]] bool try_lock_until(const std::chrono::time_point<Clock, Duration>&) noexcept { return true; }

        // Shared locks
        [[maybe_unused]] void lock_shared() noexcept {}
        [[maybe_unused]] void unlock_shared() noexcept {}
        [[maybe_unused]] static bool try_lock_shared() noexcept { return true; }

        template <typename Rep, typename Period>
        [[maybe_unused]] bool try_lock_shared_for(const std::chrono::duration<Rep, Period>&) noexcept { return true; }

        template <typename Clock, typename Duration>
        [[maybe_unused]] bool try_lock_shared_until(const std::chrono::time_point<Clock, Duration>&) noexcept { return true; }
    };

    static_assert(jh::concepts::mutex_like<null_mutex_t>);
    static_assert(jh::concepts::timed_mutex_like<null_mutex_t>);
    static_assert(jh::concepts::rw_mutex_like<null_mutex_t>);
    static_assert(jh::concepts::reentrance_capable_mutex<null_mutex_t>);

    /**
     * @brief Global constant instance of <code>null_mutex_t</code>.
     *
     * <p>
     * Represents a universal no-op synchronization primitive.
     * All operations are guaranteed to be optimized away.
     * </p>
     *
     * <p><b>Usage:</b></p>
     * @code
     * auto s = jh::safe_from(thread_local_string, jh::typed::null_mutex);
     * // returns std::shared_ptr&lt;jh::immutable_str&gt;
     * @endcode
     *
     * @note
     * <b>Semantic intent:</b> Using <code>null_mutex</code> explicitly declares that
     * the protected resource is owned and accessed by a <b>single thread</b> only.
     * It serves as a formal statement of exclusive single-thread ownership rather
     * than an omission of synchronization.
     *
     * <p>
     * This singleton should be used instead of creating local instances.
     * Declaring <code>null_mutex_t</code> variables is meaningless,
     * similar to instantiating <code>std::nullptr_t</code>.
     * </p>
     */
    inline null_mutex_t null_mutex;

} // namespace jh::typed
