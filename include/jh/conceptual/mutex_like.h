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
 * @file mutex_like.h (conceptual)
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief Foundational synchronization concepts and reentrance traits.
 *
 * <p>
 * Defines the full concept hierarchy governing <code>mutex</code>-like types
 * and provides trait registries for compile-time detection of reentrancy.
 * </p>
 *
 * <h3>Concept Layers</h3>
 * <ul>
 *   <li><b>Exclusive Lock Concepts</b>: <code>basic_lockable</code>, <code>excl_lockable</code>, <code>timed_excl_lockable</code></li>
 *   <li><b>Shared Lock Concepts</b>: <code>shared_lockable</code>, <code>timed_shared_lockable</code></li>
 *   <li><b>Unified Concepts</b>: <code>mutex_like</code>, <code>timed_mutex_like</code>, <code>rw_mutex_like</code></li>
 *   <li><b>Reentrance Concepts</b>: <code>reentrant_mutex</code>, <code>recursive_mutex</code>, <code>reentrance_capable_mutex</code></li>
 * </ul>
 *
 * <h3>Design Intent</h3>
 * <ul>
 *   <li>Provide strong <b>concept constraints</b> for lock-like objects.</li>
 *   <li>Enable <b>compile-time validation</b> of synchronization primitives.</li>
 *   <li>Allow zero-cost substitution of structural locks (e.g., <code>null_mutex_t</code>).</li>
 * </ul>
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <concepts>
#include <chrono>
#include <mutex>
#include <type_traits>

namespace jh::concepts {

    /**
     * @brief Concept for basic lockable objects.
     *
     * <p>
     * Requires only <code>lock()</code> and <code>unlock()</code>.
     * Equivalent to the minimal requirement of <code>std::mutex</code>.
     * </p>
     *
     * <b>Examples:</b>
     * <ul>
     *   <li><code>std::mutex</code></li>
     *   <li><code>std::recursive_mutex</code></li>
     *   <li><code>jh::typed::null_mutex_t</code></li>
     * </ul>
     */
    template<typename M>
    concept basic_lockable = requires(M m) {
        { m.lock() } -> std::same_as<void>;
        { m.unlock() } -> std::same_as<void>;
    };

    /**
     * @brief Concept for exclusive lockable objects supporting try semantics.
     *
     * <p>
     * Extends <code>basic_lockable</code> by requiring <code>try_lock()</code>.
     * </p>
     *
     * <b>Examples:</b>
     * <ul>
     *   <li><code>std::mutex</code></li>
     *   <li><code>std::timed_mutex</code></li>
     *   <li><code>jh::typed::null_mutex_t</code></li>
     * </ul>
     */
    template<typename M>
    concept excl_lockable = basic_lockable<M> && requires(M m) {
        { m.try_lock() } -> std::convertible_to<bool>;
    };

    /**
     * @brief Concept for timed exclusive lockable objects.
     *
     * <p>
     * Extends <code>excl_lockable</code> with <code>try_lock_for()</code> and
     * <code>try_lock_until()</code> overloads.
     * </p>
     *
     * <b>Examples:</b>
     * <ul>
     *   <li><code>std::timed_mutex</code></li>
     *   <li><code>std::recursive_timed_mutex</code></li>
     *   <li><code>jh::typed::null_mutex_t</code></li>
     * </ul>
     */
    template<
            typename M,
            typename Rep = typename std::chrono::milliseconds::rep,
            typename Period = typename std::chrono::milliseconds::period,
            typename Clock = std::chrono::steady_clock,
            typename Duration = typename Clock::duration>
    concept timed_excl_lockable =
    excl_lockable<M> && requires(M m,
                                 const std::chrono::duration<Rep, Period> &rel_time,
                                 const std::chrono::time_point<Clock, Duration> &abs_time)
    {
        { m.try_lock_for(rel_time) } -> std::convertible_to<bool>;
        { m.try_lock_until(abs_time) } -> std::convertible_to<bool>;
    };

    /**
     * @brief Concept for shared (reader) lockable objects.
     *
     * <p>
     * Requires shared locking interface — <code>lock_shared()</code>,
     * <code>unlock_shared()</code>, and <code>try_lock_shared()</code>.
     * </p>
     *
     * <b>Examples:</b>
     * <ul>
     *   <li><code>std::shared_mutex</code></li>
     *   <li><code>std::shared_timed_mutex</code></li>
     * </ul>
     */
    template<typename M>
    concept shared_lockable = requires(M m) {
        { m.lock_shared() } -> std::same_as<void>;
        { m.try_lock_shared() } -> std::convertible_to<bool>;
        { m.unlock_shared() } -> std::same_as<void>;
    };

    /**
     * @brief Concept for shared lockables supporting timed acquisition.
     *
     * <p>
     * Requires <code>try_lock_shared_for()</code> and
     * <code>try_lock_shared_until()</code>.
     * </p>
     */
    template<
            typename M,
            typename Rep = typename std::chrono::milliseconds::rep,
            typename Period = typename std::chrono::milliseconds::period,
            typename Clock = std::chrono::steady_clock,
            typename Duration = typename Clock::duration>
    concept timed_shared_lockable =
    shared_lockable<M> && requires(M m,
                                   const std::chrono::duration<Rep, Period> &rel_time,
                                   const std::chrono::time_point<Clock, Duration> &abs_time)
    {
        { m.try_lock_shared_for(rel_time) } -> std::convertible_to<bool>;
        { m.try_lock_shared_until(abs_time) } -> std::convertible_to<bool>;
    };

