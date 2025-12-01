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
 * @file generator.h (asynchronous)
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief Coroutine-based generator system for modern C++20.
 *
 * <h3>Overview</h3>
 * <p>
 * This header defines <code>jh::async::generator&lt;T, U&gt;</code>,
 * a coroutine-based generator inspired by Python's <code>Generator[T, U, R]</code> type.  
 * It provides both value-yielding (<code>co_yield</code>) and interactive (<code>send()</code>)
 * semantics while keeping the implementation fully type-safe, constexpr-friendly, and header-only.
 * </p>
 *
 * <h4>Namespace Design</h4>
 * <p>
 * This version lives under the <code>jh::async</code> namespace, to emphasize that
 * the generator is coroutine-based and semantically asynchronous.  
 * However, because generator is also a foundational utility, you may include
 * <code>&lt;jh/generator&gt;</code> to import it into <code>jh</code> directly.
 * </p>
 *
 * <h4>Include Behavior</h4>
 * <ul>
 *   <li><code>#include &lt;jh/asynchronous/generator.h&gt;</code> &rarr; defines <code>jh::async::generator</code></li>
 *   <li><code>#include &lt;jh/generator&gt;</code> &rarr; also defines <code>jh::generator</code> as alias of <code>jh::async::generator</code></li>
 * </ul>
 *
 * <h4>Design Motivation</h4>
 * <p>
 * In Python, <code>Generator[T, U, R]</code> expresses three roles:
 * <ul>
 *   <li><code>T</code> &mdash; values <b>yielded</b> by the generator.</li>
 *   <li><code>U</code> &mdash; values <b>sent</b> into the generator.</li>
 *   <li><code>R</code> &mdash; the value returned when the generator finishes.</li>
 * </ul>
 * However, Python's <code>R</code> is not a true return value &mdash; it is part of the coroutine
 * exit mechanism (an exception-based control path).
 * In C++, such behavior can be cleanly modeled using standard exception handling (<tt>try</tt>/<tt>catch</tt>).
 * Therefore, <b>JH's</b> <code>generator&lt;T, U&gt;</code> intentionally omits <code>R</code>
 * to simplify design and align with idiomatic C++ coroutine semantics.
 * </p>
 *
 * <h4>Core Concepts</h4>
 * <ul>
 *   <li><b>Yield type (<code>T</code>)</b> &mdash; values produced by <code>co_yield</code>,
 *       accessible via <code>generator.value()</code> as <code>std::optional&lt;T&gt;</code>.
 *       The optional may be empty if the coroutine has completed.</li>
 *
 *   <li><b>Await type (<code>U</code>)</b> &mdash; values received by <code>co_await</code>,
 *       corresponding to inputs provided through <code>send()</code> or <code>send_ite()</code>.</li>
 *
 *   <li><b>Return type (<code>R</code>)</b> &mdash; intentionally omitted.
 *       In Python's <code>Generator[T, U, R]</code> model, <code>R</code> represents a
 *       special termination channel, but in C++ it can be naturally handled via
 *       <code>try</code>/<code>catch</code> and normal function return semantics.
 *       Thus, <code>jh::async::generator&lt;T, U&gt;</code> omits <code>R</code> entirely.</li>
 * </ul>
 *
 * <h4>Key Features</h4>
 * <ul>
 *   <li>Coroutine-based generator with full C++20 support.</li>
 *   <li>Supports both iterative (<code>next()</code>) and interactive (<code>send()</code>) control.</li>
 *   <li>Range-compatible iteration for non-input generators (<code>U == monostate</code>).</li>
 *   <li>Conversion utilities for <code>std::vector</code>, <code>std::deque</code>, and range wrapping.</li>
 *   <li>Designed for POD-safe, header-only use in low-overhead data pipelines.</li>
 * </ul>
 *
 * <h4>Usage Notes</h4>
 * <ul>
 *   <li>Generators are <b>single-pass</b> &mdash; iteration consumes them.</li>
 *   <li>Copying is disallowed (coroutine handles are unique and non-shareable).</li>
 *   <li>Prefer POD or trivially copyable types for best performance.</li>
 *   <li><code>U</code> defaults to <code>typed::monostate</code> (no input behavior).</li>
 * </ul>
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <coroutine>
#include <functional>
#include <deque>
#include <optional>
#include <stdexcept>
#include <utility>   // NOLINT for std::exchange
#include <vector>
#include <memory>    // NOLINT for std::unique_ptr

#include "jh/conceptual/sequence.h"
#include "jh/conceptual/iterator.h"
#include "jh/typing/monostate.h"

namespace jh::async {

