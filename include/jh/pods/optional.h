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
 * @file optional.h
 * @brief POD-safe <code>optional&lt;T&gt;</code> with constexpr-capable union storage.
 *
 * <h3>Design Goals:</h3>
 * <ul>
 *   <li>Strict POD semantics (<code>pod_like</code> required)</li>
 *   <li>Trivial union storage + 1 flag, no non-trivial lifetime work</li>
 *   <li>Safe in <code>pod::array</code>, serialization, and mmap'd memory</li>
 *   <li>ABI stable: inline T storage and a presence flag, with normal ABI padding</li>
 * </ul>
 *
 * @note Unlike <code>std::optional</code>, this type only accepts POD-like T and never runs
 *       non-trivial constructors or destructors.
 */

#pragma once

#include "jh/pods/pod_like.h"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <memory>

namespace jh::pod {
    /**
     * @brief POD-compatible optional wrapper.
     *
     * Stores <code>T</code> in a trivial union and a boolean flag.
     * Provides POD-level semantics similar to <code>std::optional</code>.
     *
     * <h4>Equality Semantics:</h4>
     * <ul>
     *   <li>If one has value and the other not &rarr; false</li>
     *   <li>If both are empty &rarr; true (ignores raw storage)</li>
     *   <li>If both have value &rarr; compare storage bytes (<code>memcmp</code>)</li>
     * </ul>
     *
     * <h4>Usage Model:</h4>
     * <ul>
     *   <li>Use <code>.store()</code> to assign</li>
     *   <li>Check <code>.has()</code> / <code>.empty()</code> before access</li>
     *   <li>Use <code>.ref()</code> / <code>.get()</code> to access value</li>
     *   <li>Use <code>.value_or()</code> for fallback</li>
     * </ul>
     *
     * @tparam T Element type. Must satisfy <code>pod_like&lt;T&gt;</code> and be
     *           <b>free of const/volatile qualifiers</b> (i.e. satisfy <code>cv_free_pod_like&lt;T&gt;</code>).
     *           Using a cv-qualified type would make the optional itself non-POD due to
     *           loss of trivial assignment and standard layout guarantees.
     */
    template<cv_free_pod_like T>
    struct alignas(alignof(T)) optional final {
        union storage_type {
            T value;
            std::byte bytes[sizeof(T)];
        } storage;                     ///< Inline storage for T or its raw representation.
        bool has_value;                ///< Presence flag (true = has value).

        using value_type = T;          ///< Alias of contained type.

        /// @brief Default constructor (empty state).
        constexpr optional() noexcept = default;

        /**
         * @brief Store a value using T's trivial copy construction.
         * @param value Source value to copy.
         */
        constexpr void store(const T &value) noexcept {
            if (std::is_constant_evaluated()) {
                if constexpr (std::is_copy_constructible_v<T>) {
                    std::construct_at(std::addressof(storage.value), value);
                } else {
                    std::abort();
                }
            } else {
                std::memcpy(std::addressof(storage.value), std::addressof(value), sizeof(T));
            }
            has_value = true;
        }

        /// @brief Clear the stored value (set to empty).
        constexpr void clear() noexcept { has_value = false; }

        /**
         * @brief Get mutable pointer to stored value.
         * @return Pointer to active <code>T</code>, must check <code>.has()</code> first.
         */
        constexpr T *get() noexcept {
            return std::addressof(storage.value);
        }

        /**
         * @brief Get const pointer to stored value.
         * @return Pointer to active const <code>T</code>, must check <code>.has()</code> first.
         */
        [[nodiscard]] constexpr const T *get() const noexcept {
            return std::addressof(storage.value);
        }

        /// @brief Whether a value is present.
        [[nodiscard]] constexpr bool has() const noexcept { return has_value; }

        /// @brief Whether the optional is empty.
        [[nodiscard]] constexpr bool empty() const noexcept { return !has_value; }

        /**
         * @brief Access stored value by reference.
         * @return Reference to <code>T</code>. Undefined if <code>.has() == false</code>.
         */
        [[nodiscard]] constexpr T &ref() noexcept { return *get(); }

        /**
         * @brief Access stored value by const reference.
         * @return Const reference to <code>T</code>. Undefined if <code>.has() == false</code>.
         */
        [[nodiscard]] constexpr const T &ref() const noexcept { return *get(); }

        /**
         * @brief Return stored value or fallback.
         * @param fallback Value to return if empty.
         * @return Copy of stored or fallback value.
         */
        [[nodiscard]] constexpr T value_or(T fallback) const noexcept {
            return has_value ? ref() : fallback;
        }

        /**
         * @brief Equality comparison with another optional.
         *
         * Semantics are aligned with <code>std::optional</code>:
         * <ul>
         *   <li>If one has a value and the other does not &rarr; <code>false</code></li>
         *   <li>If both are empty &rarr; <code>true</code></li>
         *   <li>If both have a value &rarr; compare the underlying storage bytes</li>
         * </ul>
         *
         * @param rhs Other optional to compare with.
         * @return <code>true</code> if both optionals have the same state and (if present)
         *         identical raw byte content, <code>false</code> otherwise.
         *
         * @note This operator does not rely on <code>= default</code>, because the default
         *       comparison would also require raw <code>storage</code> equality when
         *       <code>has_value == false</code>. That would force meaningless zeroing of
         *       storage in <code>.clear()</code>. Instead, we define comparison explicitly:
         *       empty optionals are always equal regardless of storage content.
         * @note Runtime comparison uses <code>std::memcmp</code>. Constant evaluation compares
         *       scalar values or uses a constexpr object-representation comparison.
         */
        constexpr bool operator==(const optional &rhs) const noexcept {
            if (has_value != rhs.has_value) return false;
            if (!has_value) return true;
            if (std::is_constant_evaluated()) {
                if constexpr (requires(const T &lhs, const T &other) { lhs == other; }) {
                    return static_cast<bool>(storage.value == rhs.storage.value);
                } else {
                    const auto lhs = std::bit_cast<std::array<std::byte, sizeof(T)>>(storage.value);
                    const auto other = std::bit_cast<std::array<std::byte, sizeof(T)>>(rhs.storage.value);
                    return lhs == other;
                }
            }
            return std::memcmp(
                    std::addressof(storage.value),
                    std::addressof(rhs.storage.value),
                    sizeof(T)
            ) == 0;
        }
    };

    /**
     * @brief Construct an <code>optional&lt;T&gt;</code> with a value.
     * @param value Value to copy into optional.
     * @return Filled <code>optional&lt;T&gt;</code> with <code>.has() == true</code>.
     */
    template<pod_like T>
    [[nodiscard]] constexpr optional<T> make_optional(const T &value) noexcept {
        optional<T> o;
        o.store(value);
        return o;
    }
} // namespace jh::pod
