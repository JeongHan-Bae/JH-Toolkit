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
 * @file slot.h
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 * @brief
 * Coroutine-based asynchronous <code>slot</code>/<code>listener</code>/<code>event_signal</code> system with
 * <tt>One-Slot-Per-Hub</tt> semantics, multi-listener <tt>fan-in</tt>, and user-controlled
 * <tt>fan-out</tt> logic inside the <tt>slot</tt> coroutine.
 *
 * <h3>Overview</h3>
 * <p>
 * This module defines a minimal coroutine-driven event dispatch mechanism.
 * A <code>slot</code> is a coroutine that represents arbitrary user-defined
 * behavior. A <code>listener&lt;T&gt;</code> is an awaitable endpoint that
 * provides values of type <code>T</code> into the <code>slot</code> when an event is emitted.
 * </p><p>
 * A <code>slot_hub</code> manages synchronization, timeout behavior, and
 * the <tt>One-to-One</tt> binding between the <tt>hub</tt> and a single <tt>slot</tt> . All listeners created
 * from a <code>slot_hub</code> forward events only to the <code>slot</code> bound to that <code>slot_hub</code>.
 * </p>
 *
 * <h4>Design Notes</h4>
 * <ul>
 *   <li>
 *     <strong><code>slot_hub</code> &harr; <code>slot</code> is strictly <tt>One-to-One</tt>.</strong><br>
 *     A <code>slot_hub</code> can bind exactly one slot. All listeners made from that <code>slot_hub</code> always
 *     deliver values to that same <code>slot</code>.
 *     <br>
 *     <tt>hub</tt> stands for "synchronization domain/kernel," and <tt>slot</tt> stands for "unique control coroutine."
 *     <br>
 *     We allow multiple pairs of <tt>hubs</tt> and <tt>slots</tt>, but there is a <tt>One-to-One</tt> correspondence
 *     between <tt>hubs</tt> and <tt>slots</tt>.
 *   </li>
 *   <li>
 *     <strong>One <code>slot</code> &rarr; many <code>listener</code>s.</strong><br>
 *     Multiple <code>listener</code>s are being monitored at different stages or under different conditions.
 *     Listening to multiple listeners within the same stage constitutes a synchronization
 *     barrier (semantically incorrect).
 *   </li>
 *   <li>
 *     <strong>One <code>listener</code> &rarr; many <code>event_signal</code>s (fan-in).</strong><br>
 *     A <code>listeners</code> may be connected to multiple <code>event_signal</code> objects.
 *     All <tt>signals</tt> write into the same <tt>inbox</tt> and attempt to resume the <code>slot</code>.
 *     The user can distinguish signal sources by encoding IDs/tags in the payload.
 *   </li>
 *   <li>
 *     <strong>No fan-out in <tt>signals</tt> &mdash; fan-out belongs to the <code>slot</code>.</strong><br>
 *     <code>event_signal</code> performs only "push to listener".
 *     Routing, filtering, branching, switching, multi-stage flow control,
 *     and fan-out behavior are entirely user-defined inside the <tt>slot</tt> coroutine.
 *   </li>
 *   <li>
 *     <strong>Listener <tt>inbox</tt> is a one-shot handoff <tt>slot</tt> , not a buffer.</strong><br>
 *     A value is written into the <tt>inbox</tt> only if the <code>listener</code> succeeds in acquiring
 *     the hub's mutex within the timeout window. Once written, the <code>slot</code> is resumed
 *     immediately, consumes the value during <code>await_resume()</code>, and the
 *     inbox is cleared.
 *     <br>
 *     Because the write and the <tt>slot</tt> resumption occur while holding the same lock,
 *     the inbox is <b>never</b> overwritten, <b>never</b> accumulates unread entries, and <b>never</b>
 *     loses values due to replacement. If the lock cannot be acquired, <b>nothing</b> is
 *     written at all.
 *     <br>
 *     This implements a high-pressure fuse: either the event is <b>fully delivered</b>
 *     <pre>write &rarr; resume &rarr; consume &rarr; clear</pre>
 *     or <b>not delivered</b> at all.
 *   </li>
 *   <li>
 *     <strong><code>spawn()</code> binds the <code>slot</code> to the calling thread.</strong><br>
 *     After <code>spawn()</code>, all event-triggered resumes occur on that same thread.
 *   </li>
 *   <li>
 *     <strong>Unified lifetime.</strong><br>
 *     <code>slot</code>, <code>slot_hub</code>, and all <code>listener</code>s are expected to
 *     share the same lifetime. Moving a <code>slot</code> after binding/spawn may break this
 *     constraint and <b>must be avoided</b>. <code>event_signal</code> must not outlive its
 *     mapped <code>listener</code>.
 *   </li>
 * </ul>
 *
 * <h4>Usage Model</h4>
 * <p>
 * The usage pattern is conceptually divided into two independent parts.
 * Each part has its own internal ordering constraints, while the two parts
 * themselves do not impose ordering constraints on each other.
 * </p>
 *
 * <h5>Part 1 &mdash; Infrastructure Construction</h5>
 * <p>
 * These steps must occur in the following order:
 * </p>
 * <ol>
 *   <li>Create a <code>slot_hub</code>.</li>
 *   <li>Create one or more <code>listener</code>s from the hub.</li>
 *   <li>Create a <tt>slot</tt> coroutine that observes these <code>listener</code>s.</li>
 *   <li>Bind the <code>slot</code> to the <tt>hub</tt> via <code>bind_slot()</code>.</li>
 *   <li>Call <code>spawn()</code> to start the coroutine and bind it to the
 *       calling thread.</li>
 * </ol>
 *
 * <h5>Part 2 &mdash; Event Binding and Dispatching</h5>
 * <ol>
 *   <li>A <code>listener</code> must already exist before connecting <code>event_signal</code>s to it.</li>
 *   <li><code>event_signal::connect(listener*)</code> must be called before the
 *       first <code>emit()</code> targeting that listener.</li>
 *   <li><code>emit()</code> should be called after <code>slot.spawn()</code> was called.</li>
 * </ol>
 *
 * <p>
 * Advanced patterns (multi-signal, switching, state machines, routing, phase
 * transitions, conditional awaits) are implemented entirely in the <tt>slot</tt> coroutine.
 * The library provides only the suspension/resume primitives.
 * </p>
 *
 * <h4>Mechanics of Slot Coroutines</h4>
 * <ol>
 *   <li>The <code>slot</code> is created and suspended at <code>initial_suspend()</code>.</li>
 *   <li>The <code>slot_hub</code> binds exactly one <code>slot</code>; <code>listener</code>s use this binding.</li>
 *   <li><code>spawn()</code> resumes the coroutine for the first time.</li>
 *   <li><code>co_await listener</code> suspends the coroutine and registers its handle.</li>
 *   <li>
 *      An <code>event_signal</code> writes into the listener <tt>inbox</tt> and resumes
 *      the <code>slot</code> after acquiring the hub's timed mutex.
 *   </li>
 *   <li>
 *      <code>co_yield {}</code> yields a <code>monostate</code> and resumes
 *      execution immediately &mdash; useful for deterministic scheduling points.
 *   </li>
 *   <li>Normal completion enters <code>final_suspend()</code>.</li>
 * </ol>
 *
 * <h4>Queuing and Backpressure</h4>
 * Traditional Buffer Queues are presenting a synchronous mechanism, not an asynchronous
 * one, which is why we reject them.
 * <br>
 * <code>std::timed_mutex</code> is essentially a timed queuing aid with timeout
 * (overpressure) circuit breaking to ensure that emit is invoked according to the order of calls.
 * <br>
 * <table>
 *   <thead>
 *     <tr>
 *       <th>Aspect</th>
 *       <th>Lock Queues</th>
 *       <th>Buffer Queues</th>
 *     </tr>
 *   </thead>
 *   <tbody>
 *     <tr>
 *       <td>Data Buffering</td>
 *       <td><strong>No</strong> &mdash; values are written only if the lock is acquired; at most one in-flight value</td>
 *       <td><strong>Yes</strong> &mdash; values are enqueued and buffered until consumed</td>
 *     </tr>
 *     <tr>
 *       <td>Queuing Mechanism</td>
 *       <td><strong>FIFO lock waiters</strong> &mdash; emitters are queued implicitly via the mutex</td>
 *       <td><strong>Explicit data queue</strong> &mdash; typically implemented via ring buffer or linked list</td>
 *     </tr>
 *     <tr>
 *       <td>Overflow Control</td>
 *       <td><strong>Timeout</strong> &mdash; emit is rejected if lock cannot be acquired in time</td>
 *       <td><strong>Unbounded or bounded buffer</strong> &mdash; may require manual pressure control</td>
 *     </tr>
 *     <tr>
 *       <td>Resume Semantics</td>
 *       <td><strong>Inline</strong> &mdash; <tt>slot</tt> is resumed immediately after value is written</td>
 *       <td><strong>Out-of-band</strong> &mdash; consumer must poll or wait</td>
 *     </tr>
 *     <tr>
 *       <td>Synchronization Role</td>
 *       <td><strong>Integral</strong> &mdash; part of the event delivery protocol</td>
 *       <td><strong>Separate</strong> &mdash; often requires condition variables or manual signaling</td>
 *     </tr>
 *     <tr>
 *       <td>Fan-in Behavior</td>
 *       <td><strong>Yes</strong> &mdash; multiple signals may target a single listener via mutex arbitration</td>
 *       <td><strong>Possible</strong> &mdash; but usually requires a multiplexer</td>
 *     </tr>
 *     <tr>
 *       <td>Fan-out Support</td>
 *       <td><strong>No</strong> &mdash; deliberate omission; routing is done inside slot</td>
 *       <td><strong>Optional</strong> &mdash; may push to multiple consumers</td>
 *     </tr>
 *   </tbody>
 * </table>
 *
 * <h4>Important Constraints</h4>
 * <ul>
 *   <li><code>slot_hub</code> may bind only one <code>slot</code>.</li>
 *   <li><code>event_signal</code> must not outlive the <code>listener</code> it is connected to.</li>
 *   <li><code>slot</code>, <code>slot_hub</code>, and <code>listener</code>s must share the same overall lifetime.</li>
 *   <li><code>slot</code> alone is responsible for any fan-out or complex routing logic.</li>
 * </ul>
 *
 * <h4>Example Usage</h4>
 * @code
 * jh::async::slot_hub hub(std::chrono::milliseconds(TIMEOUT_MS));
 * 
 * auto make_slot = [...](jh::async::listener&lt;T&gt; &li, ...) -&gt; jh::async::slot {
 *     for (;;) {
 *         auto v1 = co_await li;
 *         ...
 *         co_yield {};
 *     }
 *     ...
 * };
 * 
 * auto listener1 = hub.make_listener&lt;T&gt;();
 * jh::async::event_signal&lt;T&gt; sig1;
 * sig1.connect(&listener1);
 * jh::async::slot s = make_slot(listener1, ...);
 * hub.bind_slot(s);
 *
 * s.spawn();
 * // now, emitting events will resume the <tt>slot</tt> coroutine
 * @endcode
 *
 * @version <pre>1.4.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <coroutine>
