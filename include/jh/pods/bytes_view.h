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
 * @file bytes_view.h (pods)
 * @brief POD-safe minimal byte-range view with reinterpreting and cloning utilities.
 *
 * This header defines <code>jh::pod::bytes_view</code> — a low-level, read-only,
 * non-owning abstraction over raw memory regions. It enables safe reinterpretation
 * and controlled cloning of memory blocks into POD-compatible types.
 *
 * <h3>Design Goals:</h3>
 * <ul>
 *   <li>Fully POD (<code>const std::byte*</code> + <code>uint64_t</code>)</li>
 *   <li>No ownership, no destructor, no STL containers</li>
 *   <li>Support for reinterpretation (<code>at</code>, <code>fetch</code>)</li>
 *   <li>Stack-safe and heap-safe cloning (<code>clone</code>, <code>clone_new</code>)</li>
 *   <li>Works seamlessly with <code>pod_like</code> and <code>trivial_bytes</code> types</li>
 * </ul>
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
     * <h4>Requirements:</h4>
     * <ul>
     *   <li>Standard layout (predictable field order and layout)</li>
     *   <li>Trivially constructible (safe to create via memcpy)</li>
     * </ul>
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
     * <h4>Clone Safety Model</h4>
     * The <code>clone&lt;T&gt;()</code> method is only available for <code>pod_like</code> types.
     * This ensures:
     * <ul>
     *   <li>Bitwise copies are semantically valid (no heap ownership, no reference count)</li>
     *   <li>Cloned objects do not require custom destructors or resource management</li>
     *   <li>Structures reconstructed from raw bytes behave identically to their originals</li>
     * </ul>
     *
     * <h4>Formal constraints</h4>
     * In essence, this maps to:
     * <ul>
     *   <li><code>std::is_standard_layout_v&lt;T&gt;</code></li>
     *   <li><code>std::is_trivially_constructible_v&lt;T&gt;</code></li>
     *   <li><code>std::is_trivially_copyable_v&lt;T&gt;</code></li>
     *   <li><code>std::is_trivially_destructible_v&lt;T&gt;</code></li>
     * </ul>
     *
     * Which is exactly what <code>jh::pod::pod_like&lt;T&gt;</code> ensures.
     *
     * @note The functions <code>from</code>, <code>at</code>, <code>fetch</code>, and <code>clone</code>
     *       are declared <code>constexpr</code> primarily to enable aggressive compiler optimizations.
     *       However, because they internally use <code>reinterpret_cast</code>, they cannot be
     *       evaluated in <code>consteval</code> contexts. If you need to copy POD objects or construct
     *       one POD from another at compile time, write the copy or constructor manually instead of
     *       relying on these helpers.
     */
    struct bytes_view final {
        const std::byte *data;  ///< Pointer to the start of the byte range
        std::uint64_t len;      ///< Number of bytes in the view

        using value_type = std::byte;                                ///< Value type alias.
        using size_type = std::uint64_t;                             ///< Size type alias (64-bit).
        using difference_type [[maybe_unused]] = std::ptrdiff_t;     ///< Difference type alias.
        using reference = value_type &;                              ///< Reference type.
        using const_reference [[maybe_unused]] = const value_type &; ///< Const reference type.
        using pointer = value_type *;                                ///< Pointer type.
        using const_pointer [[maybe_unused]] = const value_type *;   ///< Const pointer type.

        /**
         * @brief Construct a <code>bytes_view</code> from a trivially laid-out object.
         *
         * This function provides a safer alternative to manual <code>reinterpret_cast</code>
         * when creating a view over raw memory. It guarantees that only objects satisfying
         * <code>trivial_bytes</code> (standard layout, trivially constructible) are accepted,
         * preventing misuse with non-POD types.
         *
         * @tparam T Must satisfy <code>trivial_bytes</code>.
         * @param obj Source object (not copied).
         * @return A <code>bytes_view</code> representing the raw bytes of <code>obj</code>.
         */
        template<trivial_bytes T>
        static constexpr bytes_view from(const T &obj) noexcept {
            return {
                    reinterpret_cast<const std::byte *>(std::addressof(obj)),
                    sizeof(T)
            };
        }

        /**
         * @brief Construct a <code>bytes_view</code> from a contiguous array of objects.
         *
         * Like the single-object overload, this provides a safer alternative to manual
         * <code>reinterpret_cast</code> by requiring <code>trivial_bytes</code> elements.
         * It enables treating a typed array or buffer (e.g., from a C-style array,
         * span, or mmap'd structure) as a flat byte view.
         *
         * @tparam T Must satisfy <code>trivial_bytes</code>.
         * @param arr Pointer to the first element (may be <code>nullptr</code> if <code>size == 0</code>).
         * @param size Number of elements (of type <code>T</code>, not bytes).
         * @return A <code>bytes_view</code> covering <code>sizeof(T) * size</code> bytes starting at <code>arr</code>.
         *
         * @note The caller must ensure that <code>arr</code> points to a valid contiguous block
         *       of at least <code>size</code> elements.
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
         * This is equivalent to the <code>len</code> field and reflects the total size
         * (in bytes) of the memory region being viewed. The return type is 64-bit
         * (<code>std::uint64_t</code>), allowing safe representation of large memory
         * regions.
         *
         * @return The length of the view in bytes.
         */
        [[nodiscard]] constexpr size_type size() const noexcept { return len; }

        /**
         * @brief Reinterpret a subregion of the view as a reference to <code>T</code>.
         *
         * This function provides POD-safe reinterpretation of the underlying bytes.
         * The type <code>T</code> must satisfy <code>trivial_bytes</code>, ensuring
         * the result is well-defined for POD-like memory.
         *
         * Internally it uses <code>std::launder</code> to avoid undefined behavior
         * when reinterpreting object representations. The operation is marked
         * <code>noexcept</code> and is guaranteed not to throw.
         *
         * @tparam T Must satisfy <code>trivial_bytes</code>.
         * @param offset Offset in bytes into the view (default 0).
         * @return Reference to the reinterpreted value at the specified offset.
         *
         * @note The caller must ensure <code>offset + sizeof(T) &lt;= size()</code>.
         *       No bounds checking is performed; out-of-bounds access remains the
         *       caller's responsibility.
         */
        template<trivial_bytes T>
        constexpr const T &at(const std::uint64_t offset = 0) const noexcept {
            return *std::launder(reinterpret_cast<const T *>(data + offset));
        }

        /**
         * @brief Safely fetch a pointer to a <code>T</code> from the view.
         *
         * This is the bounds-checked counterpart to <code>at</code>. It reinterprets
         * a subregion of the view as <code>T</code> if the memory range
         * <code>[offset, offset + sizeof(T))</code> is fully contained within the view.
         * If the range would exceed <code>size()</code>, it returns <code>nullptr</code>
         * instead of producing undefined behavior.
         *
         * The type <code>T</code> must satisfy <code>trivial_bytes</code>, ensuring POD-safe
         * reinterpretation semantics. Internally it uses <code>std::launder</code> to avoid
         * undefined behavior when accessing object representations. The operation is marked
         * <code>noexcept</code>.
         *
         * @tparam T Must satisfy <code>trivial_bytes</code>.
         * @param offset Offset in bytes into the view (default 0).
         * @return Pointer to the reinterpreted value, or <code>nullptr</code> if out of bounds.
         */
        template<trivial_bytes T>
        constexpr const T *fetch(const std::uint64_t offset = 0) const noexcept {
            return offset + sizeof(T) <= len
                   ? std::launder(reinterpret_cast<const T *>(data + offset))
                   : nullptr;
        }

        /**
         * @brief Clone the entire view contents into a value of type <code>T</code> on the stack.
         *
         * This is the safest way to materialize a <code>pod_like</code> object from a
         * <code>bytes_view</code>. It requires that:
         * <ul>
         *   <li><code>len == sizeof(T)</code> — the view must exactly match the size of <code>T</code>.</li>
         *   <li><code>T</code> must satisfy <code>pod_like</code> (standard layout, trivially constructible,
         *       trivially copyable, trivially destructible).</li>
         * </ul>
         *
         * If the size check fails, a default-initialized <code>T{}</code> (zero-initialized POD object)
         * is returned. Otherwise, this function copies the raw bytes into a local object and returns it
         * by value, effectively performing a POD copy.
         *
         * @tparam T Must satisfy <code>cv_free_pod_like</code>.
         *           Using <code>const</code> or <code>volatile</code> qualified types is disallowed,
         *           as this function constructs a writable value of type <code>T</code>.
         * @return A value reconstructed from the view if sizes match, or a default-initialized
         *         <code>T{}</code> if size mismatch.
         */
        template<cv_free_pod_like T>
        [[nodiscard]] constexpr T clone() const noexcept {
            if (len != sizeof(T)) return T{};
            return at<T>(0);
        }

        /**
         * @brief Compute a deterministic 64-bit hash of the view contents.
         *
         * This function computes a stable, non-cryptographic hash value from the raw
         * bytes in the view. The result depends only on the byte sequence and its length,
         * not on any type-level semantics. It is suitable for version checking,
         * cache keys, or equality grouping.
         *
         * @param hash_method Hash algorithm to use (default: <code>fnv1a64</code>).
         * @return 64-bit hash of the byte content, or <code>0xFFFFFFFFFFFFFFFF</code> if <code>data == nullptr</code>.
         *
         * @note This hash is not cryptographically secure and must not be used for security-sensitive purposes.
         * @note If <code>data == nullptr</code>, the return value is <code>0xFFFFFFFFFFFFFFFF</code> (sentinel).
         * @note Although declared <code>constexpr</code>, this function cannot be used in <code>consteval</code>
         *       contexts due to its reliance on pointer reinterpretation. It is intended for runtime use.
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

        /**
         * @brief Compare two views for byte-wise equality.
         *
         * As a view type, equality is defined strictly in terms of the underlying
         * byte sequence (deep comparison), not pointer identity or struct field comparison.
         *
         * @param rhs The other <code>bytes_view</code> to compare with.
         * @return <code>true</code> if both views have the same length and identical byte contents,
         *         <code>false</code> otherwise.
         */
        constexpr bool operator==(const bytes_view &rhs) const noexcept {
            return len == rhs.len && std::memcmp(data, rhs.data, len) == 0;
        }
    };
} // namespace jh::pod