    /**
     * @brief Coroutine-based generator supporting both yielding and receiving values.
     *
     * @details
     * This class implements a coroutine-driven sequence producer, conceptually equivalent to
     * Python's <tt>Generator[T, U, None]</tt>.
     * It provides a clear and type-safe interface for two-way coroutine communication:
     * <ul>
     *   <li><b>Yield</b> &mdash; values are produced via <code>co_yield</code>.</li>
     *   <li><b>Await</b> &mdash; inputs are received via <code>co_await</code>, corresponding to <code>send()</code>.</li>
     * </ul>
     *
     * A generator is a <b>consumable object</b> &mdash; each call to <code>next()</code> or
     * <code>send()</code> advances its internal coroutine state.
     * Once advanced, previously yielded values cannot be revisited.
     *
     * Values produced by <code>co_yield</code> are retrieved via <code>.value()</code>,
     * which returns <code>std::optional&lt;T&gt;</code>.
     * Before the first <code>next()</code> call or after completion,
     * this optional contains <code>std::nullopt</code>.
     *
     * <p><strong>When consuming generator outputs:</strong></p>
     * <ul>
     *   <li>Use <code>.value().value()</code> when you are certain a value exists
     *       (i.e. immediately after a successful <code>next()</code>).</li>
     *   <li>Check <code>.value().has_value()</code> before dereferencing if unsure.</li>
     *   <li>Convert to containers via <code>jh::async::to_vector()</code> or <code>jh::async::to_deque()</code>.</li>
     *   <li>For repeatable iteration, wrap a <em>generator-producing function</em>
     *       (e.g. a lambda returning a new generator) using <code>jh::to_range()</code>,
     *       instead of passing a generator instance directly.</li>
     * </ul>
     *
     * @tparam T The yielded value type (produced by <code>co_yield</code>).
     *           Must be copy-constructible since it is stored within
     *           <code>std::optional&lt;T&gt;</code>.
     *           Prefer trivially copyable or POD-like types for best performance:
     *           <ul>
     *             <li>POD types such as <code>int</code>, <code>float</code>, <code>jh::pod::pair</code>.</li>
     *             <li>Trivial objects such as <code>std::array</code>, <code>std::string</code>.</li>
     *           </ul>
     *
     * @tparam U The input type sent to the generator (via <code>send()</code> or <code>send_ite()</code>).
     *           Corresponds to values received by <code>co_await</code> inside the coroutine.
     *           Defaults to <code>typed::monostate</code>, making the generator a pure output sequence.
     *
     * @note
     * Move-only types (e.g. <code>std::unique_ptr&lt;T&gt;</code>) are not supported by default
     * because the implementation relies on <code>std::optional&lt;T&gt;</code>.
     * To support them, implement a custom buffering or ownership model.
     */
    template<typename T, typename U = typed::monostate> requires std::is_copy_constructible_v<T>
    class generator final {
    public:

        /// @brief Type alias for the value type produced by the generator.
        using value_type = T;

        ///< @brief Type alias for the value type sent to the generator.
        using send_type = U;

        /**
         * <h4>Iterator</h4>
         * <p>
         * Input iterator for <code>jh::async::generator&lt;T, U&gt;</code>.
         * Enables range-based iteration (<code>for(auto v : gen)</code>) when
         * <code>U == typed::monostate</code>.
         * Iteration is <strong>single-pass</strong>: once a value is consumed,
         * it cannot be revisited.
         * </p>
         *
         * <ul>
         *   <li><strong>Category:</strong> <code>std::input_iterator_tag</code></li>
         *   <li><strong>Value semantics:</strong> dereferencing yields a copy of <code>T</code>.</li>
         *   <li><strong>Safety:</strong> dereferencing an exhausted iterator throws <code>std::runtime_error</code>.</li>
         * </ul>
         *
         * <h5>Operations</h5>
         * <ul>
         *   <li><code>iterator(generator&amp; g)</code> &mdash; constructs an iterator bound to a generator.</li>
         *   <li><code>operator++()</code> &mdash; advances to the next yielded value.</li>
         *   <li><code>operator*()</code> &mdash; returns a const reference to the current value.</li>
         *   <li><code>operator-&gt;()</code> &mdash; returns a pointer to the current value.</li>
         *   <li><code>operator==(const iterator&amp;)</code> / <code>operator!=()</code> &mdash; compare iterator positions.</li>
         * </ul>
         *
         * <p>
         * When the generator finishes, the iterator becomes equal to <code>generator::end()</code>.
         * Attempting to increment beyond this point is undefined.
         * </p>
         *
         * <h5>Input Restrictions</h5>
         * <p>
         * Iteration is always supported, even when <code>U != typed::monostate</code>;
         * however, <code>begin()</code> and <code>end()</code> are deliberately omitted
         * to prevent unintended ranged-for iteration with implicit empty input.
         * </p>
         *
         * <p>
         * For generators expecting input (<code>U</code> non-monostate),
         * users must construct <code>iterator{gen}</code> manually to iterate,
         * or advance the coroutine explicitly using
         * <code>next()</code>, <code>send()</code>, or <code>send_ite()</code>.
         * </p>
         *
         * <p>
         * Within the coroutine, <code>co_await</code> may appear selectively&mdash;
         * if no value is sent, a default-constructed <code>U{}</code> is supplied.
         * </p>
         */
        struct iterator final {
            using iterator_concept = std::input_iterator_tag;
            using iterator_category [[maybe_unused]] = iterator_concept;
            using value_type = T;
            using difference_type [[maybe_unused]] = std::ptrdiff_t;
            using pointer = value_type *;
            using reference = value_type &;

            std::optional<std::reference_wrapper<generator>> gen;
            std::optional<value_type> current_value;
            bool is_begin = false;

            explicit iterator(generator &g) : gen(g), is_begin(true) {}

            iterator() = default;

            iterator &operator++() {
                begin_check();
                if (gen && gen->get().next()) current_value = gen->get().value();
                else {
                    gen.reset();
                    current_value.reset();
                }
                return *this;
            }

            iterator operator++(int) {
                iterator tmp = *this;
                ++*this;
                return tmp;
            }

            const value_type &operator*() {
                begin_check();
                if (!current_value) throw std::runtime_error("Dereferencing end iterator");
                return *current_value;
            }

            const value_type *operator->() { return &**this; }

            bool operator==(const iterator &other) const {
                return (!gen && !other.gen) ||
                       (gen && other.gen && std::addressof(gen->get()) == std::addressof(other.gen->get()));
            }

            bool operator!=(const iterator &other) const {
                return !(*this == other); // NOLINT
            }

        private:
            void begin_check() noexcept {
                if (is_begin) [[unlikely]] {
                    if (gen && gen->get().next()) current_value = gen->get().value();
                    else gen.reset();
                    is_begin = false;
                }
            }
        };