#include <optional>
#include <mutex>
#include <chrono>
#include <functional>
#include "jh/typed"

namespace jh::async {

    template<typename T>
    class listener;

    /**
     * @brief
     * Coroutine representing the user-defined asynchronous state machine.
     *
     * @details
     * A <code>slot</code> is the <b>only</b> execution context of the entire "slot-listener-signal"
     * system. It defines the state machine, phase switching, routing, filtering,
     * and fan-out logic. A <code>slot</code> always runs on the thread where <code>spawn()</code>
     * is invoked.
     * </p>
     *
     * <h4>Strong Synchronization Semantics</h4>
     * <p>
     * A <code>slot</code> may suspend on <em>exactly one</em> listener at any time.
     * Each <code>co_await</code> represents one logical synchronous step.
     * </p><p>
     * <strong>Awaiting multiple listeners inside the same loop is technically
     * idempotent per iteration but semantically meaningless</strong>, because this
     * requires the external event sources to be <em>strictly time-aligned</em>.
     * If external producers drift in timing (which is unavoidable in real-time
     * systems), the synchronous semantics break immediately.
     * </p><p>
     * In other words, the following pattern is strongly discouraged:
     * @code
     * auto a = co_await A;
     * auto b = co_await B;
     * @endcode
     * This is irrelevant to our implementation.
     * Semantically, asynchronous operations always perform triggering and consuming.
     * </p><p>
     * In other words, the discouraged behavior implies a sync-barrier semantic.
     * This means that if any listener is advancing too rapidly and resumes two or more times
     * before the other listener resumes even once in the same iteration, the state machine is broken.
     * </p><p>
     * According to the C++20 coroutine model, if you want to guarantee the correctness of this behavior,
     * then you need external synchronization; that maps to sync-barrier rather than async-resuming.
     * </p><p>
     * However, If a user truly has perfectly time-aligned external timing, then the
     * recommended design is:
     * <ul>
     *   <li><strong>Use one <code>event_signal</code> to emit a <code>tuple</code></strong> containing
     *     all values needed for that round.</li>
     *   <li>Do <strong>not</strong> await multiple listeners in the same phase.</li>
     * </ul>
     * </p>
     * <p>In summary, this behavior is semantically determined and implementation-independent.
     * If you need to observe multiple input sources in the same logical step,
     * then it's fan-in, see <code>listener</code>.</p>
     *
     * <h4>Correct Usage of Multiple Listeners</h4>
     * <p>
     * Multiple <code>listener</code>s exist <strong>for different phases or different conditions</strong>,
     * not for parallel waiting. A <code>slot</code> typically performs:
     * </p>
     *
     * @code
     * // Phase 1
     * for (;;) {
     *     auto v = co_await listener_A;
     *     if (v == STOP) break;
     * }
     *
     * // Phase 2
     * for (;;) {
     *     auto s = co_await listener_B;
     * }
     * @endcode
     *
     * <h4>Lifetime</h4>
     * <ul>
     *   <li>Must not be moved after binding to a hub.</li>
     *   <li><code>slot</code>, <code>slot_hub</code>, and <code>listener</code>s must share unified lifetime.</li>
     * </ul>
     *
     * @note
     * <code>co_yield {}</code> returns an empty sentinel value (<code>jh::typed::monostate</code>).
     * This is not necessarily needed, but ensures consistency on non-main threads.
     * It can also be used as a jump point (similar to <code>return</code> in a regular function).
     * <br>
     * Otherwise, the actual execution body is equivalent to the interval between the input
     * <code>co_await listener</code> and the next declared <code>co_await another_listener</code>.
     */
    class slot final {
    public:
        /// @brief Grants listener<T> access to internal slot members.
        template<typename T>
        friend
        class listener;

