/**
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
 */


/**
 * @file generator.h
 * @author JeongHan-Bae <mastropseudo@gmail.com>
 * @brief A coroutine-based generator for modern C++20, inspired by Python's generator system.
 *
 * @details
 * This module implements a coroutine-based generator using C++20 coroutines and templates.
 * It is designed to provide a Pythonic generator experience in C++ while maintaining
 * performance and type safety.
 *
 * ## Motivation
 * Python's generator system allows for both **iteration (yielding values)** and **interaction (sending values)**.
 * This module retains Python's **yield type** (`T`) and **input type** (`U`), similar to Python's `Generator[T, U, R]`.
 * However, unlike Python, **return type `R` is omitted**, as it is considered unnecessary in most practical engineering cases.
 * If a return value is needed, developers can use **smart pointers or references** to provide an explicit output channel.
 *
 * ## Key Features
 * - Supports **iterative** and **interactive** coroutine-based generators.
 * - Allows sending values into the coroutine (`send()`).
 * - Provides Pythonic **iteration** using `next()`.
 * - Supports **automatic iteration & sending** with `send_ite()`.
 * - Provides utilities for converting generators to **std::vector** and **std::list**.
 * - Allows for a seamless migration from **Python-based generator logic to C++20** without losing clarity.
 *
 * @version 1.1.x
 * @date 2025
 */

#pragma once

#include <coroutine>
#include <list>
#include <optional>
#include <stdexcept>
#include <utility>            // NOLINT for std::exchange
#include <variant>            // NOLINT for std::monostate in g++
#include <vector>

#include "sequence.h"
#include "iterator.h"

namespace jh {
    /**
     * @brief A coroutine-based generator that supports yielding values and receiving inputs.
     *
     * @tparam T The type of values produced by the generator.
     * @tparam U The type of values that can be sent to the generator.
     *           If left as the default (`std::monostate`), the generator does not require input values and can function as a simple iterable sequence.
     */
    template<typename T, typename U = std::monostate>
    struct generator final {
        /**
         * @brief Type alias for the value type produced by the generator.
         * @details
         * This defines the type of values that are yielded by the generator.
         * It enables compatibility with standard iterator traits.
         */
        using value_type [[maybe_unused]] = T;

        /**
         * @brief Type alias for the iterator associated with the generator.
         * @details
         * This defines `generator<T, U>`'s iterator type as `iterator<generator>`.
         * - The iterator is always defined, regardless of `U` (i.e., whether the generator supports `send()` or not).
         * - However, `begin()` and `end()` are only available when `U == std::monostate` (i.e., the generator does not require `send()`).
         * - This allows users to manually create iterators if needed, although we do not encourage inheritance-based customization.
         */
        using iterator = jh::iterator<generator>; // NOLINT Fixes GCC issue with iterator lookup, ensuring cross-platform stability

        /**
         * @brief Deleted copy constructor.
         * @details
         * Since `generator<T, U>` manages a coroutine handle (`std::coroutine_handle<promise_type>`),
         * copying the generator would lead to double ownership issues.
         * - To prevent accidental copies, the copy constructor is explicitly deleted.
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
         * Transfers ownership of the coroutine handle from `other` to `this`.
         * - The `other` generator is set to `nullptr` to prevent double destruction.
         * - This ensures safe movement of generator instances.
         *
         * @param other The generator to move from.
         */
        generator(generator &&other) noexcept : co_ro(std::exchange(other.co_ro, nullptr)) {
        }

        /**
         * @brief Move assignment operator.
         * @details
         * Details:
         * - First, it stops the current coroutine if it exists.
         * - Then, it transfers ownership of the coroutine handle from `other` to `this`.
         * - The `other` generator is set to `nullptr` to prevent double destruction.
         * - This ensures safe assignment of generator instances.
         *
         * @param other The generator to move from.
         * @return Reference to `this` generator after assignment.
         */
        generator &operator=(generator &&other) noexcept {
            if (this != &other) {
                stop();
                co_ro = std::exchange(other.co_ro, nullptr);
            }
            return *this;
        }

