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
 * @file array.h (pods)
 * @brief Implementation of <code>jh::pod::array&lt;T, N&gt;</code>.
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include "jh/pods/pod_like.h"

namespace jh::pod {
    /// @brief Maximum size of a POD array (16KB). This is a compile-time constant.
    inline constexpr std::uint16_t max_pod_array_bytes = 16 * 1024;

    /**
     * @brief POD-compatible fixed-size array, similar in shape to <code>std::array</code>, but simpler and fully POD.
     *
     * @tparam T Element type. Must satisfy <code>pod_like&lt;T&gt;</code> and must be
     *           <b>free of const/volatile qualifiers</b> (i.e. satisfy <code>cv_free_pod_like&lt;T&gt;</code>).
     *           Using <code>const T</code> or <code>volatile T</code> would make the array itself
     *           non-POD due to loss of trivial assignment and copy semantics.
     * @tparam N Number of elements. Total memory (<code>sizeof(T) * N</code>) must not exceed 16KB.
     *
     * This structure is designed for:
     * <ul>
     *   <li>Raw memory containers (<code>pod_stack</code>, <code>arena</code>)</li>
     *   <li>In-place value blocks (<code>placement-new</code>, <code>mmap</code>, <code>.data</code> segments)</li>
     *   <li>Zero-allocation, constexpr-safe stack usage</li>
     * </ul>
     *
     * <h4>Design Constraints:</h4>
     * <ul>
     *   <li>Memory is fully inline and contiguous (<code>T data[N]</code>)</li>
     *   <li>Compile-time limited to 16KB for safety and portability</li>
     *   <li>Supports <code>operator[]</code>, range-based for-loops, <code>==</code> comparison</li>
     *   <li>No bounds checking — required to preserve POD/constexpr/noexcept semantics.
     *       Since the layout is trivial, most out-of-bounds cases can be caught at compile-time.</li>
     * </ul>
     *
     * @note This is <b>not</b> a drop-in replacement for <code>std::array</code>. It has:
     * <ul>
     *   <li>No <code>.at()</code>, <code>.fill()</code>, <code>.swap()</code> helpers</li>
     *   <li>No allocator, and bounds safety deliberately omitted to retain POD/constexpr semantics</li>
     * </ul>
     *
     * @warning Do not use this for large arrays or heap-like buffers.
     */
    template<cv_free_pod_like T, std::uint16_t N> requires (sizeof(T) * N <= max_pod_array_bytes)
    struct alignas(alignof(T)) array final {
        T data[N];                             ///< Inline contiguous storage for N elements of type T.

        using value_type = T;                                     ///< Value type alias.
        using size_type = std::uint16_t;                          ///< Size type alias (16-bit).
        using difference_type [[maybe_unused]] = std::ptrdiff_t;  ///< Difference type alias.
        using reference = value_type &;                           ///< Reference type.
        using const_reference = const value_type &;               ///< Const reference type.
        using pointer = value_type *;                             ///< Pointer type.
        using const_pointer = const value_type *;                 ///< Const pointer type.

        /// @brief Access element by index (no bounds checking).
        constexpr reference operator[](std::size_t i) noexcept { return data[i]; }

        /// @brief Access element by index (const, no bounds checking).
        constexpr const_reference operator[](std::size_t i) const noexcept { return data[i]; }

        /// @brief Get pointer to beginning of array.
        constexpr pointer begin() noexcept { return data; }

        /// @brief Get const pointer to beginning of array.
        [[nodiscard]] constexpr const_pointer begin() const noexcept { return data; }

        /// @brief Get pointer to end of array (one past last element).
        constexpr pointer end() noexcept { return data + N; }

        /// @brief Get const pointer to end of array (one past last element).
        [[nodiscard]] constexpr const_pointer end() const noexcept { return data + N; }

        /// @brief Return the number of elements in the array.
        [[nodiscard]] static constexpr size_type size() noexcept { return N; }

        /// @brief Compare two arrays for equality (element-wise).
        constexpr bool operator==(const array &) const = default;
    };
}
