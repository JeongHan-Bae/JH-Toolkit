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
 * @file array.h (pods)
 * @author JeongHan-Bae <mastropseudo@gmail.com>
 * @brief Implementation of `jh::pod::array<T, N>`.
 */


#pragma once

#include "pod_like.h"

namespace jh::pod {

    inline constexpr std::uint16_t max_pod_array_bytes = 16 * 1024;
    /// @brief Maximum size of a POD array. This is a compile-time constant.

    /**
     * @brief POD-compatible fixed-size array, similar in shape to `std::array`, but simpler and fully POD.
     *
     * @tparam T Element type. Must satisfy `pod_like<T>`.
     * @tparam N Number of elements. Total memory (sizeof(T) * N) must not exceed 16KB.
     *
     * This structure is designed for:
     * - Raw memory containers (`pod_stack`, `arena`)
     * - In-place value blocks (`placement-new`, `mmap`, `.data` segments)
     * - Zero-allocation, constexpr-safe stack usage
     *
     * ### Design Constraints:
     * - Memory is fully inline and contiguous (`T data[N]`)
     * - Compile-time limited to 16KB for safety and portability
     * - Supports `operator[]`, range-based for-loops, `==` comparison
     * - No bounds checking (by design — zero-overhead)
     *
     * @note This is **not** a drop-in replacement for `std::array`. It has:
     * - No `.at()`, `.fill()`, `.swap()` helpers
     * - No allocator, no bounds safety
     *
     * @warning Do not use this for large arrays or heap-like buffers.
     */
    template<pod_like T, std::uint16_t N>
    requires (sizeof(T) * N <= max_pod_array_bytes)
    struct alignas(alignof(T)) array final{
        T data[N];

        constexpr T &operator[](std::size_t i) noexcept { return data[i]; }
        constexpr const T &operator[](std::size_t i) const noexcept { return data[i]; }

        constexpr T *begin() noexcept { return data; }
        [[nodiscard]] constexpr const T *begin() const noexcept { return data; }
        constexpr T *end() noexcept { return data + N; }
        [[nodiscard]] constexpr const T *end() const noexcept { return data + N; }

        [[nodiscard]] static constexpr std::size_t size() noexcept { return N; }

        constexpr bool operator==(const array &) const = default;
    };
}