        /**
         * @brief Deleted copy constructor.
         * @details
         * Since <code>generator&lt;T, U&gt;</code> manages a coroutine handle (<code>std::coroutine_handle&lt;promise_type&gt;</code>),
         * copying the generator would lead to double ownership issues.
         *
         * <p>To prevent accidental copies, the copy constructor is explicitly deleted.</p>
         */
        generator(const generator &) = delete;

        /**
         * @brief Deleted copy assignment operator.
         * @details
         * Like the copy constructor, the copy assignment operator is deleted to ensure
         * that the coroutine handle is not duplicated, which would lead to undefined behavior.
         */
        generator &operator=(const generator &) = delete;

        /**
         * @brief Move constructor.
         * @details
         * Transfers ownership of the coroutine handle from <tt>other</tt> to <tt>this</tt>.
         * <ul>
         *    <li>The <code>other</code> generator is set to <code>nullptr</code> to prevent double destruction.</li>
         *    <li>This ensures safe movement of generator instances.</li>
         * </ul>
         *
         * @param other The generator to move from.
         */
        generator(generator &&other) noexcept: co_ro(std::exchange(other.co_ro, nullptr)) {
        }

        /**
         * @brief Move assignment operator.
         * @details
         * <ol>
         *   <li>First, it stops the current coroutine if it exists.</li>
         *   <li>Then, it transfers ownership of the coroutine handle from <tt>other</tt> to <tt>this</tt>.</li>
         *   <li>The <code>other</code> generator is set to <code>nullptr</code> to prevent double destruction.</li>
         *   <li>This ensures safe assignment of generator instances.</li>
         * </ol>
         *
         * @param other The generator to move from.
         * @return Reference to <code>this</code> generator after assignment.
         */
        generator &operator=(generator &&other) noexcept {
            if (this != &other) {
                stop();
                co_ro = std::exchange(other.co_ro, nullptr);
            }
            return *this;
        }

        /**
         * <h4>Promise Type</h4>
         * <p>
         * The <code>promise_type</code> defines the state and behavior of the coroutine
         * underlying a <code>jh::async::generator&lt;T, U&gt;</code>.
         * It manages yielded values, received inputs, exception propagation,
         * and coroutine suspension control points.
         * </p>
         *
         * <h5>Responsibilities</h5>
         * <ul>
         *   <li>Store the most recent <b>yielded value</b> (<code>T</code>) and <b>sent input</b> (<code>U</code>).</li>
         *   <li>Provide <b>suspension points</b> for coroutine start, yield, and finalization.</li>
         *   <li>Propagate exceptions to the generator interface via <code>std::exception_ptr</code>.</li>
         *   <li>Transform <code>co_await</code> expressions into awaiters that access the sent value.</li>
         * </ul>
         *
         * <h5>Member Variables</h5>
         * <ul>
         *   <li><code>std::optional&lt;T&gt; current_value</code> &mdash; latest value produced by <code>co_yield</code>.</li>
         *   <li><code>std::optional&lt;U&gt; last_sent_value</code> &mdash; most recent input value provided by <code>send()</code>.</li>
         *   <li><code>std::exception_ptr exception</code> &mdash; captures any uncaught exceptions raised during execution.</li>
         * </ul>
         *
         * <h5>Coroutine Lifecycle Hooks</h5>
         * <ul>
         *   <li><code>get_return_object()</code> &mdash; constructs and returns the bound <code>generator</code> instance.</li>
         *   <li><code>initial_suspend()</code> &mdash; always suspends before the first resumption (<code>std::suspend_always</code>).</li>
         *   <li><code>final_suspend()</code> &mdash; always suspends upon coroutine completion, ensuring handle safety.</li>
         *   <li><code>yield_value(T)</code> &mdash; stores the yielded value and suspends execution.</li>
         *   <li><code>return_void()</code> &mdash; marks normal coroutine completion (no explicit return value).</li>
         *   <li><code>unhandled_exception()</code> &mdash; catches and stores any unhandled exceptions.</li>
         * </ul>
         *
         * <h5>Awaiter Mechanism</h5>
         * <p>
         * The inner <code>awaiter</code> structure implements <code>co_await</code> behavior.
         * When the coroutine executes <code>co_await U{}</code>, the following occurs:
         * </p>
         * <ul>
         *   <li><code>await_ready()</code> &mdash; always returns <code>false</code>, forcing suspension.</li>
         *   <li><code>await_suspend()</code> &mdash; performs no action (passive suspension point).</li>
         *   <li><code>await_resume()</code> &mdash; retrieves the last value sent to the coroutine,
         *       or a default-constructed <code>U{}</code> if no value was provided.</li>
         * </ul>
         */
        struct promise_type final {
            std::optional<T> current_value;   ///< Stores the current yielded value.
            std::optional<U> last_sent_value; ///< Stores the last value sent to the generator.
            std::exception_ptr exception;     ///< Stores an exception if one occurs.

            /**
             * @brief Creates and returns the generator object.
             * @return A generator instance bound to this coroutine promise.
             */
            generator get_return_object() {
                return generator{std::coroutine_handle<promise_type>::from_promise(*this)};
            }

            /**
             * @brief Suspends execution initially.
             * @return Always returns <code>std::suspend_always</code> to suspend execution at the start.
             */
            std::suspend_always initial_suspend() // NOLINT
            noexcept { return {}; }

            /**
             * @brief Suspends execution at the final stage.
             * @return Always returns <code>std::suspend_always</code> to suspend execution at the end.
             */
            std::suspend_always final_suspend() // NOLINT
            noexcept { return {}; }

            /**
             * @brief Yields a value from the generator.
             * @param value The value to yield.
             * @return Always returns <code>std::suspend_always</code> to suspend execution after yielding.
             */
            std::suspend_always yield_value(T value) {
                current_value.emplace(std::forward<T>(value));
                return {};
            }

