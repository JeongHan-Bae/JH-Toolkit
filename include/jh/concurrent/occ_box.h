/**
 * \verbatim
 * Copyright 2025 JeongHan-Bae <mastropseudo@gmail.com>
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
 * @file occ_box.h (concurrent)
 * @brief A generic container abstraction based on
 *        <strong>OCC (Optimistic Concurrency Control)</strong>.
 *
 * <h3>Concurrency control models</h3>
 * <ul>
 *   <li><strong>LBCC (Lock-Based Concurrency Control)</strong><br/>
 *       Built-in support in C++ (<code>std::mutex</code>, <code>std::shared_mutex</code>).<br/>
 *       Flexible, efficient, but requires careful lock ordering to avoid deadlocks.<br/>
 *       No wrapper provided in this library.</li>
 *
 *   <li><strong>MVCC (Multi-Version Concurrency Control)</strong><br/>
 *       Used in databases for snapshot isolation.<br/>
 *       Requires version chains, garbage collection, and complex rules.<br/>
 *       Not suitable for lightweight, in-memory concurrency here.</li>
 *
 *   <li><strong>OCC (Optimistic Concurrency Control)</strong><br/>
 *       Implemented here as <code>occ_box&lt;T&gt;</code>.<br/>
 *       Works for arbitrary copy/move-constructible types.<br/>
 *       Provides optimistic reads and atomic replacement writes.</li>
 * </ul>
 *
 * <h3>Read cost model</h3>
 * A single <code>read()</code> operation typically incurs:
 * <ul>
 *   <li>Two atomic loads of <code>shared_ptr&lt;state&gt;</code> (before/after validation).</li>
 *   <li>Two pointer dereferences (state → data → object).</li>
 *   <li>One function invocation (the user lambda).</li>
 *   <li>By default (<code>JH_OCC_ENABLE_MULTI_COMMIT == 1</code>),
 *       one extra atomic load of <code>flag_</code> is performed during validation.</li>
 * </ul>
 * Reads are wait-free, never block writes, and retries only if a concurrent commit
 * changes the <code>state</code> pointer between the two loads.
 *
 * <h3>Write semantics</h3>
 * <ul>
 *   <li><code>write()</code> always creates a fresh copy of the object,
 *       applies the user lambda, and commits with a single CAS.</li>
 *   <li><code>write_ptr()</code> allows the caller to supply a new
 *       <code>shared_ptr&lt;T&gt;</code>, avoiding deep copy overhead
 *       for large or expensive-to-copy objects.</li>
 *   <li>Both guarantee atomic replacement:
 *       no reader ever observes a partially written object.</li>
 * </ul>
 *
 * <h3>try_* methods and retries</h3>
 * <ul>
 *   <li>All <code>try_*()</code> methods perform the first attempt outside of the loop.</li>
 *   <li>This ensures that <code>retries == 1</code> executes exactly once with no loop overhead.</li>
 *   <li><code>retries == 0</code> is equivalent to <code>1</code> (a single attempt).</li>
 *   <li>For <code>retries > 1</code>, the loop covers the remaining <code>retries - 1</code> attempts.</li>
 *   <li>Implementations are intentionally not abstracted into a common helper:
 *       any such abstraction would either add an <code>std::optional</code> allocation
 *       or an extra <code>invoke</code> in the retry path, both undesirable in hot loops.</li>
 *   <li>Users may implement exponential backoff or jitter in retry lambdas to mitigate contention.
 *       See <code>examples/example_occ_box.cpp</code> for a full example. In short:
 *       the lambda can take a <code>duration&amp;</code>, sleep if nonzero, update it
 *       (0 → min → min×base … capped at max), then run business logic.</li>
 * </ul>
 *
 * <h3>Atomicity and contention</h3>
 * <ul>
 *   <li><strong>Strong atomicity</strong>: each commit replaces the entire state
 *       (data + version) with a single CAS.</li>
 *   <li>Readers are always safe: they either succeed with a consistent snapshot
 *       or retry internally.</li>
 *   <li>Writers never expose intermediate states.</li>
 *   <li>High-frequency writes may increase retries, but safety is never compromised.</li>
 *   <li>When <code>JH_OCC_ENABLE_MULTI_COMMIT == 1</code> (default),
 *       contention is resolved by strict priority:
 *       <strong>multi-write &gt; single-write &gt; read</strong>.</li>
 * </ul>
 * <h4>Multi-commit policy</h4>
 * <ul>
 *   <li>If <code>JH_OCC_ENABLE_MULTI_COMMIT</code> is <strong>1</strong> (default):
 *     <ul>
 *       <li><code>occ_box</code> supports <code>apply_to()</code> for atomic multi-box transactions.</li>
 *       <li>Each box carries an extra <code>flag_</code> (<code>atomic&lt;bool&gt;</code>) used as a transaction marker.</li>
 *       <li><code>read()</code> incurs one additional atomic load to check <code>flag_</code>.</li>
 *       <li>Conflict resolution follows: multi-write &gt; single-write &gt; read.</li>
 *     </ul>
 *   </li>
 *   <li>If <code>JH_OCC_ENABLE_MULTI_COMMIT</code> is <strong>0</strong>:
 *     <ul>
 *       <li><code>apply_to()</code> is disabled.</li>
 *       <li><code>occ_box</code> does not contain <code>flag_</code>, reducing object size.</li>
 *       <li>Single-box OCC still works, with lighter <code>read()</code> cost.</li>
 *     </ul>
 *   </li>
 * </ul>
 *
 * <h3>Design intent</h3>
 * <ul>
 *   <li>Correctness and composability over raw microsecond performance.</li>
 *   <li>Deadlock-free by design: readers never block writers, writers never block readers.</li>
 *   <li>Best suited for application-level concurrency where retries are acceptable.</li>
 * </ul>
 *
 * @version <pre>1.4.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#ifndef JH_OCC_ENABLE_MULTI_COMMIT
#define JH_OCC_ENABLE_MULTI_COMMIT 1
#endif

#include "../../../../../../../../opt/homebrew/Cellar/llvm/20.1.3/include/c++/v1/memory"
#include "../../../../../../../../opt/homebrew/Cellar/llvm/20.1.3/include/c++/v1/atomic"
#include "../../../../../../../../opt/homebrew/Cellar/llvm/20.1.3/include/c++/v1/concepts"
#include "../../../../../../../../opt/homebrew/Cellar/llvm/20.1.3/include/c++/v1/cstdint"
#include "../../../../../../../../opt/homebrew/Cellar/llvm/20.1.3/include/c++/v1/functional"
#include "../../../../../../../../opt/homebrew/Cellar/llvm/20.1.3/include/c++/v1/type_traits"
#include "../../../../../../../../opt/homebrew/Cellar/llvm/20.1.3/include/c++/v1/optional"

namespace jh::conc {
#if JH_OCC_ENABLE_MULTI_COMMIT
    namespace detail {
        template<typename... BoxTypes, typename... Funcs, std::size_t... I>
        bool apply_to_impl(std::tuple<BoxTypes &...>,
                           std::tuple<Funcs...> &&,
                           std::index_sequence<I...>);

        template<typename... BoxTypes, typename... Funcs, std::size_t... I>
        bool apply_to_ptr_impl(std::tuple<BoxTypes &...>,
                               std::tuple<Funcs...> &&,
                               std::index_sequence<I...>);
    }
#endif

    /**
     * @brief Generic container providing <strong>Optimistic Concurrency Control (OCC)</strong>.
     *
     * @tparam T Value type, must be copy- and move-constructible.
     *
     * <h4>Semantics</h4>
     * <ul>
     *   <li>Encapsulates a value of type <code>T</code> with atomic versioned state.</li>
     *   <li>Reads are wait-free: they either succeed with a consistent snapshot or retry internally.</li>
     *   <li>Writes are commit-replace: each update creates a fresh state and replaces atomically via CAS.</li>
     *   <li>No reader ever observes a partially written value.</li>
     * </ul>
     *
     * <h4>Retry model</h4>
     * <ul>
     *   <li>All <code>try_*()</code> APIs attempt once outside the loop, then retry up to N−1 times.</li>
     *   <li><code>retries == 0</code> is equivalent to one attempt.</li>
     *   <li>Backoff and jitter strategies can be layered on top (see examples).</li>
     * </ul>
     *
     * @note When <code>JH_OCC_ENABLE_MULTI_COMMIT == 1</code> (default),
     *       boxes under <code>apply_to()</code> are given priority over single writes and reads,
     *       ensuring that multi-box transactions cannot be broken by concurrent commits.
     */
    template<typename T> requires (std::is_copy_constructible_v<T> && std::is_move_constructible_v<T>)
    class occ_box final {
        /**
         * @brief Internal immutable state wrapper.
         *
         * Holds the version counter and the managed value.
         */
        struct state {
            std::uint64_t version;       ///< Monotonic version number, increments on each commit.
            std::shared_ptr<T> data;     ///< Pointer to the stored value of type <code>T</code>.
        };

        /// Current atomic state (shared and versioned).
        std::shared_ptr<state> state_;

#if JH_OCC_ENABLE_MULTI_COMMIT
        /// Transaction marker: true while box is locked by an ongoing multi-box commit.
        std::atomic<bool> flag_;

        /// @brief Grant internal helpers access to <code>state_</code> for multi-box atomic commit.
        template<typename... BoxTypes, typename... Funcs, std::size_t... I>
        friend bool detail::apply_to_impl(std::tuple<BoxTypes &...>,
                                          std::tuple<Funcs...> &&,
                                          std::index_sequence<I...>);

        /// @brief Same as <code>detail::apply_to_impl</code>, but for pointer-based updates (<code>apply_to_ptr_impl</code>).
        template<typename... BoxTypes, typename... Funcs, std::size_t... I>
        friend bool detail::apply_to_ptr_impl(std::tuple<BoxTypes &...>,
                                              std::tuple<Funcs...> &&,
                                              std::index_sequence<I...>);

#endif

    public:
        /// @brief Copy constructor: manually resets flag_ to false, cannot use =default.
        occ_box(const occ_box &other) noexcept {
            auto st = std::atomic_load_explicit(&other.state_, std::memory_order_acquire);
            std::atomic_store_explicit(&state_, st, std::memory_order_release);
#if JH_OCC_ENABLE_MULTI_COMMIT
            flag_.store(false, std::memory_order_release);
#endif
        }

        /// @brief Copy assignment: manually resets flag_ to false, cannot use =default.
        occ_box &operator=(const occ_box &other) noexcept {
            if (this != &other) {
                auto st = std::atomic_load_explicit(&other.state_, std::memory_order_acquire);
                std::atomic_store_explicit(&state_, st, std::memory_order_release);
#if JH_OCC_ENABLE_MULTI_COMMIT
                flag_.store(false, std::memory_order_release);
#endif
            }
            return *this;
        }

        /// @brief Move constructor: transfers state but resets flag_ to false, cannot use =default.
        occ_box(occ_box &&other) noexcept {
            auto st = std::atomic_load_explicit(&other.state_, std::memory_order_acquire);
            std::atomic_store_explicit(&state_, st, std::memory_order_release);
#if JH_OCC_ENABLE_MULTI_COMMIT
            flag_.store(false, std::memory_order_release);
#endif
        }

        /// @brief Move assignment: transfers state but resets flag_ to false, cannot use =default.
        occ_box &operator=(occ_box &&other) noexcept {
            if (this != &other) {
                auto st = std::atomic_load_explicit(&other.state_, std::memory_order_acquire);
                std::atomic_store_explicit(&state_, st, std::memory_order_release);
#if JH_OCC_ENABLE_MULTI_COMMIT
                flag_.store(false, std::memory_order_release);
#endif
            }
            return *this;
        }

        using value_type = T;

        /**
         * @brief Construct a new occ_box from an existing shared_ptr<T>.
         *
         * @param ptr Shared pointer to an already-constructed T.
         *
         * Takes ownership by wrapping it into the initial state
         * with version = 0.
         */
        explicit occ_box(std::shared_ptr<T> ptr) {
            std::atomic_store_explicit(
                    &state_,
                    std::make_shared<state>(state{0, std::move(ptr)}),
                    std::memory_order_release
            );
        }

        /**
         * @brief Construct a new occ_box by forwarding arguments to T.
         *
         * @tparam Args Argument types for T's constructor.
         * @param args Arguments perfectly forwarded to T's constructor.
         *
         * Initializes the internal state with version = 0
         * and a freshly constructed <code>std::shared_ptr<T></code>.
         */
        template<typename... Args>
        explicit occ_box(Args &&... args) {
            auto init = std::make_shared<T>(std::forward<Args>(args)...);
            std::atomic_store_explicit(
                    &state_,
                    std::make_shared<state>(state{0, std::move(init)}),
                    std::memory_order_release
            );
        }

        /**
         * @brief Blocking read with optimistic validation.
         *
         * @tparam F Callable type, must accept <code>const T&</code> and optional extra args.
         * @tparam Args Additional argument types.
         *
         * @param f User-provided callable, invoked with the current value.
         * @param args Optional extra arguments forwarded to the callable.
         *
         * @return <code>R</code>, the value returned by the callable, provided the read
         *         passes optimistic validation (snapshot is consistent).
         *
         * <h4>Semantics</h4>
         * <ul>
         *   <li>Performs a load–invoke–validate sequence under optimistic concurrency.</li>
         *   <li>If the state changes between two atomic loads, the read retries internally.</li>
         *   <li>Wait-free for readers: never blocks writers.</li>
         * </ul>
         *
         * <h4>Why void is disallowed</h4>
         * <p>
         * Returning <code>void</code> is forbidden because <code>read()</code>
         * must conceptually produce a value from the snapshot.
         * Allowing <code>void</code> would encourage using this API
         * solely for side effects, which violates the read model.
         * </p>
         *
         * <h4>Permitted side effects</h4>
         * <p>
         * Minor auxiliary effects (e.g. updating a <code>duration&amp;</code>
         * for backoff logic, or writing to a log) are acceptable,
         * provided they do not alter application state or depend
         * on non-idempotent behavior.
         * </p>
         * <p>
         * For output purposes, prefer returning a value
         * (e.g. a <code>std::string</code> built from an <code>ostringstream</code>)
         * instead of directly printing inside <code>read()</code>.
         * </p>
         */
        template<typename F, typename... Args>
        requires std::invocable<F, const T &, Args...> &&
                 (!std::same_as<std::invoke_result_t<F, const T &, Args...>, void>)
        [[nodiscard]] auto read(F &&f, Args &&... args) const {
            while (true) {
                auto st1 = std::atomic_load_explicit(&state_, std::memory_order_acquire);
                auto result = std::invoke(f, *st1->data, std::forward<Args>(args)...);
                auto st2 = std::atomic_load_explicit(&state_, std::memory_order_acquire);
#if JH_OCC_ENABLE_MULTI_COMMIT
                if (std::atomic_load_explicit(&flag_, std::memory_order_acquire)) {
                    continue;
                }
#endif
                if (st1 == st2) return result;
            }
        }

        /**
         * @brief Non-blocking read with limited retries.
         *
         * @tparam F Callable type, must accept <code>const T&</code> and optional extra args.
         * @tparam Args Additional argument types.
         *
         * @param f User-provided callable, invoked with the current value.
         * @param retries Maximum number of attempts (0 treated as 1).
         * @param args Optional extra arguments forwarded to the callable.
         *
         * @return <code>std::optional&lt;R&gt;</code>, where <code>R</code> is the callable's return type.
         *         Returns <code>std::nullopt</code> if all retries fail validation.
         *
         * <h4>Semantics</h4>
         * <ul>
         *   <li>Performs optimistic load–invoke–validate like <code>read()</code>.</li>
         *   <li>Unlike <code>read()</code>, it gives up after at most <code>retries</code> attempts.</li>
         *   <li>Retry count <code>0</code> is normalized to one attempt.</li>
         * </ul>
         *
         * <h4>Purity rule</h4>
         * <p>
         * Pure side-effect-only operations are disallowed:
         * this method must conceptually produce a value from the snapshot.
         * Minor auxiliary effects (e.g. backoff instrumentation, logging) are acceptable
         * if they do not alter application state.
         * </p>
         *
         * @note <code>retries</code> must be explicitly specified if extra
         *       <code>args...</code> are provided, since <code>args</code>
         *       always follow it in the parameter list.
         *
         * @see read()
         */
        template<typename F, typename... Args>
        requires std::invocable<F, const T &, Args...> &&
                 (!std::same_as<std::invoke_result_t<F, const T &, Args...>, void>)
        [[nodiscard]] auto try_read(
                F &&f, std::uint16_t retries = 1, Args &&... args) const
        -> std::optional<std::invoke_result_t<F, const T &, Args...>> {
            using R = std::invoke_result_t<F, const T &, Args...>;

            { // first attempt
                auto st1 = std::atomic_load_explicit(&state_, std::memory_order_acquire);
                R result = std::invoke(std::forward<F>(f), *st1->data, std::forward<Args>(args)...);
                auto st2 = std::atomic_load_explicit(&state_, std::memory_order_acquire);
#if JH_OCC_ENABLE_MULTI_COMMIT
                if (!std::atomic_load_explicit(&flag_, std::memory_order_acquire) && (st1 == st2)) {
                    return result;
                }
#else
                if (st1 == st2) return result;
#endif
            }

            for (std::uint32_t i = 1; i < static_cast<std::uint32_t>(retries); ++i) {
                auto st1 = std::atomic_load_explicit(&state_, std::memory_order_acquire);
                R result = std::invoke(std::forward<F>(f), *st1->data, std::forward<Args>(args)...);
                auto st2 = std::atomic_load_explicit(&state_, std::memory_order_acquire);
#if JH_OCC_ENABLE_MULTI_COMMIT
                if (std::atomic_load_explicit(&flag_, std::memory_order_acquire)) {
                    continue;
                }
#endif
                if (st1 == st2) return result;
            }
            return std::nullopt;
        }

        /**
         * @brief Blocking write with optimistic commit-replace semantics.
         *
         * @tparam F Callable type, must accept <code>T&</code> and optional extra args.
         * @tparam Args Additional argument types.
         *
         * @param f User-provided callable, applied to a fresh copy of the object.
         * @param args Optional extra arguments forwarded to the callable.
         *
         * @return void
         *
         * <h4>Semantics</h4>
         * <ul>
         *   <li>Performs a load–copy–invoke–CAS loop until commit succeeds.</li>
         *   <li>Always copies the current object before applying <code>f</code>.</li>
         *   <li>Guarantees atomic replacement: readers never see a partially written object.</li>
         * </ul>
         *
         * <h4>Performance notes</h4>
         * <ul>
         *   <li>Safe under high contention: even if writes are very frequent, each commit
         *       is strictly atomic and never exposes torn or inconsistent states.</li>
         *   <li>Excessive use may hurt performance due to repeated deep copies and CAS retries,
         *       but correctness and race-freedom are guaranteed.</li>
         *   <li>Prefer embedding repeated logic inside <code>f</code> rather than
         *       invoking <code>write()</code> repeatedly in a loop.</li>
         *   <li>If deep copies are undesirable, consider <code>write_ptr()</code>
         *       to construct and install a new object directly.</li>
         * </ul>
         *
         * <h4>Return semantics</h4>
         * <p>
         * This method always returns <code>void</code>.
         * The write is guaranteed to complete before returning.
         * It is best suited for critical update paths where completion is mandatory.
         * </p>
         *
         * <h4>Fairness</h4>
         * <p>
         * On most POSIX platforms, the scheduler tends to grant forward progress,
         * so livelock is practically avoided.
         * However, applications should not over-rely on this property.
         * </p>
         *
         * @see write_ptr()
         */
        template<typename F, typename... Args>
        requires std::invocable<F, T &, Args...> &&
                 std::same_as<std::invoke_result_t<F, T &, Args...>, void>
        void write(F &&f, Args &&... args) {
            while (true) {
                auto old = std::atomic_load_explicit(&state_, std::memory_order_acquire);
                auto new_data = std::make_shared<T>(*old->data);
                std::invoke(std::forward<F>(f), *new_data, std::forward<Args>(args)...);

                auto new_state = std::make_shared<state>(state{old->version + 1, std::move(new_data)});

                auto expected = old;
#if JH_OCC_ENABLE_MULTI_COMMIT
                if (std::atomic_load_explicit(&flag_, std::memory_order_acquire)) {
                    continue;
                }
#endif
                if (std::atomic_compare_exchange_strong_explicit(
                        &state_, &expected, new_state,
                        std::memory_order_acq_rel, std::memory_order_acquire)) {
                    return;
                }
            }
        }

        /**
         * @brief Non-blocking write with limited retries.
         *
         * @tparam F Callable type, must accept <code>T&</code> and optional extra args.
         * @tparam Args Additional argument types.
         *
         * @param f User-provided callable, applied to a freshly copied value.
         * @param retries Maximum number of attempts (0 treated as 1).
         * @param args Optional extra arguments forwarded to the callable.
         *
         * @return <code>true</code> if the update is committed successfully,
         *         <code>false</code> if all attempts fail due to contention.
         *
         * <h4>Semantics</h4>
         * <ul>
         *   <li>Each attempt loads the current state, copies the value,
         *       applies the user function, and tries to commit via CAS.</li>
         *   <li>Fails if another writer replaces the state before CAS succeeds.</li>
         *   <li>Unlike <code>write()</code>, this method does not spin indefinitely:
         *       it retries at most <code>retries</code> times.</li>
         * </ul>
         *
         * <h4>Copy semantics</h4>
         * <p>
         * Each attempt deep-copies the underlying value.
         * If deep copies are undesirable, consider
         * <code>try_write_ptr()</code> to construct and install a new
         * object directly.
         * </p>
         *
         * <h4>Usage note</h4>
         * <p>
         * For retryable operations or performance-sensitive paths,
         * prefer <code>try_write()</code> or <code>try_write_ptr()</code>
         * over their blocking counterparts, since they allow graceful
         * failure handling under contention.
         * </p>
         *
         * @note <code>retries</code> must be explicitly specified if extra
         *       <code>args...</code> are provided, since <code>args</code>
         *       always follow it in the parameter list.
         *
         * @see write()
         */
        template<typename F, typename... Args>
        requires std::invocable<F, T &, Args...> &&
                 std::same_as<std::invoke_result_t<F, T &, Args...>, void>
        [[nodiscard]] bool try_write(F &&f, std::uint16_t retries = 1, Args &&... args) {
            { // first attempt
                auto old = std::atomic_load_explicit(&state_, std::memory_order_acquire);
                auto new_data = std::make_shared<T>(*old->data);
                std::invoke(std::forward<F>(f), *new_data, std::forward<Args>(args)...);

                auto new_state = std::make_shared<state>(state{old->version + 1, std::move(new_data)});
                auto expected = old;
#if JH_OCC_ENABLE_MULTI_COMMIT
                if (!std::atomic_load_explicit(&flag_, std::memory_order_acquire)
                    && (std::atomic_compare_exchange_strong_explicit(
                        &state_, &expected, new_state,
                        std::memory_order_acq_rel, std::memory_order_acquire))) {
                    return true;
                }
#else
                if (std::atomic_compare_exchange_strong_explicit(
                        &state_, &expected, new_state,
                        std::memory_order_acq_rel, std::memory_order_acquire)) {
                    return true;
                    }
#endif

            }
            for (std::uint32_t i = 1; i < static_cast<std::uint32_t>(retries); ++i) {
                auto old = std::atomic_load_explicit(&state_, std::memory_order_acquire);
                auto new_data = std::make_shared<T>(*old->data);
                std::invoke(std::forward<F>(f), *new_data, std::forward<Args>(args)...);

                auto new_state = std::make_shared<state>(state{old->version + 1, std::move(new_data)});
                auto expected = old;
                if (std::atomic_compare_exchange_strong_explicit(
                        &state_, &expected, new_state,
                        std::memory_order_acq_rel, std::memory_order_acquire)) {
                    return true;
                }
            }
            return false;
        }

        /**
         * @brief Blocking write using pointer replacement.
         *
         * @tparam F Callable type, must accept <code>const std::shared_ptr&lt;T&gt;&</code>
         *           and optional extra args, and return a new <code>std::shared_ptr&lt;T&gt;</code>.
         * @tparam Args Additional argument types.
         *
         * @param f User-provided callable, invoked with the old shared_ptr.
         *          Must return a new <code>shared_ptr</code> holding the replacement object.
         * @param args Optional extra arguments forwarded to the callable.
         *
         * @return void
         *
         * <h4>Semantics</h4>
         * <ul>
         *   <li>Mechanism is the same as <code>write()</code>, but avoids deep copies.</li>
         *   <li>Each attempt invokes the user function to construct a brand-new object
         *       and commits it atomically via CAS.</li>
         *   <li>Retries indefinitely until success.</li>
         * </ul>
         *
         * <h4>Recommended scenarios</h4>
         * <ul>
         *   <li>When the object contains fields that can be safely discarded
         *       (e.g. large buffers or caches you don’t need to preserve).</li>
         *   <li>When the object has resizable members (like <code>std::vector</code>
         *       or <code>std::string</code>), and constructing directly at the new
         *       size is cheaper than copying and then resizing.</li>
         * </ul>
         *
         * @see write(), try_write_ptr()
         */
        template<typename F, typename... Args>
        requires std::invocable<F, const std::shared_ptr<T> &, Args...> &&
                 std::same_as<std::invoke_result_t<F, const std::shared_ptr<T> &, Args...>, std::shared_ptr<T>>
        void write_ptr(F &&f, Args &&... args) {
            while (true) {
                auto old = std::atomic_load_explicit(&state_, std::memory_order_acquire);
                auto new_data = std::invoke(std::forward<F>(f), old->data, std::forward<Args>(args)...);
                auto new_state = std::make_shared<state>(state{old->version + 1, std::move(new_data)});

                auto expected = old;
#if JH_OCC_ENABLE_MULTI_COMMIT
                if (std::atomic_load_explicit(&flag_, std::memory_order_acquire)) {
                    continue;
                }
#endif
                if (std::atomic_compare_exchange_strong_explicit(
                        &state_, &expected, new_state,
                        std::memory_order_acq_rel, std::memory_order_acquire)) {
                    return;
                }
            }
        }

        /**
         * @brief Non-blocking pointer-based write with limited retries.
         *
         * @tparam F Callable type, must accept <code>const std::shared_ptr&lt;T&gt;&</code>
         *           and optional extra args, and return a new <code>std::shared_ptr&lt;T&gt;</code>.
         * @tparam Args Additional argument types.
         *
         * @param f User-provided callable, invoked with the old shared_ptr.
         *          Must return a new <code>shared_ptr</code> holding the replacement object.
         * @param retries Maximum number of attempts (0 treated as 1).
         * @param args Optional extra arguments forwarded to the callable.
         *
         * @return <code>true</code> if the update is committed successfully,
         *         <code>false</code> if all attempts fail due to contention.
         *
         * <h4>Semantics</h4>
         * <ul>
         *   <li>Similar to <code>try_write()</code>, but avoids copying the old object.</li>
         *   <li>Each attempt calls the user function to produce a fresh object
         *       and tries to commit it with CAS.</li>
         *   <li>Stops after at most <code>retries</code> attempts.</li>
         * </ul>
         *
         * <h4>Recommended scenarios</h4>
         * <ul>
         *   <li>When replacing large objects where copying is wasteful.</li>
         *   <li>When constructing a new object directly is cheaper than mutating a copy
         *       (e.g. resizing a string or vector before applying logic).</li>
         * </ul>
         *
         * @note <code>retries</code> must be explicitly specified if extra
         *       <code>args...</code> are provided, since <code>args</code>
         *       always follow it in the parameter list.
         *
         * @see try_write(), write_ptr()
         */
        template<typename F, typename... Args>
        requires std::invocable<F, const std::shared_ptr<T> &, Args...> &&
                 std::same_as<std::invoke_result_t<F, const std::shared_ptr<T> &, Args...>, std::shared_ptr<T>>
        [[nodiscard]] bool try_write_ptr(F &&f, std::uint16_t retries = 1, Args &&... args) {
            { // first attempt
                auto old = std::atomic_load_explicit(&state_, std::memory_order_acquire);
                auto new_data = std::invoke(std::forward<F>(f), old->data, std::forward<Args>(args)...);
                auto new_state = std::make_shared<state>(state{old->version + 1, std::move(new_data)});

                auto expected = old;
#if JH_OCC_ENABLE_MULTI_COMMIT
                if (!std::atomic_load_explicit(&flag_, std::memory_order_acquire)
                    && (std::atomic_compare_exchange_strong_explicit(
                        &state_, &expected, new_state,
                        std::memory_order_acq_rel, std::memory_order_acquire))) {
                    return true;
                }
#else
                if (std::atomic_compare_exchange_strong_explicit(
                        &state_, &expected, new_state,
                        std::memory_order_acq_rel, std::memory_order_acquire)) {
                    return true;
                }
#endif
            }
            for (std::uint32_t i = 1; i < static_cast<std::uint32_t>(retries); ++i) {
                auto old = std::atomic_load_explicit(&state_, std::memory_order_acquire);
                auto new_data = std::invoke(std::forward<F>(f), old->data, std::forward<Args>(args)...);
                auto new_state = std::make_shared<state>(state{old->version + 1, std::move(new_data)});

                auto expected = old;
#if JH_OCC_ENABLE_MULTI_COMMIT
                if (std::atomic_load_explicit(&flag_, std::memory_order_acquire)) {
                    continue;
                }
#endif
                if (std::atomic_compare_exchange_strong_explicit(
                        &state_, &expected, new_state,
                        std::memory_order_acq_rel, std::memory_order_acquire)) {
                    return true;
                }
            }
            return false;
        }

        /**
         * @brief Get the current version counter of the box.
         *
         * @return The version number associated with the current state.
         *
         * <h4>Semantics</h4>
         * <ul>
         *   <li>Each successful commit increments the version counter.</li>
         *   <li>The counter is of type <code>std::uint64_t</code> and may wrap
         *       around on overflow.</li>
         *   <li>To detect change, compare versions with <code>!=</code> rather
         *       than ordering (<code>&gt;</code>/<code>&lt;</code>).</li>
         *   <li>The version counter advances exactly once per successful commit,
         *       with no data races or partial state exposure.</li>
         * </ul>
         */
        [[nodiscard]] std::uint64_t get_version() const noexcept {
            return std::atomic_load_explicit(&state_, std::memory_order_acquire)->version;
        }
    };