        /**
         * @brief The promise type required for coroutine functionality.
         */
        struct promise_type {
            std::optional<T> current_value; ///< Stores the current yielded value.
            std::optional<U> last_sent_value; ///< Stores the last value sent to the generator.
            std::exception_ptr exception; ///< Stores an exception if one occurs.

            /**
             * @brief Creates and returns the generator object.
             * @return A generator instance bound to this coroutine promise.
             */
            generator get_return_object() {
                return generator{std::coroutine_handle<promise_type>::from_promise(*this)};
            }

            /**
             * @brief Suspends execution initially.
             * @return Always returns std::suspend_always to suspend execution at the start.
             */
            std::suspend_always initial_suspend() noexcept {
                [[maybe_unused]] auto *self = this; // Prevents Clang-Tidy from suggesting static
                return {};
            }

            /**
             * @brief Suspends execution at the final stage.
             * @return Always returns std::suspend_always to suspend execution at the end.
             */
            std::suspend_always final_suspend() noexcept {
                [[maybe_unused]] auto *self = this; // Prevents Clang-Tidy from suggesting static
                return {};
            }

            /**
             * @brief Yields a value from the generator.
             * @param value The value to yield.
             * @return Always returns std::suspend_always to suspend execution after yielding.
             */
            std::suspend_always yield_value(T value) {
                current_value = value;
                return {};
            }

            /**
             * @brief Awaiter structure to manage `co_await` operations for receiving values.
             */
            struct awaiter {
                promise_type &promise; ///< Reference to the coroutine promise.

                [[maybe_unused]] static bool await_ready() { return false; } ///< Always returns false to suspend.

                [[maybe_unused]] static void await_suspend(std::coroutine_handle<>) {
                } ///< No-op on suspension.

                /**
                 * @brief Retrieves the last sent value or a default.
                 * @return The most recent value sent to the generator.
                 */
                [[maybe_unused]] U await_resume() {
                    return promise.last_sent_value.value_or(U{});
                }
            };

            /**
             * @brief Transforms an awaited value into an `awaiter` for coroutine execution.
             * @return An `awaiter` object tied to this coroutine promise.
             */
            awaiter await_transform(U) noexcept {
                return awaiter{*this}; // Ensures Clang-Tidy does not suggest making it static
            }

            /**
             * @brief Defines the behavior when the coroutine completes.
             */
            void return_void() noexcept { [[maybe_unused]] auto *self = this; }

            /**
             * @brief Handles uncaught exceptions by storing them.
             */
            void unhandled_exception() {
                exception = std::current_exception();
            }
        };

        std::coroutine_handle<promise_type> co_ro; ///< Handle to the coroutine.

        /**
         * @brief Constructs the generator with a coroutine handle.
         * @param h The coroutine handle to be managed.
         */
        explicit generator(std::coroutine_handle<promise_type> h) : co_ro(h) {
        }

        /**
         * @brief Destroys the coroutine handle if it exists.
         */
        ~generator() { if (co_ro) co_ro.destroy(); }

        /**
         * @brief Advances the generator to the next value.
         * @return `true` if a new value is available, `false` if the coroutine has finished.
         */
        bool next() {
            if (!co_ro || co_ro.done()) return false;
            co_ro.resume();
            if (co_ro.promise().exception) std::rethrow_exception(co_ro.promise().exception);
            return !co_ro.done();
        }

        /**
         * @brief Checks if the generator has completed execution.
         * @return `true` if the generator has finished, `false` otherwise.
         */
        [[nodiscard]] bool done() const {
            return !co_ro || co_ro.done();
        }