        /// @brief Default constructor for slot.
        slot() = default;

        /// @brief Move constructor transferring coroutine handle ownership.
        slot(slot &&o) noexcept: h(o.h) { o.h = {}; }

        /// @brief Move assignment operator transferring coroutine handle ownership.
        slot &operator=(slot &&o) noexcept {
            if (this != &o) {
                h = o.h;
                o.h = {};
            }
            return *this;
        }

        /// @brief Copy constructor is deleted to prevent duplication of coroutine handle.
        slot(const slot &) = delete;

        /// @brief Copy assignment is deleted to prevent unsafe copying of coroutine handle.
        slot &operator=(const slot &) = delete;

        /**
         * @brief Start the coroutine associated with this slot.
         * @details Transitions the coroutine from the initial suspended state to active execution.
         * It binds the coroutine to the current thread permanently. This function is only effective once.
         */
        void spawn() {
            std::call_once(flag, [&] {
                if (h) h.resume();
            });
        }

        /**
         * @brief Slot coroutine promise type.
         * @details Manages the coroutine's suspension behavior, lifecycle hooks, and return semantics.
         */
        struct [[maybe_unused]] promise_type final {

            /**
             * @brief Create a slot coroutine return object.
             * @details Binds the coroutine handle to the returned slot.
             * @return A new slot instance.
             */
            slot get_return_object() noexcept {
                slot s;
                s.h = std::coroutine_handle<promise_type>::from_promise(*this);
                return s;
            }