    /**
     * @brief General mutex-like concept.
     *
     * <p>
     * Represents any object that provides exclusive or shared locking semantics.
     * Requires support for either <code>excl_lockable</code> or <code>shared_lockable</code>.
     * </p>
     */
    template<typename M>
    concept mutex_like = excl_lockable<M> || shared_lockable<M>;

    /**
     * @brief Timed variant of <code>mutex_like</code>.
     *
     * <p>
     * Requires support for either <code>timed_excl_lockable</code> or
     * <code>timed_shared_lockable</code>.
     * </p>
     */
    template<
            typename M,
            typename Rep = typename std::chrono::milliseconds::rep,
            typename Period = typename std::chrono::milliseconds::period,
            typename Clock = std::chrono::steady_clock,
            typename Duration = typename Clock::duration>
    concept timed_mutex_like =
    timed_excl_lockable<M, Rep, Period, Clock, Duration> ||
    timed_shared_lockable<M, Rep, Period, Clock, Duration>;

    /**
     * @brief Read–write (RW) mutex concept.
     *
     * <p>
     * Requires both exclusive and shared locking support.
     * </p>
     *
     * <b>Examples:</b>
     * <ul>
     *   <li><code>std::shared_mutex</code></li>
     *   <li><code>jh::typed::null_mutex_t</code></li>
     * </ul>
     */
    template<typename M>
    concept rw_mutex_like = excl_lockable<M> && shared_lockable<M>;

} // namespace jh::concepts

namespace jh {

    /**
     * @brief Trait registry for detecting counting reentrance (recursive).
     *
     * <p>
     * Specialized for standard recursive mutex types and can be extended
     * by user-defined reentrant locks.
     * </p>
     *
     * <b>Default:</b> <code>false_type</code><br>
     * <b>Specialized:</b>
     * <ul>
     *   <li><code>std::recursive_mutex</code></li>
     *   <li><code>std::recursive_timed_mutex</code></li>
     * </ul>
     */
    template<jh::concepts::mutex_like T>
    struct recursive_registry : std::false_type {
    };

    /**
     * @brief Trait registry for detecting idempotent (structural) reentrance.
     *
     * <p>
     * Used by <code>jh::typed::null_mutex_t</code> and similar structural
     * types that support safe re-locking within the same context.
     * </p>
     *
     * <b>Default:</b> <code>false_type</code><br>
     * <b>Specializable:</b> For user-defined idempotent locks.
     */
    template<jh::concepts::mutex_like T>
    struct reentrant_registry : std::false_type {
    };

    /// @brief Specialization for <code>std::recursive_mutex</code>.
    template<>
    struct recursive_registry<std::recursive_mutex> : std::true_type {
    };

    /// @brief Specialization for <code>std::recursive_timed_mutex</code>.
    template<>
    struct recursive_registry<std::recursive_timed_mutex> : std::true_type {
    };

} // namespace jh

namespace jh::concepts {

    /**
     * @brief Concept for counting reentrant (recursive) mutexes.
     *
     * <p>
     * A recursive mutex allows repeated locking by the same thread,
     * maintaining an internal depth counter. It must be unlocked the
     * same number of times as it was locked.
     * </p>
     *
     * <b>Examples:</b>
     * <ul>
     *   <li><code>std::recursive_mutex</code></li>
     *   <li><code>std::recursive_timed_mutex</code></li>
     * </ul>
     */
    template<typename M>
    concept recursive_mutex =
    mutex_like<M> &&
    (recursive_registry<std::remove_cvref_t<M>>::value || requires {
        typename M::is_recursive_tag;
        std::convertible_to<typename M::is_recursive_tag, std::true_type>;
    });

    /**
     * @brief Concept for idempotent (structurally reentrant) mutexes.
     *
     * <p>
     * A reentrant mutex allows re-locking within the same context as
     * a no-op, guaranteeing structural safety without internal counters.
     * </p>
     *
     * <b>Examples:</b>
     * <ul>
     *   <li><code>jh::typed::null_mutex_t</code></li>
     * </ul>
     */
    template<typename M>
    concept reentrant_mutex =
    mutex_like<M> &&
    (reentrant_registry<std::remove_cvref_t<M>>::value || requires {
        typename M::is_reentrant_tag;
        std::convertible_to<typename M::is_reentrant_tag, std::true_type>;
    });

    /**
     * @brief Concept for any mutex supporting some form of reentrance.
     *
     * <p>
     * A <code>reentrance_capable_mutex</code> satisfies either
     * <code>recursive_mutex</code> (counting reentrance) or
     * <code>reentrant_mutex</code> (idempotent reentrance).
     * </p>
     *
     * <b>Examples:</b>
     * <ul>
     *   <li><code>std::recursive_mutex</code></li>
     *   <li><code>jh::typed::null_mutex_t</code></li>
     * </ul>
     */
    template<typename M>
    concept reentrance_capable_mutex =
    recursive_mutex<M> || reentrant_mutex<M>;

} // namespace jh::concepts