            /**
             * @brief Lightweight awaiter used to deliver values sent into the generator via <code>co_await</code>.
             * <p>
             * Acts as the bridge between <code>send()</code> and <code>co_await</code>:
             * when the coroutine executes <code>co_await U{}</code>,
             * this object suspends execution and later resumes it with the value
             * passed by the most recent <code>send()</code> call.
             * </p>
             * <p>
             * If no value has been sent, <code>await_resume()</code> returns a
             * default-constructed <code>U{}</code>, allowing safe use even without input.
             * </p>
             */
            struct awaiter final {
                promise_type &promise;                                       ///< Reference to the coroutine promise.

                [[maybe_unused]] static bool await_ready() { return false; } ///< Always returns false to suspend.

                [[maybe_unused]] static void await_suspend(std::coroutine_handle<>) {
                }                                                            ///< No-op on suspension.

                /**
                 * @brief Retrieves the last sent value or a default.
                 * @return The most recent value sent to the generator.
                 */
                [[maybe_unused]] U await_resume() {
                    return promise.last_sent_value.value_or(U{});
                }
            };

            /**
             * @brief Transforms an awaited value into an <code>awaiter</code> for coroutine execution.
             * @return An <code>awaiter</code> object tied to this coroutine promise.
             */
            awaiter await_transform(U) noexcept {
                return awaiter{*this}; // Ensures Clang-Tidy does not suggest making it static
            }

            /**
             * @brief Defines the behavior when the coroutine completes.
             */
            void return_void() // NOLINT
            noexcept {}

            /**
             * @brief Handles uncaught exceptions by storing them.
             */
            void unhandled_exception() {
                exception = std::current_exception();
            }
        };

        std::coroutine_handle<promise_type> co_ro; ///< Handle to the coroutine.

        /**
         * @brief Constructs a <code>generator</code> directly from its coroutine handle.
         * <p>
         * This constructor is the linkage point between the coroutine's
         * <code>promise_type</code> and its corresponding
         * <code>jh::async::generator&lt;T, U&gt;</code> object.
         * It is invoked automatically by the compiler when a coroutine function
         * returning a generator is defined and called.
         * </p>
         * <p>
         * This enables Python-like semantics for defining and using coroutine generators:
         * <ul>
         *   <li><code>jh::async::generator&lt;T, U&gt; Func(Args...) { scope_with_co_yield(); }</code> &mdash; defines a coroutine generator.</li>
         *   <li><code>Func(args...)</code> &mdash; directly obtains a generator instance, without explicitly handling <code>std::coroutine_handle</code>.</li>
         *   <li><code>jh::to_range([...] { Func(args...); })</code> &mdash; wraps the generator-producing function into a reusable, re-entrant range.</li>
         * </ul>
         * </p>
         * <p>
         * Thus, <code>jh::async::generator&lt;T, U&gt;</code> aligns closely with Python's
         * <tt>Generator[T, U, None]</tt> semantics, making coroutine-based data pipelines
         * natural and concise in C++.
         * </p>
         *
         * @param h The coroutine handle to be managed by this generator.
         */
        explicit generator(std::coroutine_handle<promise_type> h) : co_ro(h) {
        }

        /**
         * @brief Destroys the coroutine handle if it exists.
         */
        ~generator() { if (co_ro) co_ro.destroy(); }

        /**
         * @brief Advances the generator to the next value.
         * @return <code>true</code> if a new value is available, <code>false</code> if the coroutine has finished.
         */
        bool next() {
            if (!co_ro || co_ro.done()) return false;
            co_ro.resume();
            if (co_ro.promise().exception) std::rethrow_exception(co_ro.promise().exception);
            return !co_ro.done();
        }

        /**
         * @brief Checks if the generator has completed execution.
         * @return <code>true</code> if the generator has finished, <code>false</code> otherwise.
         */
        [[nodiscard]] bool done() const noexcept {
            return !co_ro || co_ro.done();
        }

        /**
         * @brief Sends a value to the generator and resumes execution.
         * @param value The value to send.
         * @return <code>true</code> if the coroutine is still active, <code>false</code> otherwise.
         *
         * @note
         * When <code>U == typed::monostate</code>, this function becomes a no-op.
         * Since there is no <code>co_await</code> to receive input, the call does not
         * advance the coroutine or affect its state.
         * Use <code>next()</code> or <code>send_ite()</code> instead to progress the generator.
         */
        bool send(send_type value) {
            if (!co_ro || co_ro.done()) return false;

            auto &promise = co_ro.promise();
            promise.last_sent_value = value; // Explicitly reference instance
            co_ro.resume();

            // Ensure the next value is updated immediately
            if (co_ro.promise().exception) std::rethrow_exception(co_ro.promise().exception);
            return !co_ro.done();
        }

        /**
         * @brief Advances the generator and sends a value in one step.
         *
         * This function combines <code>next()</code> and <code>send()</code>,
         * eliminating the need for a separate <code>next()</code> call.
         * It first advances the generator, and if successful, sends the provided value.
         *
         * @param value The value to send to the generator.
         * @return <code>true</code> if the generator successfully advances and accepts the value,
         *         <code>false</code> if the generator has finished.
         *
         * @note
         * When <code>U == typed::monostate</code>, this behaves identically to
         * <code>next()</code>, since no input is transmitted and the send stage is inert.
         */
        bool send_ite(send_type value) {
            if (!co_ro || co_ro.done()) return false;

            co_ro.resume(); // Advance coroutine
            if (co_ro.promise().exception) std::rethrow_exception(co_ro.promise().exception);

            co_ro.promise().last_sent_value = value; // Store input value
            co_ro.resume(); // Resume coroutine after sending
            if (co_ro.promise().exception) std::rethrow_exception(co_ro.promise().exception);

            return !co_ro.done();
        }

