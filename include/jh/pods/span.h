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
 * @file span.h (pods)
 * @author JeongHan-Bae <mastropseudo@gmail.com>
 * @brief POD-safe minimal span implementation for viewing contiguous memory blocks.
 *
 * This header defines `jh::pod::span<T>` — a non-owning view into a typed memory range,
 * designed for high-performance, POD-only systems such as `pod_vector`, `arena`, or `mmap`.
 *
 * ## Design Goals:
 * - Fully POD (`T* + uint64_t`)
 * - No dynamic allocation, no STL
 * - Iteration + slicing (`begin`, `end`, `sub`, `operator[]`)
 * - Suitable for view-only use cases in high-performance subsystems
 *
 * @note This type assumes lifetime of the memory is managed externally.
 *       It should be treated as read/write *view* into pre-allocated or static memory.
 *
 *       This span is strictly intended for viewing contiguous, array-like memory.
 *       If your data represents a packed or heterogeneous layout (e.g., binary blob,
 *       protocol packet, or non-uniform struct), use `bytes_view` instead.
 *
 *       Additionally, this span only supports direct memory access — containers
 *       must expose `.data()` convertible to `T*` and `.size()` convertible to `uint64_t`.
 *       Iterator-based ranges or virtualized containers are not supported by design.
 */

#pragma once

#include <cstdint>   // for uint64_t
#include <concepts>  // NOLINT for std::convertible_to<>

#include "pod_like.h"

namespace jh::pod {
    namespace detail {
        template<typename C, typename T = std::remove_pointer_t<decltype(std::declval<C>().data())> >
        concept LinearContainer =
                requires(const C &c)
                {
                    { c.data() } -> std::convertible_to<const T *>;
                    { c.size() } -> std::convertible_to<std::uint64_t>;
                };
    }


    /**
     * @brief Typed, non-owning view over a contiguous memory block.
     *
     * `span<T>` behaves like a stripped-down `std::span<T>`, but remains fully POD.
     * This is ideal for viewing regions in arena allocators, mmaps, or in-place structs.
     */
    template<pod_like T>
    struct span final {
        T *data;           ///< Pointer to the first element
        std::uint64_t len; ///< Number of elements

        /// @brief Access an element by index (no bounds check).
        constexpr T &operator[](std::uint64_t index) const noexcept {
            return data[index];
        }

        /// @brief Pointer to the first element.
        [[nodiscard]] constexpr T *begin() const noexcept { return data; }

        /// @brief Pointer to one-past-the-end.
        [[nodiscard]] constexpr T *end() const noexcept { return data + len; }

        /// @brief Number of elements in view.
        [[nodiscard]] constexpr std::uint64_t size() const noexcept { return len; }

        /// @brief Whether the view is empty.
        [[nodiscard]] constexpr bool empty() const noexcept { return len == 0; }

        /**
         * @brief Creates a sub-span from `offset`, with optional `count` elements.
         *
         * If count == 0 (default), the view extends to end.
         * If offset > len, returns empty span.
         */
        [[nodiscard]] constexpr span sub(const std::uint64_t offset,
                                         const std::uint64_t count = 0) const noexcept {
            if (offset > len) return {nullptr, 0};
            const std::uint64_t remaining = len - offset;
            const std::uint64_t real_len = (count == 0 || count > remaining) ? remaining : count;
            return {data + offset, real_len};
        }

        /**
         * @brief Returns the first `count` elements as a new span.
         * If `count >= len`, returns a full span.
         */
        [[nodiscard]] constexpr span first(const std::uint64_t count) const noexcept {
            if (!count) return {nullptr, 0};
            return {data, (len > count ? count : len)};
        }

        /**
         * @brief Returns the last `count` elements as a new span.
         * If `count >= len`, returns a full span.
         */
        [[nodiscard]] constexpr span last(const std::uint64_t count) const noexcept {
            if (!count) return {nullptr, 0};
            if (count >= len) return *this;
            return {data + len - count, count};
        }

        constexpr bool operator==(const span &rhs) const = default;
    };

    /// @brief Create span from a raw array (T[N]).
    template<typename T, std::size_t N>
    [[nodiscard]] constexpr span<T> to_span(T (&arr)[N]) noexcept {
        return {arr, static_cast<std::uint64_t>(N)};
    }

    /// @brief Create span from a const raw array (const T[N]).
    template<typename T, std::size_t N>
    [[nodiscard]] constexpr span<const T> to_span(const T (&arr)[N]) noexcept {
        return {arr, static_cast<std::uint64_t>(N)};
    }

    /// @brief Create span from an object with .data() and .size() methods.
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

    /// @brief Compile-time POD conformance
    static_assert(pod_like<span<int> >);
} // namespace jh::pod