#if JH_OCC_ENABLE_MULTI_COMMIT
    namespace detail {

        /**
         * @brief Internal helper for multi-box apply (by copy semantics).
         *
         * Captures current states, makes deep copies, applies user functions,
         * validates consistency, and commits via CAS.
         *
         * @return true if commit succeeds, false otherwise.
         *
         * @note Intended for internal use only. Users should call
         *       <code>apply_to()</code> instead.
         */
        template<typename... BoxTypes, typename... Funcs, std::size_t... I>
        bool apply_to_impl(std::tuple<BoxTypes &...> boxes_tuple,
                           std::tuple<Funcs...> &&funcs,
                           std::index_sequence<I...>) {
            // Step 0. Try to acquire all flags for the transaction
            auto try_lock_all = [&](auto &&... boxes) -> bool {
                bool success = true;
                // Expand the parameter pack and attempt each lock one by one
                (void) std::initializer_list<int>{
                        ([&] {
                            if (success) {
                                bool expected = false;
                                // Attempt to set the flag from false -> true
                                if (!boxes.flag_.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
                                    // If any lock acquisition fails, roll back all previously acquired flags
                                    ((boxes.flag_.store(false, std::memory_order_release)), ...);
                                    success = false;
                                }
                            }
                            return 0; // required to satisfy initializer_list<int>
                        }(), 0)...
                };
                return success; // true if all flags acquired, false if any failed
            };

            // Abort the transaction immediately if not all flags could be acquired
            if (!try_lock_all(std::get<I>(boxes_tuple)...)) {
                return false;
            }

            // Step 1. Capture old states
            auto old_states = std::tuple{
                    std::atomic_load_explicit(&std::get<I>(boxes_tuple).state_, std::memory_order_acquire)...
            };

            // Step 2. Create deep copies of the underlying values
            auto new_data = std::tuple{
                    std::make_shared<typename std::remove_reference_t<BoxTypes>::value_type>(
                            *std::get<I>(old_states)->data
                    )...
            };

            // Step 3. Apply user functions to the copies
            (std::get<I>(std::move(funcs))(*std::get<I>(new_data)), ...);

            // Step 4. Construct new states with incremented version
            auto new_states = std::tuple{
                    std::make_shared<typename std::remove_reference_t<BoxTypes>::state>(
                            typename std::remove_reference_t<BoxTypes>::state{
                                    std::get<I>(old_states)->version + 1,
                                    std::move(std::get<I>(new_data))
                            }
                    )...
            };

            // Step 5. Validate that states were not concurrently modified
            bool ok = ((std::atomic_load_explicit(&std::get<I>(boxes_tuple).state_, std::memory_order_acquire)
                        == std::get<I>(old_states)) && ...);
            if (!ok) return false;

            // Step 6. Commit all new states with CAS
            bool cas_ok = true;
            ((cas_ok &= std::atomic_compare_exchange_strong_explicit(
                    &std::get<I>(boxes_tuple).state_,
                    &std::get<I>(old_states),
                    std::get<I>(new_states),
                    std::memory_order_acq_rel,
                    std::memory_order_acquire
            )), ...);

            // unlock the lock-flags
            (std::get<I>(boxes_tuple).flag_.store(false, std::memory_order_release), ...);

            return cas_ok;
        }

        /**
         * @brief Internal helper for multi-box apply (by pointer semantics).
         *
         * Captures current states, invokes user functions to produce new
         * <code>shared_ptr&lt;T&gt;</code>, validates consistency, and
         * commits via CAS.
         *
         * @return true if commit succeeds, false otherwise.
         *
         * @note Intended for internal use only. Users should call
         *       <code>apply_to()</code> instead.
         */
        template<typename... BoxTypes, typename... Funcs, std::size_t... I>
        bool apply_to_ptr_impl(std::tuple<BoxTypes &...> boxes_tuple,
                               std::tuple<Funcs...> &&funcs,
                               std::index_sequence<I...>) {
            // Step 0. Try to acquire all flags for the transaction
            auto try_lock_all = [&](auto &&... boxes) -> bool {
                bool success = true;
                // Expand the parameter pack and attempt each lock one by one
                (void) std::initializer_list<int>{
                        ([&] {
                            if (success) {
                                bool expected = false;
                                // Attempt to set the flag from false -> true
                                if (!boxes.flag_.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
                                    // If any lock acquisition fails, roll back all previously acquired flags
                                    ((boxes.flag_.store(false, std::memory_order_release)), ...);
                                    success = false;
                                }
                            }
                            return 0; // required to satisfy initializer_list<int>
                        }(), 0)...
                };
                return success; // true if all flags acquired, false if any failed
            };

            // Abort the transaction immediately if not all flags could be acquired
            if (!try_lock_all(std::get<I>(boxes_tuple)...)) {
                return false;
            }

            // Step 1. Capture old states
            auto old_states = std::tuple{
                    std::atomic_load_explicit(&std::get<I>(boxes_tuple).state_, std::memory_order_acquire)...
            };

            // Step 2. Invoke user functions to generate new shared_ptr<T>
            auto new_data = std::tuple{
                    std::get<I>(std::move(funcs))(std::get<I>(old_states)->data)...
            };

            // Step 3. Construct new states with incremented version
            auto new_states = std::tuple{
                    std::make_shared<typename std::remove_reference_t<BoxTypes>::state>(
                            typename std::remove_reference_t<BoxTypes>::state{
                                    std::get<I>(old_states)->version + 1,
                                    std::move(std::get<I>(new_data))
                            }
                    )...
            };

            // Step 4. Validate that states were not concurrently modified
            bool ok = ((std::atomic_load_explicit(&std::get<I>(boxes_tuple).state_, std::memory_order_acquire)
                        == std::get<I>(old_states)) && ...);
            if (!ok) return false;

            // Step 5. Commit all new states with CAS
            bool cas_ok = true;
            ((cas_ok &= std::atomic_compare_exchange_strong_explicit(
                    &std::get<I>(boxes_tuple).state_,
                    &std::get<I>(old_states),
                    std::get<I>(new_states),
                    std::memory_order_acq_rel,
                    std::memory_order_acquire
            )), ...);

            // unlock the lock-flags
            (std::get<I>(boxes_tuple).flag_.store(false, std::memory_order_release), ...);

            return cas_ok;
        }

        /// @brief Concept: callable taking <code>T&</code> and returning <code>void</code>.
        template<typename F, typename T>
        concept single_arg_void_func =
        std::invocable<F, T &> &&
        std::same_as<std::invoke_result_t<F, T &>, void>;

        /// @brief Concept: callable taking <code>const std::shared_ptr<T>&</code> and returning <code>std::shared_ptr<T></code>.
        template<typename F, typename T>
        concept ptr_func =
        std::invocable<F, const std::shared_ptr<T> &> &&
        std::same_as<std::invoke_result_t<F, const std::shared_ptr<T> &>, std::shared_ptr<T>>;

        /// @brief Checks if a tuple of functions matches boxes by value (<code>copy</code> semantics).
        template<typename BoxesTuple, typename FuncsTuple, std::size_t... I>
        constexpr bool funcs_match_boxes(std::index_sequence<I...>) {
            return (single_arg_void_func<
                    std::tuple_element_t<I, FuncsTuple>,
                    typename std::remove_reference_t<
                            std::tuple_element_t<I, BoxesTuple>
                    >::value_type
            > && ...);
        }

        /// @brief Checks if a tuple of functions matches boxes by pointer (<code>shared_ptr</code> semantics).
        template<typename BoxesTuple, typename FuncsTuple, std::size_t... I>
        constexpr bool funcs_match_boxes_t_ptrs(std::index_sequence<I...>) {
            return (ptr_func<
                    std::tuple_element_t<I, FuncsTuple>,
                    typename std::remove_reference_t<
                            std::tuple_element_t<I, BoxesTuple>
                    >::value_type
            > && ...);
        }
    }

    /**
     * @brief Apply functions to multiple <code>occ_box</code>es atomically.
     *
     * <h3>Choosing between copy-based and shared_ptr-based apply_to</h3>
     *
     * <ul>
     *   <li><strong>Copy-based</strong>
     *     <ul>
     *       <li>Each box value is deep-copied before applying the function.</li>
     *       <li>Best suited for small or trivially copyable types.</li>
     *       <li>Ensures that modifications are isolated until commit.</li>
     *       <li>Functions must be of type <code>void(T&)</code>.</li>
     *     </ul>
     *   </li>
     *
     *   <li><strong>Shared_ptr-based</strong>
     *     <ul>
     *       <li>The function constructs a new <code>shared_ptr&lt;T&gt;</code>
     *           instead of copying the old object.</li>
     *       <li>Best for large or complex types where deep copies are expensive.</li>
     *       <li>When mixing small and large objects in one transaction,
     *           prefer the shared_ptr-based version for consistency and speed.</li>
     *       <li>Functions must be of type
     *           <code>std::shared_ptr&lt;T&gt;(const std::shared_ptr&lt;T&gt;&)</code>.</li>
     *     </ul>
     *   </li>
     * </ul>
     *
     * <p>
     * Both styles are mutually exclusive for a single transaction.
     * Attempting to mix them will fail at compile time.
     * </p>
     *
     * @tparam Boxes   The set of occ_box types
     * @tparam Funcs   The set of functions (one per box)
     *
     * @param boxes    Tuple of references to occ_boxes
     * @param funcs    Tuple of functions to apply
     *
     * @return <code>true</code> if commit succeeds, <code>false</code> otherwise.
     */
    template<typename... Boxes, typename... Funcs>
    requires ((sizeof...(Boxes) == sizeof...(Funcs)) &&
              (detail::funcs_match_boxes<std::tuple<Boxes...>, std::tuple<Funcs...>>(
                      std::index_sequence_for<Boxes...>{}) ||
               detail::funcs_match_boxes_t_ptrs<std::tuple<Boxes...>, std::tuple<Funcs...>>(
                       std::index_sequence_for<Boxes...>{})))
    bool apply_to(std::tuple<Boxes &...> boxes, std::tuple<Funcs...> &&funcs) {
        if constexpr (detail::funcs_match_boxes<std::tuple<Boxes...>, std::tuple<Funcs...>>(
                std::index_sequence_for<Boxes...>{})) {
            // Copy-based
            return detail::apply_to_impl(boxes, std::move(funcs), std::index_sequence_for<Boxes...>{});
        } else {
            // Shared_ptr-based
            return detail::apply_to_ptr_impl(boxes, std::move(funcs), std::index_sequence_for<Boxes...>{});
        }
    }

#endif

}