            /// @brief Define initial suspension behavior.
            std::suspend_always initial_suspend() // NOLINT
            noexcept { return {}; }

            /// @brief Define final suspension behavior.
            std::suspend_always final_suspend() // NOLINT
            noexcept { return {}; }

            /// @brief Handle yielding of a monostate value as a no-op sentinel.
            std::suspend_never yield_value(jh::typed::monostate) // NOLINT
            noexcept {
                return {};
            }

            /**
             * @brief Define return behavior.
             * @details Coroutine ends without returning a value.
             */
            void return_void() noexcept {}

            /**
             * @brief Handle unhandled exceptions.
             * @details Terminates the program in case of coroutine failure.
             */
            void unhandled_exception() // NOLINT
            { std::terminate(); }
        };

    private:
        /// @brief Coroutine handle managed by the slot.
        std::coroutine_handle<> h;

        /// @brief Ensures slot is spawned only once.
        std::once_flag flag;
    };

    /**
     * @brief
     * Synchronization domain managing timed mutex acquisition and binding
     * exactly one slot.
     *
     * @details
     * A <code>slot_hub</code> is not just a synchronizer; it's also a factory for listeners and a creator of topologies.
     * It allows slots and listeners to be constructed separately but bound to the same scope.
     *
     * <h4>Positioning</h4>
     * A <code>slot_hub</code> defines:
     * <ul>
     *   <li>The strong-synchronization domain using a <code>std::timed_mutex</code>.</li>
     *   <li>The timeout policy for event delivery.</li>
     *   <li>The one-to-one binding to a single <code>slot</code>.</li>
     * </ul>
     *
     * <h4>Responsibilities</h4>
     * <ul>
     *   <li>Atomically: write inbox &rarr; resume <code>slot</code> (under the same lock).</li>
     *   <li>Reject event submissions that exceed the timeout window.</li>
     *   <li>Create listeners via <code>make_listener</code>.</li>
     * </ul>
     *
     * @note
     * The <code>slot_hub</code> performs <strong>no buffering</strong>. Every event is either:
     * <em>fully delivered</em> (value written + <code>slot</code> resumed) or <em>completely rejected</em>.
     *
     * @warning
     * Only one <code>slot</code> may be bound to a hub. Binding multiple slots is wrong.
     */
    class slot_hub final {
    public:
        /// @brief Grants listener<T> access to internal slot.
        template<typename T>
        friend
        class listener;

