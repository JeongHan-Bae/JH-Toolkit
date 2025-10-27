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
 * @file span.h (pods)
 * @brief POD-safe minimal span for contiguous memory.
 *
 * <h3>Design Goals:</h3>
 * <ul>
 *   <li>Fully POD (<code>T* + uint64_t</code>)</li>
 *   <li>No dynamic allocation, no STL dependencies</li>
 *   <li>Iteration, slicing, and indexing support</li>
 *   <li>Suitable for arena allocators, mmap, and raw containers</li>
 * </ul>
 *
 * @note Unlike <code>std::span</code>, this type is limited to POD-compatible
 *       contiguous memory. It does not support iterator ranges, polymorphic
 *       containers, or allocator-aware semantics.
 * @note Lifetime of underlying memory must be managed externally.
 * @note Functions rely on <code>reinterpret_cast</code> and therefore cannot
 *       be used in <code>consteval</code> contexts.
 */

#pragma once

#include <cstdint>   // for uint64_t
#include <concepts>  // NOLINT for std::convertible_to<>
#include <type_traits>

#include "jh/pods/pod_like.h"

namespace jh::pod {
    namespace detail {
        /**
         * @brief Concept for linear containers exposing <code>.data()</code> and <code>.size()</code>.
         *
         * Requires that:
         * <ul>
         *   <li><code>c.data()</code> is convertible to a pointer type</li>
         *   <li><code>c.size()</code> is convertible to <code>uint64_t</code></li>
         * </ul>
         */
        template<typename C, typename T = std::remove_pointer_t<decltype(std::declval<C>().data())> >
        concept LinearContainer =
                requires(const C &c)
                {
                    { c.data() } -> std::convertible_to<const T *>;
                    { c.size() } -> std::convertible_to<std::uint64_t>;
                };
    }

    /**
     * @brief Non-owning typed view over a contiguous memory block.
     *
     * Behaves like a stripped-down <code>std::span</code>, but remains fully POD
     * (<code>T* + uint64_t</code>).
     *
     * <h4>Differences from std::span:</h4>
     * <ul>
     *   <li>No bounds-checked <code>.at()</code></li>
     *   <li>No dynamic extent; always runtime-sized</li>
     *   <li>No interop with iterator-only containers</li>
     * </ul>
     *
     * <h4>Usage Model:</h4>
     * <ul>
     *   <li>Provides indexing (<code>operator[]</code>)</li>
     *   <li>Range iteration via <code>begin()</code>/<code>end()</code></li>
     *   <li>Slicing via <code>sub()</code>, <code>first()</code>, <code>last()</code></li>
     * </ul>
     *
     * @tparam T Element type. Must satisfy <code>pod_like</code>.
     */
    template<pod_like T>
    struct span final {
        T *data;           ///< @brief Pointer to the first element.
        std::uint64_t len; ///< @brief Number of elements.

        using element_type = T;                                   ///< @brief Element type (alias of <code>T</code>).
        using value_type = std::remove_cv_t<T>;                   ///< @brief Value type without const/volatile.
        using size_type = std::uint64_t;                          ///< @brief Size type (64-bit).
        using difference_type [[maybe_unused]] = std::ptrdiff_t;  ///< @brief Signed difference type.
        using reference = value_type &;                           ///< @brief Reference to element.
        using const_reference = const value_type &;               ///< @brief Const reference to element.
        using pointer = value_type *;                             ///< @brief Pointer to element.
        using const_pointer = const value_type *;                 ///< @brief Const pointer to element.

        /// @brief Access an element by index (no bounds check).
        constexpr const_reference operator[](std::uint64_t index) const noexcept {
            return data[index];
        }

        /// @brief Pointer to the first element.
        [[nodiscard]] constexpr const_pointer begin() const noexcept { return data; }

        /// @brief Pointer to one-past-the-end.
        [[nodiscard]] constexpr const_pointer end() const noexcept { return data + len; }

        /// @brief Number of elements in view.
        [[nodiscard]] constexpr size_type size() const noexcept { return len; }

        /// @brief Whether the view is empty.
        [[nodiscard]] constexpr bool empty() const noexcept { return len == 0; }

        /**
         * @brief Creates a sub-span from <code>offset</code>, with optional <code>count</code> elements.
         *
         * If <code>count == 0</code> (default), the view extends to end.
         * If <code>offset &gt; len</code>, returns empty span.
         */
        [[nodiscard]] constexpr span sub(const std::uint64_t offset,
                                         const std::uint64_t count = 0) const noexcept {
            if (offset > len) return {nullptr, 0};
            const std::uint64_t remaining = len - offset;
            const std::uint64_t real_len = (count == 0 || count > remaining) ? remaining : count;
            return {data + offset, real_len};
        }

        /// @brief Returns the first <code>count</code> elements as a new span.
        [[nodiscard]] constexpr span first(const std::uint64_t count) const noexcept {
            if (!count) return {nullptr, 0};
            return {data, (len > count ? count : len)};
        }

        /// @brief Returns the last <code>count</code> elements as a new span.
        [[nodiscard]] constexpr span last(const std::uint64_t count) const noexcept {
            if (!count) return {nullptr, 0};
            if (count >= len) return *this;
            return {data + len - count, count};
        }

        /**
         * @brief Equality comparison between two spans.
         *
         * Two spans are considered equal if they reference the <b>same sequence object</b>
         * (i.e. identical pointer and identical length).
         * This does <b>not</b> compare the element values.
         *
         * @note Implemented with <code>= default</code>, which checks <code>data</code> and <code>len</code>.
         * If you need value-wise comparison, use algorithms such as
         * <code>std::equal(lhs.begin(), lhs.end(), rhs.begin())</code>.
         */
        constexpr bool operator==(const span &rhs) const = default;
    };

    /// @brief Create span from a raw array (<code>T[N]</code>).
    template<typename T, std::uint64_t N>
    [[nodiscard]] constexpr span<T> to_span(T (&arr)[N]) noexcept {
        return {arr, static_cast<std::uint64_t>(N)};
    }

    /// @brief Create span from a const raw array (<code>const T[N]</code>).
    template<typename T, std::uint64_t N>
    [[nodiscard]] constexpr span<const T> to_span(const T (&arr)[N]) noexcept {
        return {arr, static_cast<std::uint64_t>(N)};
    }

    /// @brief Create span from an object with <code>.data()</code> and <code>.size()</code>.
    template<detail::LinearContainer C>
    [[nodiscard]] constexpr auto to_span(C &c) noexcept
        -> span<std::remove_pointer_t<decltype(c.data())> > {
        return {c.data(), static_cast<std::uint64_t>(c.size())};
    }

    /// @brief Const overload for containers.
    template<detail::LinearContainer C>
    [[nodiscard]] constexpr auto to_span(const C &c) noexcept
        -> span<const std::remove_pointer_t<decltype(c.data())>> {
        return {c.data(), static_cast<std::uint64_t>(c.size())};
    }
} // namespace jh::pod