        /**
         * @brief Retrieves the currently yielded value.
         * @return An optional containing the current value.
         *
         * @note
         * The returned value is a <strong>copy</strong> of the last yielded element.
         * This ensures that each access is memory-safe and independent of
         * coroutine resumption, avoiding dangling references.
         *
         * If your generator needs to yield pointer-like data,
         * use a <strong>copyable smart pointer</strong> such as
         * <code>std::shared_ptr&lt;T&gt;</code> or any equivalent
         * reference-counted handle type.
         *
         * @warning
         * <code>std::unique_ptr&lt;T&gt;</code> and other move-only types
         * are not supported because the generator requires
         * <code>T</code> to be copy-constructible for
         * <code>std::optional&lt;T&gt;</code> storage.
         */
        std::optional<value_type> value() const noexcept {
            return co_ro.promise().current_value;
        }

        /**
         * @brief Retrieves the last value sent to the generator.
         * @return An optional containing the most recent input value transmitted via <code>send()</code> or <code>send_ite()</code>.
         *
         * @details
         * This accessor returns the most recent value (<code>U</code>)
         * that was sent into the coroutine through <code>send()</code> or <code>send_ite()</code>.
         * The stored value is preserved until the next input or coroutine resumption.
         * If no input has been sent yet, the returned <code>std::optional</code> is empty.
         *
         * <p>
         * The returned value is a <strong>copy</strong> of the last sent element.
         * This guarantees safety after coroutine resumption.
         * If pointer semantics or shared ownership are required, use
         * <strong>copyable reference-counted types</strong> such as
         * <code>std::shared_ptr&lt;U&gt;</code> instead of move-only handles.
         * </p>
         *
         * <p>
         * When <code>U == typed::monostate</code>, the generator does not consume inputs at all.
         * In such cases, this accessor has <strong>no semantic meaning</strong>
         * and always yields an empty <code>std::optional</code>.
         * </p>
         */
        [[maybe_unused]] std::optional<send_type> last_sent_value() const noexcept {
            return co_ro.promise().last_sent_value;
        }

        /**
         * @brief Stops the generator and destroys the coroutine.
         */
        void stop() {
            if (co_ro) {
                co_ro.destroy();
                co_ro = nullptr;
            }
        }

        /**
         * @brief Returns an iterator for ranged-for loops.
         * @details
         * Enables use of the generator in a C++ range-based loop:
         * <code>for (auto x : gen)</code>.
         * This overload is available only when <code>U == typed::monostate</code>,
         * meaning the generator does not expect any input values.
         *
         * <p>
         * Each iteration step advances the coroutine and <strong>consumes its internal state</strong>.
         * Unlike standard ranges, a generator cannot be treated as a view or re-iterated,
         * because iteration directly resumes and mutates the underlying coroutine frame.
         * </p>
         *
         * <p>
         * A <code>const</code> version of <code>begin()</code> is <strong>intentionally deleted</strong>
         * because invoking iteration on a constant generator would violate
         * logical immutability: advancing the coroutine inherently modifies
         * its promise object and execution context.
         * </p>
         *
         * @return An iterator positioned at the beginning of the generator sequence.
         * @see generator::end
         */
        iterator begin() requires typed::monostate_t<send_type> {
            return iterator{*this};
        }

        /**
         * @brief Deleted <code>const</code> overload of <code>begin()</code>.
         * @details
         * Generator iteration is a <strong>stateful and consuming</strong> operation.
         * Allowing a <code>const</code> overload would incorrectly imply
         * immutability, even though every iteration step mutates the coroutine's
         * suspended frame.
         *
         * <p>
         * This deletion enforces the invariant that <code>jh::async::generator&lt;T, U&gt;</code>
         * may only be iterated when held as a mutable instance.
         * </p>
         */
        iterator begin() const = delete;

        /**
         * @brief Returns a sentinel iterator representing the end of the generator sequence.
         * @details
         * This function provides the canonical <strong>past-the-end sentinel</strong>
         * for use in range-based iteration.
         * Unlike <code>begin()</code>, calling <code>end()</code> never resumes or interacts
         * with the underlying coroutine; it simply returns a default-constructed
         * <code>iterator</code> object used to mark the termination of iteration.
         *
         * <p>
         * Because it performs no coroutine access, <code>end()</code> is
         * <strong>idempotent</strong> &mdash; it can be safely invoked multiple times
         * without affecting the generator state.
         * </p>
         *
         * <p>
         * This overload is available only when <code>U == typed::monostate</code>,
         * meaning the generator is purely output-driven and does not require
         * input through <code>send()</code>.
         * </p>
         *
         * @return A default-constructed iterator serving as the logical end sentinel.
         * @see generator::begin
         */
        static iterator end() requires typed::monostate_t<send_type> {
            return iterator{};
        }

    };

    /**
     * @brief Converts a duck-typed <strong>sequence-like</strong> object into a generator.
     *
     * @details
     * This overload provides the <strong>most permissive</strong> fallback version
     * of <code>make_generator()</code>.
     * It accepts any type that satisfies the minimal <code>jh::sequence</code> concept:
     * having <code>begin()</code> and <code>end()</code> returning readable iterators
     * and supporting <code>!=</code> comparison.
     *
     * <p>
     * Unlike <code>std::ranges::range</code>, a <code>jh::sequence</code> does <strong>not</strong>
     * guarantee forwarding or lifetime semantics &mdash; only that it can be read immutably.
     * Therefore, this overload uses <strong>const reference</strong> and avoids forwarding
     * or move-based iteration.
     * </p>
     *
     * <ul>
     *   <li>Acts as a <strong>duck-typed fallback</strong> for legacy or lightweight containers
     *       that behave like ranges but are not formally defined as such.</li>
     *   <li>Preserves the minimal safety invariant: iteration must not mutate <code>seq</code>.</li>
     *   <li>Ensures compatibility with <strong>immutable const iteration</strong>
     *       even in non–range-conforming types.</li>
     * </ul>
     *
     * @tparam SeqType The input sequence type; must satisfy <code>jh::sequence</code>
     *         but not <code>std::ranges::range</code>.
     * @param seq The sequence-like object to convert (read-only).
     * @return A generator yielding elements from <code>seq</code>.
     */
    template<concepts::sequence SeqType> requires (!std::ranges::range<SeqType>)
    [[maybe_unused]] generator<concepts::sequence_value_t < SeqType> >