        /**
         * @brief Construct a slot_hub with a timeout.
         * @details Initializes the internal timed mutex and sets the delivery timeout window.
         * @param t Timeout duration for acquiring the lock during emit.
         */
        explicit slot_hub(std::chrono::milliseconds t)
                : timeout(t) {}

        /**
         * @brief Bind a slot coroutine to this hub.
         * @details Associates a single slot coroutine with this synchronization domain.
         * @param s Reference to the slot to bind.
         */
        void bind_slot(slot &s) noexcept {
            attached_slot = &s;
        }

        /**
         * @brief Create a new listener bound to this slot_hub.
         * @details The listener will share the hub's timeout and mutex context.
         * @tparam T Type of payload the listener will receive.
         * @return A new listener instance.
         */
        template<typename T>
        listener<T> make_listener() noexcept {
            return listener<T>(*this);
        }

    private:
        /// @brief Mutex for event delivery synchronization.
        std::timed_mutex mtx;

        /// @brief Maximum duration to wait for acquiring lock during event delivery.
        std::chrono::milliseconds timeout;

        /// @brief Bound slot coroutine. Only one slot may be attached.
        slot *attached_slot = nullptr;
    };

    /**
     * @brief
     * A one-shot inbox that serves as the <strong>fan-in aggregation point</strong>.
     *
     * <h4>Construction</h4>
     * <p>
     * <strong>A listener cannot be created directly.</strong><br>
     * It must always be constructed through <code>slot_hub::make_listener&lt;T&gt;()</code>.
     * </p>
     *
     * <p>
     * This restriction is intentional: a listener without an associated
     * <code>slot_hub</code> is semantically meaningless. A listener's
     * fundamental responsibilities&mdash;synchronous delivery, timed-mutex protection,
     * and <code>slot</code> resumption&mdash;require a <code>slot_hub</code> context. Thus, its constructor is private
     * and <code>slot_hub</code> acts as the factory and parent domain.
     * </p>
     *
     * <h4>Positioning</h4>
     * <p>
     * A listener represents a <em>single suspension point</em> for the slot.
     * It aggregates events coming from one or more <code>event_signal</code>
     * objects and delivers them synchronously to the slot.
     * </p>
     *
     * <h4>Fan-in Capabilities</h4>
     * A <code>listener</code> supports:
     * <ul>
     *   <li><strong>Single-source binding</strong>:
     *     one <code>event_signal&lt;T&gt;</code> &rarr; <code>listener&lt;T&gt;</code></li>
     *   <li><strong>Multi-source equivalent binding</strong>:
     *     many <code>event_signal&lt;T&gt;</code> &rarr; same <code>listener&lt;T&gt;</code>
     *     (sources are indistinguishable)</li>
     *   <li><strong>Multi-source fan-in with identification</strong>:
     *     <ul>
     *       <li><code>listener&lt;std::tuple&lt;ID, T&gt;&gt;</code></li>
     *       <li><code>listener&lt;std::tuple&lt;ID, std::variant&lt;T...&gt;&gt;&gt;</code></li>
     *     </ul>
     *     Sources can be distinguished inside the slot.</li>
     * </ul>
     *
     * <h4>Inbox Semantics</h4>
     * <ul>
     *   <li>Holds at most one value.</li>
     *   <li>Value is consumed during <code>await_resume()</code>.</li>
     *   <li>No buffering or overwriting.</li>
     *   <li>Write + resume is atomic under <code>slot_hub</code> mutex.</li>
     *   <li>Rejection happens only when the mutex cannot be acquired within timeout.</li>
     * </ul>
     *
     * @tparam T Payload type delivered to the slot.
     *
     * @note
     * A <code>slot</code> must only await one <code>listener</code> at a time. Parallel awaits violate the
     * asynchronous semantics and are strongly discouraged.
     *
     * @warning
     * The listener must share lifetime with its <code>slot_hub</code> and <code>slot</code>.
     */
    template<typename T>
    class listener final {
    public:
        /// @brief The type of value delivered to this listener.
        using value_type = T;

