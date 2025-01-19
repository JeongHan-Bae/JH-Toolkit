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
 * @version 1.0
 * @date 2025
 */

#pragma once

#include <coroutine>
#include <list>
#include <optional>
#include <vector>
#include <variant>

#include "sequence.h"

namespace jh {
    /**
     * @brief A coroutine-based generator that supports yielding values and receiving inputs.
     *
     * @tparam T The type of values produced by the generator.
     * @tparam U The type of values that can be sent to the generator.
     *           If left as the default (`std::monostate`), the generator does not require input values and can function as a simple iterable sequence.
     */
    template<typename T, typename U = std::monostate>
    struct generator {

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

                static bool await_ready() { return false; } ///< Always returns false to suspend.

                static void await_suspend(std::coroutine_handle<>) {
                } ///< No-op on suspension.

                /**
                 * @brief Retrieves the last sent value or a default.
                 * @return The most recent value sent to the generator.
                 */
                U await_resume() {
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
         * @brief Sends a value to the generator and resumes execution.
         * @param value The value to send.
         * @return `true` if the coroutine is still active, `false` otherwise.
         */
        bool send(U value) {
            if (!co_ro || co_ro.done()) return false;

            auto &promise = co_ro.promise();
            promise.last_sent_value = value; // ✅ Explicitly reference instance
            co_ro.resume();

            // ✅ Ensure the next value is updated immediately
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
            if (!this->next()) return false;
            return this->send(value);
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
        std::optional<U> last_sent_value() {
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
    };

    /// @name Generator Conversion Functions
    /// @{

    /**
     * @brief Converts a sequence into a generator.
     * @tparam T The sequence type.
     * @param seq The sequence to convert.
     * @return A generator yielding elements from the sequence.
     */
    template <sequence T>
    generator<sequence_value_type<T>> make_generator(const T& seq) {
        for (const auto& elem : seq) { // ✅ Use range-based for-loop
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
