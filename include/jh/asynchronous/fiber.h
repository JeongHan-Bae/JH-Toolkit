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
 * @file fiber.h (asynchronous)
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief Coroutine-based fiber for modern C++20.
 *
 * <h3>Overview</h3>
 * <p>
 * This file defines a lightweight coroutine-based execution unit named <code>fiber</code>.
 * A <code>fiber</code> represents a resumable coroutine without scheduling support.
 * Its behavior is conceptually aligned with a thread blocked on
 * <code>condition_variable.wait()</code>, but with significantly lower overhead.
 * </p>
 *
 * <h4>Design Notes</h4>
 * <ul>
 * <li>Execution occurs entirely inside the coroutine body.</li>
 * <li>All coroutine logic must be <code>noexcept</code>.</li>
 * <li>Any uncaught exception results in an immediate call to <code>std::terminate()</code>.</li>
 * <li>The coroutine does not guarantee <code>co_return</code> or <code>co_yield</code>
 * like a generator; this makes exception masking mandatory.</li>
 * </ul>
 *
 * <h4>Usage Model</h4>
 * <p>
 * The <code>fiber</code> object can be resumed repeatedly using <code>resume()</code> until it
 * reaches its final suspend point. The type does not integrate with an external scheduler
 * and does not provide automatic continuation handling.
 * </p>
 */

#pragma once

#include <coroutine>
#include <utility>
#include <stdexcept>


namespace jh::async {

    /**
     * @brief Tag type used to trigger a <code>co_await</code> inside a <code>fiber</code>.
     *
     * @details
     * <ul>
     * <li>Acts as a marker for <code>await_transform()</code> inside the promise type.</li>
     * <li>Always suspends when awaited.</li>
     * </ul>
     */
    struct resume_t {
    };

    /// @brief Global constant instance of <code>resume_t</code>.
    inline resume_t resume_tag{};

    /**
     * @brief Coroutine-based fiber providing manual suspension and resumption.
     *
     * @details
     * A <code>fiber</code> models a manually advanced coroutine, conceptually equivalent to
     * a thread blocked on <code>std::condition_variable.wait()</code>:
     * <p>
     * execution is suspended until an explicit external resume request is issued.
     * Unlike an OS thread, a <code>fiber</code> is extremely lightweight because it stores
     * only a coroutine frame, not a stack.
     * </p>
     *
     * Suspension inside the coroutine is performed using
     * <code>co_await resume_tag</code>, which introduces a controlled wait point
     * analogous to <code>condition_variable.wait()</code>:
     * <ul>
     *   <li>always suspends,</li>
     *   <li>registers no scheduler,</li>
     *   <li>resumes only when the caller explicitly invokes <code>resume()</code>.</li>
     * </ul>
     *
     * Exception propagation is disabled inside a <code>fiber</code>. Any uncaught exception
     * results in a call to <code>std::terminate()</code>, matching the semantics of
     * a thread whose entry function throws without being caught. This avoids continuing
     * execution after a fatal error.
     *
     * <h4>Behavior Characteristics</h4>
     * <ul>
     *   <li>Lightweight coroutine frame instead of an OS thread.</li>
     *   <li>Manual control: external calls to <code>resume()</code> drive progress.</li>
     *   <li><code>co_await resume_tag</code> provides explicit suspension points.</li>
     *   <li>Fatal errors end in <code>std::terminate()</code>, matching thread behavior.</li>
     * </ul>
     */
    class fiber final {
    public:

        bool done_flag = false; ///< flag to prevent UCRT unexpected done() check after destroy
        /**
         * <h4>Promise Type</h4>
         *
         * @details
         * The <code>promise_type</code> defines the coroutine state and lifecycle
         * underlying a <code>fiber</code>. It provides the suspension hooks required to
         * construct, start, pause, and finalize the coroutine, and enforces the
         * <b>no-exception</b> execution model.
         *
         * A <code>fiber</code> does not propagate exceptions. Any uncaught exception is
         * treated as a fatal error and results in <code>std&#58;&#58;terminate()</code>,
         * mirroring the behavior of a thread whose entry function throws without being
         * caught. This prevents the coroutine from continuing after a corrupted state.
         *
         * <h5>Responsibilities</h5>
         * <ul>
         *   <li>Construct and return the bound <code>fiber</code> object.</li>
         *   <li>Provide suspension points at coroutine start and completion.</li>
         *   <li>Enforce the rule that the coroutine body must not allow exceptions to escape.</li>
         *   <li>Transform <code>co_await resume_tag</code> into a passive suspension awaiter.</li>
         * </ul>
         *
         * <h5>Coroutine Lifecycle Hooks</h5>
         * <ul>
         *   <li><code>get_return_object()</code> — constructs the associated <code>fiber</code>.</li>
         *   <li><code>initial_suspend()</code> — suspends before first execution step.</li>
         *   <li><code>final_suspend()</code> — suspends after completion to ensure safe destruction.</li>
         *   <li><code>return_void()</code> — indicates successful completion without a return value.</li>
         *   <li><code>unhandled_exception()</code> — aborts the program via <code>std&#58;&#58;terminate()</code>.</li>
         * </ul>
         *
         * <h5>Awaiter Mechanism</h5>
         * <p>
         * The expression <code>co_await resume_tag</code> produces a simple awaiter
         * that behaves like a controlled suspension point. It performs no scheduling,
         * does not enqueue work, and resumes only when the outer <code>resume()</code>
         * method is called.
         * </p>
         * <ul>
         *   <li><code>await_ready()</code> — always returns <code>false</code>.</li>
         *   <li><code>await_suspend()</code> — performs no action.</li>
         *   <li><code>await_resume()</code> — no-op.</li>
         * </ul>
         *
         * <h5>Compiler Notes</h5>
         * <p>
         * Certain functions contain lines such as:
         * </p>
         * <pre><code>[[maybe_unused]] auto *self = this;</code></pre>
         * <p>
         * These are intentional diagnostic suppressors to prevent tools from
         * incorrectly suggesting <code>static</code> conversion for coroutine methods
         * which must remain instance-bound according to the coroutine specification.
         * </p>
         */
        struct promise_type {

            /**
             * @brief Creates and returns the generator object.
             * @return A fiber instance bound to this coroutine promise.
             */
            fiber get_return_object() {
                return fiber{std::coroutine_handle<promise_type>::from_promise(*this)};
            }

            /**
             * @brief Suspends execution initially.
             * @return Always returns <code>std::suspend_always</code> to suspend execution at the start.
             */
            std::suspend_always initial_suspend() noexcept {
                [[maybe_unused]] auto *self = this; // Prevents Clang-Tidy from suggesting static
                return {};
            }

            /**
             * @brief Suspends execution at the final stage.
             * @return Always returns <code>std::suspend_always</code> to suspend execution at the end.
             */
            std::suspend_always final_suspend() noexcept {
                [[maybe_unused]] auto *self = this; // Prevents Clang-Tidy from suggesting static
                return {};
            }

            /**
             * @brief Indicates normal completion of the coroutine.
             */
            void return_void() noexcept {}

            /**
             * @brief Handles uncaught exceptions inside the coroutine.
             * @details
             * Fiber execution does not propagate exceptions; any uncaught error causes
             * <code>std::terminate()</code>, matching the behavior of an OS thread
             * whose entry function throws without being caught.
             */
            void unhandled_exception() {
                [[maybe_unused]] auto *self = this;
                std::terminate();
            }

