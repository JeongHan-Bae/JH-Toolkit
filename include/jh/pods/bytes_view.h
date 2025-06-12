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
 * @file bytes_view.h (pods)
 * @author JeongHan-Bae <mastropseudo@gmail.com>
 * @brief POD-safe minimal byte-range view with reinterpreting and cloning utilities.
 *
 * This header defines `jh::pod::bytes_view` — a low-level, read-only, non-owning
 * abstraction over raw memory regions. It enables safe reinterpretation and controlled
 * cloning of memory blocks into POD-compatible types.
 *
 * ## Design Goals:
 * - Fully POD (`const std::byte* + uint64_t`)
 * - No ownership, no destructor, no STL containers
 * - Support for reinterpretation (`at`, `fetch`)
 * - Stack-safe and heap-safe cloning (`clone`, `clone_new`)
 * - Works seamlessly with `pod_like` and `trivial_bytes` types
 *
 * @note This type assumes the data lifetime is externally guaranteed.
 *       It is ideal for parsing binary payloads, memory-mapped blobs,
 *       protocol headers, or arena-based serialization systems.
 */


#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>     // NOLINT for std::launder
#include <cstring> // for memcmp, memcpy

#include "pod_like.h"
#include "jh/utils/hash_fn.h"

namespace jh::pod {

    /**
     * @brief Concept for trivially layout-compatible types (POD-compatible memory view).
     *
     * Requires:
     * - Standard layout (predictable field order and layout)
     * - Trivially constructible (safe to create via memcpy)
     *
     * Suitable for reinterpretation, raw memory casting, and heap cloning.
     */
    template<typename T>
    concept trivial_bytes = requires(T)
    {
        std::is_standard_layout_v<T>;
        std::is_trivially_constructible_v<T>;
    };

    /**
     * @brief A read-only view over a block of raw bytes.
     *
     * This struct holds a pointer + length representing a memory region.
     * It supports reinterpretation as POD types, safe view extraction,
     * and object-level clone utilities.
     *
     * ## Clone Safety Model
     * The `clone<T>()` method is only available for `pod_like` types.
     * This ensures:
     * - Bitwise copies are semantically valid (no heap ownership, no reference count)
     * - Cloned objects do not require custom destructors or resource management
     * - Structures reconstructed from raw bytes behave identically to their originals
     *
     * In essence, this constraint maps to:
     *     - `std::is_standard_layout_v<T>`
     *     - `std::is_trivially_constructible_v<T>`
     *     - `std::is_trivially_copyable_v<T>`
     *     - `std::is_trivially_destructible_v<T>`
     *
     * Which is exactly what `jh::pod::pod_like<T>` ensures.
     */
    struct bytes_view final {
        const std::byte *data;  ///< Pointer to the start of the byte range
        std::uint64_t len;      ///< Number of bytes in the view

        using value_type = std::byte;
        using size_type [[maybe_unused]] = std::uint64_t;
        using difference_type [[maybe_unused]] = std::ptrdiff_t;
        using reference [[maybe_unused]] = value_type &;
        using const_reference [[maybe_unused]] = const value_type &;
        using pointer [[maybe_unused]] = value_type *;
        using const_pointer [[maybe_unused]] = const value_type *;

        /**
         * @brief Construct a view from any trivially laid-out object.
         *
         * @tparam T Must satisfy `trivial_bytes` (standard layout, no constructor)
         * @param obj Source object (not copied)
         * @return View into obj memory
         */
        template<trivial_bytes T>
        static constexpr bytes_view from(const T &obj) noexcept {
            return {
                    reinterpret_cast<const std::byte *>(std::addressof(obj)),
                    sizeof(T)
            };
        }