        /// @brief Enables slot_hub to create listeners.
        friend class slot_hub;

        /**
         * @brief Check if the coroutine should resume immediately.
         * @details Always returns false to suspend the coroutine.
         * @return false
         */
        [[maybe_unused]] [[nodiscard]] bool await_ready() const noexcept { return false; }

        /**
         * @brief Register coroutine handle for resumption.
         * @details Stores the coroutine handle to resume later when value is delivered.
         * @param h Coroutine handle.
         */
        [[maybe_unused]] void await_suspend(std::coroutine_handle<> h) noexcept {
            handle_ = h;
        }

        /**
         * @brief Resume coroutine and consume the value.
         * @details Extracts the stored value from the inbox and clears it.
         * @return The value of type T delivered by a signal.
         */
        [[maybe_unused]] T await_resume() {
            T v = *inbox_;
            inbox_.reset();
            return v;
        }

        /**
         * @brief Deliver a new value to this listener.
         * @details Attempts to acquire the mutex, store the value, and resume the attached slot.
         * @tparam Args Constructor arguments for T.
         * @param args Arguments to construct the value.
         * @return true if the value was accepted and slot resumed, false otherwise.
         */
        template<typename... Args>
        bool invoke(Args &&... args) {
            slot *sl = hub.attached_slot;
            if (!sl) return false;

            std::unique_lock lock(hub.mtx, std::defer_lock);
            if (lock.try_lock_for(hub.timeout)) {
                inbox_.emplace(std::forward<Args>(args)...);

                if (sl->h && !sl->h.done()) {
                    sl->h.resume();
                    return true;
                }
            }
            return false;
        }