        /**
         * @brief Sends a value to the generator and resumes execution.
         * @param value The value to send.
         * @return `true` if the coroutine is still active, `false` otherwise.
         */
        bool send(U value) {
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
         * This function combines `next()` and `send()`, eliminating the need for a separate `next()` call.
         * It first advances the generator, and if successful, sends the provided value to the coroutine.
         *
         * @param value The value to send to the generator.
         * @return `true` if the generator successfully advances and accepts the value, `false` if the generator has finished.
         */
        bool send_ite(U value) {
            if (!co_ro || co_ro.done()) return false;

            co_ro.resume(); // Advance coroutine
            if (co_ro.promise().exception) std::rethrow_exception(co_ro.promise().exception);

            co_ro.promise().last_sent_value = value; // Store input value
            co_ro.resume(); // Resume coroutine after sending

            return !co_ro.done();
        }

        /**
         * @brief Retrieves the current yielded value.
         * @return An optional containing the current value, or empty if none.
         */
        std::optional<T> value() {
            return co_ro.promise().current_value;
        }

        /**
         * @brief Retrieves the last value sent to the generator.
         * @return An optional containing the last sent value.
         */
        [[maybe_unused]] std::optional<U> last_sent_value() {
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
         * This method allows the generator to be used in `for(auto x : gen)`.
         * It is only enabled when `U == std::monostate` (i.e., no `send()` required).
         * - Since iterating over a generator inherently consumes its elements,
         * a `const` version is intentionally disallowed (`begin() const = delete`).
         * - This ensures that a generator cannot be iterated over while preserving its state.
         *
         * @return An iterator to begin iteration over the generator.
         */
        iterator begin() requires std::is_same_v<U, std::monostate> {
            return iterator{*this};
        }

        /**
         * @brief Deleted `const` overload of `begin()`.
         * @details
         * Since a generator's iteration state is modified each time an element is consumed,
         * allowing a `const` version would be misleading. This deletion enforces the
         * expectation that generators are mutable objects.
         */
        iterator begin() const = delete;

        /**
         * @brief Returns an iterator representing the end of the generator sequence.
         * @details
         * This iterator serves as the "past-the-end" iterator, marking the termination
         * of iteration. Unlike `begin()`, `end()` does not consume values and is safe to call
         * multiple times.
         * Like `begin()`, `end()` is only enabled when `U == std::monostate` (i.e., no `send()` required).
         *
         * @return An iterator representing the end of the generator.
         */
        iterator end() const requires std::is_same_v<U, std::monostate> {
            return iterator{};
        }
    };

    /**
     * @brief Iterator for coroutine-based generators.
     * @details
     * This iterator is designed exclusively for range-based iteration over a `generator<T>`.
     * It enables standard input iterator behavior (`++`, `*`, `==`), but **should never be manually instantiated**.
     * Instead, it should be obtained via `generator::begin()` and `generator::end()`.
     * - Only generators **without input values** (`U == std::monostate`) can provide iterators.
     * - If a generator requires `send()`, it **cannot** be iterated using `for(auto x : gen)`,
     *   as input values must be explicitly provided at each step.
     * - If the generator is consumed or destroyed elsewhere, the iterator **becomes invalid**.
     * - Instead of using `std::shared_ptr<G>`, this iterator holds a `std::optional<std::reference_wrapper<G>>`.
     * This prevents unintended ownership extension, as the generator is managed by its coroutine.
     * If the generator reaches its end or is destroyed, the iterator safely resets itself.
     *
     * @tparam T The generator value_type.
     * @tparam U The generator send_type.
     */
    template<typename T, typename U>
    struct iterator<generator<T, U> > {
        using iterator_category [[maybe_unused]] = std::input_iterator_tag;
        using value_type = T;
        using type = iterator;
        using difference_type [[maybe_unused]] = std::ptrdiff_t;
        using pointer = value_type *;
        using reference [[maybe_unused]] = value_type &;

        std::optional<std::reference_wrapper<generator<T, U> > > gen;
        ///< Reference to the generator (optional to handle end-state).

        std::optional<value_type> current_value; ///< Stores the currently yielded value.

        /**
         * @brief Constructs an iterator bound to a generator.
         * @details
         * This constructor should only be called internally via `generator::begin()`.
         * Manual construction of iterators is discouraged.
         *
         * @param generator The generator to iterate over.
         */
        explicit iterator(generator<T, U> &generator) : gen(generator), is_begin(true) {
        }

        /**
         * @brief Constructs an end iterator.
         * @details
         * This constructor is only used internally by `generator::end()`, representing the past-the-end state.
         */
        iterator(): gen(std::nullopt), is_begin(false) {
        }

        /**
         * @brief Advances the iterator to the next value.
         * @details
         * If the generator produces a new value, `current_value` is updated.
         * If the generator is exhausted, the reference is cleared to mark iteration as complete.
         * @return Reference to the updated iterator.
         */
        iterator &operator++() {
            begin_check();
            if (gen && gen->get().next()) {
                current_value = gen->get().value();
            } else {
                current_value = std::nullopt;
                gen.reset(); // Generator exhausted, clear reference.
            }
            return *this;
        }

        /**
         * @brief Advances the iterator and returns its previous state.
         * @details
         * This function follows the standard input iterator convention of returning a
         * copy of the iterator before incrementing. However, since the generator is **single-pass
         * and consumable**, there is no actual "previous state" to return.
         * Unlike bidirectional or random-access iterators, where the previous value remains
         * accessible, advancing a generator **consumes the current value irreversibly**.
         * Therefore, although `temp` is a copy of `*this` before `++`, **both `temp` and `*this`
         * will have advanced to the next state**, making them effectively identical.
         * This operator is still useful for compatibility with input iterator interfaces,
         * but users should note that it does not preserve past values.
         *
         * @return Copy of the iterator (which, in reality, has already advanced).
         */
        iterator operator++(int) {
            iterator temp = *this;
            ++*this;
            return temp;
        }

        /**
         * @brief Dereferences the iterator to access the current value.
         * @throws std::runtime_error If attempting to dereference an end iterator.
         * @return Reference to the current value.
         */
        const value_type &operator*() {
            begin_check();
            if (!current_value)
                throw std::runtime_error(
                    "Attempted to dereference an end iterator. Ensure that iteration has started and is not finished.");
            return *current_value;
        }

        /**
         * @brief Provides pointer access to the current value.
         * @throws std::runtime_error If attempting to access an end iterator.
         * @return Pointer to the current value.
         */
        const value_type *operator->() {
            begin_check();
            if (!current_value) throw std::runtime_error("Dereferencing end iterator");
            return &*current_value;
        }

        /**
         * @brief Compares two iterators for equality.
         * @details
         * Two iterators are considered equal if they:
         * - Reference the same generator instance and hold the same value, or
         * - Are both past-the-end iterators (`gen` is `std::nullopt`).
         * - Handling Generator Expiration:
         *    - If the generator is exhausted or explicitly destroyed elsewhere,
         *      `gen` is set to `std::nullopt`.
         *    - This ensures that an iterator with a lost reference is treated **equivalently
         *      to an end iterator**.
         *    - As a result, once the generator is gone, all its iterators become equivalent
         *      to `end()` and will compare equal.
         * - This behavior ensures safe iteration without accessing invalid memory
         * if the generator is destroyed mid-iteration.
         *
         * @param other The iterator to compare against.
         * @return `true` if iterators are equal, otherwise `false`.
         */
        bool operator==(const iterator &other) const {
            // Ensure both iterators are working with valid generators
            const_cast<iterator *>(this)->check_validity();
            const_cast<iterator *>(&other)->check_validity();

            return gen.has_value() == other.gen.has_value() &&
                   (!gen.has_value() || std::addressof(gen.value().get()) == std::addressof(other.gen.value().get())) &&
                   current_value == other.current_value;
        }

        /**
         * @brief Compares two iterators for inequality.
         * @param other The iterator to compare against.
         * @return `true` if iterators are not equal, otherwise `false`.
         */
        bool operator!=(const iterator &other) const {
            return !(*this == other);
        }

    private:
        bool is_begin; ///< Tracks whether this is the initial state of iteration.

        /**
         * @brief Ensures that iteration starts correctly.
         * @details
         * When iteration begins, this function ensures that the first value is
         * retrieved immediately, avoiding an off-by-one error.
         * If the generator is already exhausted, the iterator resets itself.
         */
        void begin_check() {
            if (is_begin) {
                if (gen && gen->get().next()) {
                    current_value = gen->get().value();
                } else {
                    current_value = std::nullopt;
                    gen.reset();
                }
                is_begin = false;
            }
        }

        /**
         * @brief Checks if the generator is still valid.
         * @details
         * If the generator has been destroyed or exhausted, this function resets `gen` to `std::nullopt`
         * to prevent invalid memory access.
         */
        void check_validity() {
            if (!gen.has_value()) return; // Already nullopt, nothing to do

            // Access the generator safely
            generator<T, U> *generator_ptr = std::addressof(gen.value().get());

            // If generator is deallocated or exhausted, reset `gen`
            if (!generator_ptr || generator_ptr->done()) {
                gen.reset();
                current_value.reset();
            }
        }
    };


    /// @name Generator Conversion Functions
    /// @{

    /**
     * @brief Converts a sequence into a generator.
     * @tparam T The sequence type.
     * @param seq The sequence to convert.
     * @return A generator yielding elements from the sequence.
     */
    template<sequence T>
    generator<sequence_value_type<T> > make_generator(const T &seq) {
        for (const auto &elem: seq) {
            // ✅ Use range-based for-loop
            co_yield elem;
        }
    }

    /**
     * @brief Converts a generator to a std::vector.
     * @param gen The generator to convert.
     * @return A std::vector containing all generated values.
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
     * @brief Converts a generator to a std::vector using a single input value.
     * @param gen The generator to convert.
     * @param input_value The input value to send at each step.
     * @return A std::vector containing all generated values.
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
     * @brief Converts a generator to a std::vector using a sequence of input values.
     * @param gen The generator to convert.
     * @param inputs A std::vector of input values to send sequentially.
     * @return A std::vector containing all generated values.
     */
    template<typename T, typename U>
    std::vector<T> to_vector(generator<T, U> &gen, const std::vector<U> &inputs) {
        std::vector<T> result;
        size_t index = 0;

        if (!inputs.empty()) {
            if (!gen.send(inputs[index])) return result;
            while (gen.next()) {
                // ✅ Ensure each step processes a new input
                result.push_back(gen.value().value());
                if (++index >= inputs.size()) break;
                if (!gen.send(inputs[index])) break; // ✅ Ensure input is used immediately
            }
        }

        return result;
    }

    /**
     * @brief Converts a generator to a std::list.
     * @param gen The generator to convert.
     * @return A std::list containing all generated values.
     */
    template<typename T>
    std::list<T> to_list(generator<T> &gen) {
        std::list<T> result;
        while (gen.next()) {
            result.push_back(gen.value().value());
        }
        return result;
    }

    /**
     * @brief Converts a generator to a std::list using a single input value.
     * @param gen The generator to convert.
     * @param input_value The input value to send at each step.
     * @return A std::list containing all generated values.
     */
    template<typename T, typename U>
    std::list<T> to_list(generator<T, U> &gen, U input_value) {
        std::list<T> result;
        while (gen.next()) {
            if (!gen.send(input_value)) break;
            result.push_back(gen.value().value());
        }
        return result;
    }

    /**
     * @brief Converts a generator to a std::list using a sequence of input values.
     * @param gen The generator to convert.
     * @param inputs A std::vector of input values to send sequentially.
     * @return A std::list containing all generated values.
     */
    template<typename T, typename U>
    std::list<T> to_list(generator<T, U> &gen, const std::vector<U> &inputs) {
        std::list<T> result;
        size_t index = 0;

        if (!inputs.empty()) {
            if (!gen.send(inputs[index])) return result;
            while (gen.next()) {
                // ✅ Ensure each step processes a new input
                result.push_back(gen.value().value());
                if (++index >= inputs.size()) break;
                if (!gen.send(inputs[index])) break; // ✅ Ensure input is used immediately
            }
        }

        return result;
    }

    /// @}
} // namespace jh