            /**
             * @brief Transforms <code>co_await resume_tag</code> into an awaiter.
             * @details
             * This awaiter introduces a controlled suspension point similar to
             * <code>condition_variable.wait()</code>. The awaiter:
             * <ul>
             *   <li>always suspends,</li>
             *   <li>performs no scheduling,</li>
             *   <li>resumes only when <code>fiber.resume()</code> is called.</li>
             * </ul>
             *
             * @return A lightweight awaiter object.
             */
            auto await_transform(resume_t) noexcept {
                struct awaiter {
                    /**
                     * @brief Indicates that suspension is always required.
                     * @return Always <code>false</code>.
                     */
                    [[maybe_unused]] [[nodiscard]] bool await_ready() const noexcept {
                        [[maybe_unused]] auto *self = this;
                        return false;
                    }

                    /// @brief Performs no action during suspension.
                    [[maybe_unused]] void await_suspend(std::coroutine_handle<>) noexcept {}

                    /// @brief No-op resume handler.
                    [[maybe_unused]] void await_resume() const noexcept {}
                };
                [[maybe_unused]] auto *self = this;

                return awaiter{};
            }
        };

        /**
         * @brief Constructs a fiber from a coroutine handle.
         *
         * @details
         * The passed handle represents an allocated coroutine frame created by
         * <code>promise_type.get_return_object()</code>. Ownership is adopted by the
         * <code>fiber</code> instance, which is responsible for destroying the frame when no
         * longer needed.
         *
         * @param h The coroutine handle associated with this fiber.
         */
        explicit fiber(std::coroutine_handle<promise_type> h) noexcept
                : co_ro(h) {}

        /**
         * @brief Destroys the fiber and its coroutine frame if valid.
         *
         * @details
         * If the internal coroutine handle is non-null, its coroutine frame is destroyed
         * via <code>destroy()</code>. This fully releases all memory associated with the
         * coroutine.
         */
        ~fiber() {
            if (co_ro) co_ro.destroy();
        }

        /**
         * @brief Copy construction is disabled.
         *
         * @details
         * A fiber represents exclusive ownership of a coroutine frame; copying the
         * handle would produce double-destruction and is therefore prohibited.
         */
        fiber(const fiber &) = delete;

        /// @brief Copy assignment is disabled.
        fiber &operator=(const fiber &) = delete;

        /**
         * @brief Move constructor transferring ownership of the coroutine handle.
         *
         * @param other Source fiber whose handle will be moved from.
         */
        fiber(fiber &&other) noexcept
                : co_ro(std::exchange(other.co_ro, nullptr)) {}

        /**
         * @brief Move assignment operator transferring ownership of the coroutine handle.
         *
         * @details
         * Destroys the current coroutine frame if present, then adopts the frame from
         * <code>other</code>. The source fiber is left in a null state.
         *
         * @param other Source fiber.
         * @return <code>*this</code>.
         */
        fiber &operator=(fiber &&other) noexcept {
            if (this != &other) {
                if (co_ro) co_ro.destroy();
                co_ro = std::exchange(other.co_ro, nullptr);
            }
            return *this;
        }

        /**
         * @brief Checks whether the fiber has reached its final suspend point.
         *
         * @details
         * A fiber is considered complete if:
         * <ul>
         *   <li>it has no coroutine handle, or</li>
         *   <li>its coroutine handle reports <code>done() == true</code>.</li>
         * </ul>
         *
         * @return <code>true</code> if the coroutine is finished; otherwise <code>false</code>.
         */
        [[nodiscard]] bool done() const noexcept {
            return (!co_ro || done_flag);
        }

        /**
         * @brief Resumes execution of the fiber.
         *
         * @details
         * If the fiber is already complete, the function returns <code>false</code>.
         * Otherwise, it resumes the coroutine, advancing it until the next suspension
         * point (<code>co_await resume_tag</code>) or final completion.
         *
         * @return <code>true</code> if further progress is possible;
         *         <code>false</code> if the fiber has finished execution.
         */
        bool resume() // NOLINT (readability-const-return-type)
        {
            if (done()) return false;

            co_ro.resume();

            if (co_ro.done())
                done_flag = true;

            return !done_flag;
        }

        std::coroutine_handle<promise_type> co_ro; ///< Handle to the coroutine.
    };
}