    private:
        /// @brief The associated slot_hub.
        slot_hub &hub;
        /// @brief The associated slot_hub.
        std::optional<T> inbox_;
        /// @brief Coroutine handle to resume when a value is delivered.
        [[maybe_unused]] std::coroutine_handle<> handle_;

        /**
         * @brief Construct a listener bound to the given hub.
         * @details Private constructor accessible only by slot_hub.
         * @param h The synchronization domain.
         */
        explicit listener(slot_hub &h) : hub(h) {}
    };

    /**
     * @brief
     * Lightweight push-only event emitter.
     *
     * <h4>Positioning</h4>
     * <p>
     * An <code>event_signal</code> provides a user-facing injection mechanism.
     * It does not store or buffer events. It only forwards <code>emit()</code> into
     * its connected <code>listener</code>.
     * </p><p>
     * <code>event_signal</code> can have a shorter lifespan than the connected <code>listener</code>,
     * but it cannot have a longer lifespan (dangling). In particular, after a <code>slot</code> coroutine
     * switches the listening object between different states, the <code>event_signal</code> can be recycled by
     * running out of scope.
     * </p>
     *
     * <h4>Semantics</h4>
     * <ul>
     *   <li><code>connect(listener*)</code> must be called before the first <code>emit()</code>.</li>
     *   <li><code>emit()</code> must be called after the <code>slot</code> has been spawned.</li>
     *   <li><code>emit(...)</code> delegates synchronously to <code>listener.invoke()</code>.</li>
     *   <li>If the <tt>hub</tt>'s mutex cannot be acquired in time, the event is rejected.</li>
     * </ul>
     *
     * @tparam T Payload type.
     *
     * @note
     * An <code>event_signal</code> <b>never</b> performs fan-out nor routing. All routing, filtering,
     * switching and fan-out are handled inside the <code>slot</code> coroutine.
     *
     * @warning
     * Must not outlive the <code>listener</code> it is connected to.
     */
    template<typename T>
    class event_signal final {
    public:

        /**
         * @brief Connect this signal to a listener.
         * @details The signal will deliver emitted events to this listener.
         * @param l Pointer to the listener.
         */
        void connect(listener<T> *l) noexcept {
            listener_ = l;
        }

        /**
         * @brief Emit a value to the connected listener.
         * @details Invokes listener's inbox delivery logic with constructed value.
         * @tparam Args Constructor arguments for T.
         * @param args Arguments to construct the value.
         * @return true if delivery succeeded, false otherwise.
         */
        template<typename... Args>
        bool emit(Args &&... args) {
            if (listener_) {
                return listener_->invoke(std::forward<Args>(args)...);
            }
            return false;
        }

    private:
        /// @brief Pointer to the listener connected to this signal.
        listener<T> *listener_ = nullptr;
    };

} // namespace jh::async
