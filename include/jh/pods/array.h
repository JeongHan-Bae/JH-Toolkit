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
 * @file array.h
 * @brief Implementation of <code>jh::pod::array&lt;T, N&gt;</code>.
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include "jh/pods/pod_like.h"
#include "jh/metax/expected.h"

namespace jh::pod {
    /// @brief Maximum size of a POD array (16KB). This is a compile-time constant.
    inline constexpr std::size_t max_pod_array_bytes = 16 * 1024;

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
     *   <li>Raw memory containers (<code>jh::runtime_arr</code>, <code>arena</code>)</li>
     *   <li>In-place value blocks (<code>placement-new</code>, <code>mmap</code>, <code>.data</code> segments)</li>
     *   <li>Zero-allocation, constexpr-safe stack usage</li>
     * </ul>
     *
     * <h4>Design Constraints:</h4>
     * <ul>
     *   <li>Memory is fully inline and contiguous (<code>T data[N]</code>)</li>
     *   <li>Compile-time limited to 16KB for safety and portability</li>
     *   <li>Supports <code>operator[]</code>, checked <code>at()</code>, range-based for-loops, and <code>==</code> comparison</li>
     *   <li><code>operator[]</code> remains unchecked; <code>at()</code> reports an out-of-range index
     *       through <code>expected</code> without throwing.</li>
     * </ul>
     *
     * @note This is <b>not</b> a drop-in replacement for <code>std::array</code>. It has:
     * <ul>
     *   <li>No <code>.fill()</code> or <code>.swap()</code> helpers</li>
     *   <li>No allocator; <code>operator[]</code> remains unchecked for POD/constexpr use cases</li>
     * </ul>
     *
     * @warning Do not use this for large arrays or heap-like buffers.
     */
    template<cv_free_pod_like T, std::size_t N> requires (sizeof(T) * N <= max_pod_array_bytes)
    struct alignas(alignof(T)) array final {
        /** @brief Failure reasons for checked array access. */
        enum class error_code : std::uint8_t {
            out_of_bounds
        };

        T data[N];                             ///< Inline contiguous storage for N elements of type T.

        using value_type = T;                           ///< Value type alias.
        using size_type = std::size_t;                  ///< Size type alias.
        using difference_type = std::ptrdiff_t;         ///< Difference type alias.
        using reference = value_type &;                 ///< Reference type.
        using const_reference = const value_type &;     ///< Const reference type.
        using pointer = value_type *;                   ///< Pointer type.
        using const_pointer = const value_type *;       ///< Const pointer type.

        /// @brief Access element by index (no bounds checking).
        constexpr reference operator[](std::size_t i) noexcept { return data[i]; }

        /// @brief Access element by index (const, no bounds checking).
        constexpr const_reference operator[](std::size_t i) const noexcept { return data[i]; }

        /** @brief Checked mutable element access. */
        [[nodiscard]] constexpr jh::meta::expected<T *, error_code>
        at(std::size_t i) noexcept {
            if (i >= N) return jh::meta::unexpected(error_code::out_of_bounds);
            return &data[i];
        }

        /** @brief Checked const element access. */
        [[nodiscard]] constexpr jh::meta::expected<const T *, error_code>
        at(std::size_t i) const noexcept {
            if (i >= N) return jh::meta::unexpected(error_code::out_of_bounds);
            return &data[i];
        }

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
} // namespace jh::pod