    make_generator(const SeqType &seq) {
        for (const auto &elem: seq) {
            // Use range-based for-loop
            co_yield elem;
        }
        co_return;
    }

    /**
     * @brief Converts a standard <strong>range</strong> into a generator.
     *
     * @details
     * This overload handles any type satisfying the C++20 <code>std::ranges::range</code> concept.
     * It directly consumes the range by iteration, yielding each element via <code>co_yield</code>.
     *
     * <p>
     * Because <code>range</code> guarantees valid lifetime and iterator semantics,
     * this overload may safely take the range by <strong>universal reference</strong>
     * and use <code>std::forward</code> to preserve value category.
     * </p>
     *
     * <ul>
     *   <li>Preserves const correctness and supports read-only iteration.</li>
     *   <li>Allows moving temporary or view-based ranges directly into the coroutine &mdash;
     *       the generator takes ownership of the iteration sequence.</li>
     *   <li>Unlike the <code>sequence</code> overload, this version fully supports
     *       <strong>forwarded rvalue ranges</strong> and <strong>lazy views</strong>.</li>
     * </ul>
     *
     * @tparam R A valid <code>std::ranges::range</code> type.
     * @param rng The range object or view to convert.
     * @return A generator yielding elements from <code>rng</code>.
     */
    template<std::ranges::range R>
    generator<std::ranges::range_value_t<R> > make_generator(R &&rng) {
        for (auto &&elem: rng) {
            co_yield std::forward<decltype(elem)>(elem);
        }
    }

    /**
     * @brief Collects all yielded values from a generator into a <code>std::vector</code>.
     *
     * @details
     * This overload applies to generators that produce values but do not receive input
     * (<code>U == typed::monostate</code>).
     * It repeatedly advances the coroutine using <code>next()</code> until completion,
     * copying each yielded value into a contiguous <code>std::vector</code>.
     *
     * <ul>
     *   <li>Intended for output-only generators.</li>
     *   <li>Preserves iteration order and copies each yielded element.</li>
     *   <li>Acts as the most lightweight way to "materialize" a generator sequence.</li>
     * </ul>
     *
     * @tparam T The generator's <strong>yielded value type</strong>.
     * @param gen The generator instance to collect from.
     * @return A <code>std::vector</code> containing all yielded elements in order.
     */
    template<typename T>
    std::vector<T> to_vector(generator<T> &gen) {
        std::vector<T> result;

        while (gen.next()) {
            result.push_back(gen.value().value());
        }

        return result;
    }

    /**
     * @brief Collects all yielded values into a <code>std::vector</code> using a fixed input value.
     *
     * @details
     * This overload supports interactive generators that expect an input type
     * (<code>U != typed::monostate</code>).
     * It sends the same <strong>input value</strong> at every coroutine step via <code>send()</code>,
     * producing a deterministic output sequence.
     *
     * <ul>
     *   <li>Each iteration performs <code>next()</code> then <code>send(input_value)</code>.</li>
     *   <li>Useful for constant-parameter simulations or iterative transforms.</li>
     *   <li>If the generator terminates early, iteration stops gracefully.</li>
     * </ul>
     *
     * @tparam T The yielded value type.
     * @tparam U The input type accepted by the generator.
     * @param gen The generator to consume.
     * @param input_value The fixed input sent at each step.
     * @return A <code>std::vector</code> containing all yielded values.
     */
    template<typename T, typename U>
    std::vector<T> to_vector(generator<T, U> &gen, U input_value) {
        std::vector<T> result;
        while (gen.next()) {
            if (!gen.send(input_value)) break;
            result.push_back(gen.value().value());
        }
        return result;
    }

    /**
     * @brief Collects all yielded values into a <code>std::vector</code> using a sequence of input values.
     *
     * @details
     * This overload supports generators that consume varying inputs.
     * It sequentially sends each element from <code>inputs</code> to the generator
     * via <code>send()</code> and accumulates the produced values into a vector.
     *
     * <ul>
     *   <li>Synchronizes one <strong>send()</strong> call per yielded element.</li>
     *   <li>Terminates when either the input range or generator is exhausted.</li>
     *   <li>Ensures order-preserving correspondence between inputs and outputs.</li>
     * </ul>
     *
     * <p>
     * This function formally requires <code>inputs</code> to satisfy
     * <code>std::ranges::range</code>.
     * However, any <strong>duck-typed sequence</strong> that conforms to the
     * <code>jh::sequence</code> concept can be made compatible by wrapping it with
     * <code>jh::to_range()</code>:
     * </p>
     *
     * @code
     * auto vec = jh::async::to_vector(gen, jh::to_range(my_sequence));
     * @endcode
     *
     * <p>
     * This design ensures that legacy containers and lightweight sequence-like types
     * remain usable in generator pipelines without requiring full <code>std::ranges</code> compliance.
     * </p>
     *
     * @tparam T The generator's output type (<code>co_yield</code> value type).
     * @tparam R A <code>std::ranges::range</code> of input values sent to the generator.
     * @param gen The generator to consume.
     * @param inputs The input range providing values for each step.
     * @return A <code>std::vector</code> containing all yielded results in sequence.
     */
    template<typename T, std::ranges::range R>
    std::vector<T> to_vector(generator<T, std::ranges::range_value_t<R> > &gen, const R &inputs) {
        std::vector<T> result;
        auto it = std::ranges::begin(inputs);
        auto end = std::ranges::end(inputs);

        if (it != end) {
            if (!gen.send(*it)) return result;
            while (gen.next()) {
                result.push_back(gen.value().value());
                if (++it == end) break;
                if (!gen.send(*it)) break;
            }
        }

        return result;
    }