        /**
         * @brief Construct a view from a contiguous array of `T`.
         *
         * This overload enables viewing the raw memory of a typed array or buffer
         * (e.g., from a C-style array, a span, or mmap'd structure) as a flat byte view.
         *
         * @tparam T Must satisfy `trivial_bytes`
         * @param arr Pointer to the first element (might be null if `size == 0`)
         * @param size Number of elements (of type `T`), not bytes
         * @return A `bytes_view` covering `sizeof(T) * size` bytes starting at `arr`
         *
         * @note The caller must ensure that `arr` points to a valid contiguous block
         *       of at least `size` elements.
         * @note The `size` refers to element count, not byte count.
         */
        template<trivial_bytes T>
        static constexpr bytes_view from(const T *arr, const std::uint64_t size) noexcept {
            return {
                    reinterpret_cast<const std::byte *>(arr),
                    sizeof(T) * size
            };
        }

        /**
         * @brief Returns the number of bytes in the view.
         *
         * This reflects the total size (in bytes) of the memory region being viewed.
         * Equivalent to the `len` field, and useful for compatibility with STL-like
         * interfaces or when treating the view as a byte-range container.
         *
         * @return The length of the view in bytes (as `uint16_t`).
         */
        [[nodiscard]] constexpr size_type size() const noexcept { return len; }

        /**
         * @brief Reinterpret the view at offset as a reference to `T`.
         *
         * No bounds checking; unsafe if offset is invalid.
         *
         * @tparam T Must satisfy `trivial_bytes`
         * @param offset Offset into the view
         * @return Reference to reinterpreted value
         */
        template<trivial_bytes T>
        constexpr const T &at(const std::uint64_t offset = 0) const noexcept {
            return *std::launder(reinterpret_cast<const T *>(data + offset));
        }

        /**
         * @brief Safely fetch a pointer to a `T` from the view.
         *
         * Performs bounds-check against `len`. Returns nullptr on failure.
         *
         * @tparam T Must satisfy `trivial_bytes`
         * @param offset Offset into the view
         * @return Pointer to reinterpret value, or nullptr if out of bounds
         */
        template<trivial_bytes T>
        constexpr const T *fetch(const std::uint64_t offset = 0) const noexcept {
            return offset + sizeof(T) <= len
                   ? std::launder(reinterpret_cast<const T *>(data + offset))
                   : nullptr;
        }

        /**
         * @brief Clone the view contents into a value of type `T` on the stack.
         *
         * Requires `T` to be a `pod_like` type:
         * - Standard layout (well-defined memory layout)
         * - Trivially constructible (no custom constructor)
         * - Trivially copyable (no custom copy behavior)
         * - Trivially destructible (no destructor side effects)
         *
         * If `len != sizeof(T)`, returns default-initialized `T{}`.
         * Otherwise, copies the raw bytes into a local object and returns by value.
         *
         * @tparam T Must satisfy `pod_like`
         * @return A new value reconstructed from view data, or default-constructed on mismatch
         */
        template<pod_like T>
        [[nodiscard]] constexpr T clone() const noexcept {
            if (len != sizeof(T)) return T{};
            return at<T>(0);
        }

        /**
         * @brief Hash the byte view content using a selectable non-cryptographic algorithm.
         *
         * Computes a stable 64-bit hash from the byte contents of this view, using
         * one of the selectable hash strategies in `jh::utils::hash_fn::c_hash`.
         *
         * @param hash_method Hash algorithm to use (default: FNV-1a 64-bit).
         * @return 64-bit hash of the byte content, or `-1` if `data == nullptr`.
         *
         * @note This hash is not cryptographically secure.
         * @note Returns `-1` as a sentinel if the view has null data.
         * @note The result only depends on byte content and length, not on any type semantics.
         */
        [[nodiscard]] constexpr std::uint64_t
        hash(jh::utils::hash_fn::c_hash hash_method = jh::utils::hash_fn::c_hash::fnv1a64) const noexcept {
            if (!data) return static_cast<std::uint64_t>(-1);
            return jh::utils::hash_fn::hash(
                    hash_method,
                    reinterpret_cast<const char *>(data),
                    len
            );
        }

        /// @brief Compare two views for byte-wise equality.
        constexpr bool operator==(const bytes_view &rhs) const noexcept {
            return len == rhs.len && std::memcmp(data, rhs.data, len) == 0;
        }
    };
} // namespace jh::pod
