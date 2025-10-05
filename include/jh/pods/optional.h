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
 * @file optional.h (pods)
 * @brief POD-safe <code>optional&lt;T&gt;</code> with raw storage.
 *
 * <h3>Design Goals:</h3>
 * <ul>
 *   <li>Strict POD semantics (<code>pod_like</code> required)</li>
 *   <li>Raw byte buffer + 1 flag, no constructors or destructors</li>
 *   <li>Safe in <code>pod::array</code>, serialization, and mmap’d memory</li>
 *   <li>ABI predictable (<code>sizeof(optional&lt;T&gt;) == sizeof(T) + 1</code>)</li>
 * </ul>
 *
 * @note Unlike <code>std::optional</code>, this type never runs constructors or destructors.
 * @note Functions rely on <code>reinterpret_cast</code>/<code>std::launder</code> and therefore
 *       cannot be used in <code>consteval</code> contexts.
 */

#pragma once

#include "pod_like.h"
#include <new>
#include <cstring>

namespace jh::pod {
    /**
     * @brief POD-compatible optional wrapper.
     *
     * Stores raw bytes for <code>T</code> and a boolean flag.
     * Provides POD-level semantics similar to <code>std::optional</code>.
     *
     * <h4>Equality Semantics:</h4>
     * <ul>
     *   <li>If one has value and the other not → false</li>
     *   <li>If both are empty → true (ignores raw storage)</li>
     *   <li>If both have value → compare storage bytes (<code>memcmp</code>)</li>
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
     * @tparam T Must satisfy <code>pod_like</code>.
     */
    template<pod_like T>
    struct alignas(alignof(T)) optional final {
        std::byte storage[sizeof(T)];  ///< Raw storage; flattens type ABI, never access directly.
        bool has_value;                ///< Presence flag (true = has value).

        using value_type = T;          ///< Alias of contained type.

        /// @brief Default constructor (empty state).
        constexpr optional() noexcept = default;

        /**
         * @brief Store a value by copying raw memory.
         * @param value Source value to copy.
         */
        void store(const T &value) noexcept {
            std::memcpy(storage, std::addressof(value), sizeof(T));
            has_value = true;
        }

        /// @brief Clear the stored value (set to empty).
        void clear() noexcept { has_value = false; }

        /**
         * @brief Get mutable pointer to stored value.
         * @return Pointer to <code>T</code>, must check <code>.has()</code> before use.
         */
        T *get() noexcept {
            return std::launder(reinterpret_cast<T *>(&storage));
        }

        /**
         * @brief Get const pointer to stored value.
         * @return Pointer to const <code>T</code>, must check <code>.has()</code> before use.
         */
        [[nodiscard]] const T *get() const noexcept {
            return std::launder(reinterpret_cast<const T *>(&storage));
        }

        /// @brief Whether a value is present.
        [[nodiscard]] bool has() const noexcept { return has_value; }

        /// @brief Whether the optional is empty.
        [[nodiscard]] bool empty() const noexcept { return !has_value; }

        /**
         * @brief Access stored value by reference.
         * @return Reference to <code>T</code>. Undefined if <code>.has() == false</code>.
         */
        [[nodiscard]] T &ref() noexcept { return *get(); }

        /**
         * @brief Access stored value by const reference.
         * @return Const reference to <code>T</code>. Undefined if <code>.has() == false</code>.
         */
        [[nodiscard]] const T &ref() const noexcept { return *get(); }

        /**
         * @brief Return stored value or fallback.
         * @param fallback Value to return if empty.
         * @return Copy of stored or fallback value.
         */
        [[nodiscard]] T value_or(T fallback) const noexcept {
            return has_value ? ref() : fallback;
        }

        /**
         * @brief Equality comparison with another optional.
         *
         * Semantics are aligned with <code>std::optional</code>:
         * <ul>
         *   <li>If one has a value and the other does not → <code>false</code></li>
         *   <li>If both are empty → <code>true</code></li>
         *   <li>If both have a value → compare the underlying storage bytes</li>
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
         * @note Raw comparison is performed with <code>std::memcmp</code>, ensuring POD-level
         *       semantics without invoking <code>T::operator==</code>.
         */
        constexpr bool operator==(const optional &rhs) const noexcept {
            if (has_value != rhs.has_value) return false;
            if (!has_value) return true;
            return std::memcmp(storage, rhs.storage, sizeof(T)) == 0;
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