    /**
     * @brief Collects all yielded values from a generator into a <code>std::deque</code>.
     *
     * @details
     * This overload applies to generators that produce values but do not receive input
     * (<code>U == typed::monostate</code>).
     * It repeatedly advances the coroutine using <code>next()</code> until completion,
     * appending each yielded element into a <code>std::deque</code>.
     *
     * <ul>
     *   <li>Designed for output-only generators.</li>
     *   <li>Maintains stable iterator validity across insertions.</li>
     *   <li>Uses the fastest STL <strong>segmented linear container</strong>,
     *       ideal for frequent <code>push_back()</code> operations.</li>
     * </ul>
     *
     * @tparam T The generator's <strong>yielded value type</strong>.
     * @param gen The generator instance to collect from.
     * @return A <code>std::deque</code> containing all yielded elements in order.
     */
    template<typename T>
    std::deque<T> to_deque(generator<T> &gen) {
        std::deque<T> result;
        while (gen.next()) {
            result.push_back(gen.value().value());
        }
        return result;
    }

    /**
     * @brief Collects all yielded values into a <code>std::deque</code> using a fixed input value.
     *
     * @details
     * This overload supports interactive generators that expect an input type
     * (<code>U != typed::monostate</code>).
     * It sends the same <strong>input value</strong> at every coroutine step via <code>send()</code>,
     * pushing each yielded element into a <code>std::deque</code>.
     *
     * <ul>
     *   <li>Each iteration performs <code>next()</code> then <code>send(input_value)</code>.</li>
     *   <li>Efficient for streaming pipelines where append cost must remain amortized O(1).</li>
     *   <li>Deque preserves all references and pointers to existing elements upon reallocation.</li>
     * </ul>
     *
     * @tparam T The yielded value type.
     * @tparam U The input type accepted by the generator.
     * @param gen The generator to consume.
     * @param input_value The fixed input sent at each step.
     * @return A <code>std::deque</code> containing all yielded values.
     */
    template<typename T, typename U>
    [[maybe_unused]] std::deque<T> to_deque(generator<T, U> &gen, U input_value) {
        std::deque<T> result;
        while (gen.next()) {
            if (!gen.send(input_value)) break;
            result.push_back(gen.value().value());
        }
        return result;
    }

    /**
     * @brief Collects all yielded values into a <code>std::deque</code> using a sequence of input values.
     *
     * @details
     * This overload supports generators that consume varying inputs.
     * It sequentially sends each element from <code>inputs</code> to the generator
     * via <code>send()</code> and appends the produced values into a deque.
     *
     * <ul>
     *   <li>Synchronizes one <strong>send()</strong> per yielded element.</li>
     *   <li>Terminates when either the input range or generator is exhausted.</li>
     *   <li>Deque is optimal for dynamic growth and stable iteration under expansion.</li>
     * </ul>
     *
     * <p>
     * The <code>inputs</code> parameter must satisfy <code>std::ranges::range</code>.
     * Nevertheless, any <strong>duck-typed sequence</strong> that conforms to
     * <code>jh::sequence</code> can be wrapped via <code>jh::to_range()</code>:
     * </p>
     *
     * @code
     * auto dq = jh::async::to_deque(gen, jh::to_range(my_sequence));
     * @endcode
     *
     * <p>
     * This allows compatibility with user-defined containers that are not formally
     * <code>std::ranges::range</code> but still support <code>begin()</code>/<code>end()</code>.
     * </p>
     *
     * @tparam T The generator's output type (<code>co_yield</code> value type).
     * @tparam R A <code>std::ranges::range</code> of input values sent to the generator.
     * @param gen The generator to consume.
     * @param inputs The input range providing values for each step.
     * @return A <code>std::deque</code> containing all yielded results in sequence.
     */
    template<typename T, std::ranges::range R>
    [[maybe_unused]] std::deque<T> to_deque(generator<T, std::ranges::range_value_t<R> > &gen, const R &inputs) {
        std::deque<T> result;
        auto it = std::ranges::begin(inputs);
        auto end = std::ranges::end(inputs);

        if (it != end) {
            if (!gen.send(*it)) return result;
            while (gen.next()) {
                result.push_back(gen.value().value());
                if (++it == end) break;
                if (!gen.send(*it)) break;
            }
        }

        return result;
    }


    /**
     * @brief A range-like wrapper that enables iteration over a generator factory.
     *
     * @details
     * <code>generator_range&lt;T&gt;</code> provides a <b>repeatable</b> and <b>range-compatible</b>
     * interface for coroutine-based generators.
     * Instead of storing a single generator instance (which would be consumed after one iteration),
     * it holds a <strong>factory function</strong> that can produce a fresh
     * <code>jh::async::generator&lt;T&gt;</code> each time.
     *
     * <p>
     * The factory must have one of the following signatures:
     * </p>
     * <ul>
     *   <li><code>jh::async::generator&lt;T&gt; func()</code></li>
     *   <li><code>[...]() -&gt; jh::async::generator&lt;T&gt; { ... }</code> (lambda expression)</li>
     * </ul>
     *
     * <p>
     * This mechanism enforces <strong><code>U == typed::monostate</code></strong>,
     * meaning the generator cannot depend on external <code>send()</code> input values.
     * The resulting range can thus be freely iterated and reused.
     * </p>
     *
     * <h4>Key Properties</h4>
     * <ul>
     *   <li>Each call to <code>begin()</code> constructs a new generator instance via the stored factory.</li>
     *   <li>Fully compatible with <code>std::ranges</code> and STL algorithms.</li>
     *   <li>Usable in <code>for(auto v : range)</code>, <code>std::views::zip(...)</code>, and similar constructs.</li>
     *   <li>Guarantees value-type semantics &mdash; <code>T</code> must be copyable.</li>
     * </ul>
     *
     * @note The factory does <strong>not</strong> accept runtime arguments.
     * It must capture all external state through its closure (via <code>[...]</code> capture).
     * Passing parameterized generators must be done by creating multiple distinct factories.
     *
     * @tparam T The yielded value type of the underlying <code>jh::async::generator</code>.
     *
     * @see jh::to_range
     */
    template<typename T>
    class generator_range : std::ranges::view_interface<generator_range<T> > {
    public:
        using generator_factory_t = std::function<generator<T>()>;

        explicit generator_range(generator_factory_t factory)
                : factory_(std::move(factory)) {
        }

        class iterator final {
        public:
            using value_type = T;
            using reference = const T &;
            using pointer = const T *;
            using iterator_concept = std::input_iterator_tag;
            using iterator_category [[maybe_unused]] = iterator_concept;
            using difference_type [[maybe_unused]] = std::ptrdiff_t;

            /// @brief Default-constructs an <code>iterator</code> in an inactive state.
            iterator() = default;

            /// @brief Copy construction is disabled to preserve single-pass generator semantics.
            iterator(const iterator &) = delete;

            /// @brief Copy assignment is disabled because an <code>iterator</code> cannot be duplicated safely.
            iterator &operator=(const iterator &) = delete;

            /// @brief Move-constructs an <code>iterator</code>, transferring ownership of the underlying generator.
            iterator(iterator &&) noexcept = default;

            /// @brief Move-assigns an <code>iterator</code>, transferring ownership of the underlying generator.
            iterator &operator=(iterator &&) noexcept = default;

            explicit iterator(generator_factory_t factory)
                    : gen_(std::make_unique<generator<T> >(factory())) {
                ++(*this); // prime
            }

            iterator &operator++() {
                if (gen_) {
                    if (!gen_->next() || gen_->done() || !gen_->value().has_value()) {
                        gen_.reset(); // Stop: no more values
                    }
                }
                return *this;
            }

            iterator operator++(int) {
                iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            value_type operator*() const {
                if (!gen_ || !gen_->value().has_value())
                    throw std::runtime_error("Dereferencing end iterator");
                return *gen_->value();
            }

            pointer operator->() const {
                return &**this;
            }

            friend bool operator==(const iterator &lhs, const iterator &rhs) {
                // both exhausted
                if (!lhs.gen_ && !rhs.gen_) return true;
                // one is exhausted, one is not
                if (!lhs.gen_ || !rhs.gen_) return false;
                // fallback: pointer identity
                return lhs.gen_.get() == rhs.gen_.get();
            }

            friend bool operator!=(const iterator &lhs, const iterator &rhs) {
                return !(lhs == rhs); // NOLINT
            }

            // Support std::default_sentinel_t as the end
            friend bool operator==(const iterator &it, std::default_sentinel_t) {
                return !it.gen_;
            }

            friend bool operator!=(const iterator &it, std::default_sentinel_t s) {
                return !(it == s); // NOLINT
            }

        private:
            std::unique_ptr<generator<T> > gen_;
        };

        iterator begin() const {
            return iterator{factory_};
        }

        [[nodiscard]] static std::default_sentinel_t end() noexcept {
            return {};
        }

    private:
        generator_factory_t factory_;
    };
} // namespace jh::async

namespace jh {
    /**
     * @brief Converts a generator factory (lambda or function) into a repeatable range.
     *
     * @details
     * This helper transforms a callable object that returns
     * <code>jh::async::generator&lt;T&gt;</code> (with <code>U == typed::monostate</code>)
     * into a <code>jh::async::generator_range&lt;T&gt;</code>.
     * The resulting object supports multiple independent iterations,
     * since each call to <code>begin()</code> constructs a new generator instance.
     *
     * <p>
     * The callable must take <strong>no arguments</strong>:
     * </p>
     * <ul>
     *   <li><code>jh::async::generator&lt;T&gt; func()</code></li>
     *   <li><code>[=]() -&gt; jh::async::generator&lt;T&gt; { ... }</code></li>
     *   <li><code>[&]() -&gt; jh::async::generator&lt;T&gt; { ... }</code></li>
     *   <li><code>[+]() -&gt; jh::async::generator&lt;T&gt; { ... }</code></li>
     * </ul>
     *
     * <p>
     * Any captured state must be enclosed within the lambda's closure.
     * External parameters cannot be forwarded dynamically.
     * </p>
     *
     * @tparam F A callable returning <code>jh::async::generator&lt;T&gt;</code>.
     * @param f A generator factory function (no arguments).
     * @return A repeatable, range-compatible wrapper for the generator.
     *
     * @see jh::async::generator_range
     */
    template<typename F>
    requires requires(F f)
    {
        typename std::invoke_result_t<F>;
        requires std::is_same_v<std::remove_cvref_t<std::invoke_result_t<F> >,
                jh::async::generator<typename std::invoke_result_t<F>::value_type> >;
    }
    auto to_range(F &&f) {
        using Gen = std::remove_cvref_t<std::invoke_result_t<F> >;
        using T = typename Gen::value_type;
        return jh::async::generator_range<T>(std::function<Gen()>(std::forward<F>(f)));
    }
} // namespace jh
