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
 * @file string_view.h (pods)
 * @brief POD-safe <code>string_view</code> with deep comparison and constexpr hashing.
 *
 * This is a lightweight, read-only, non-owning view of immutable strings,
 * conceptually similar to <code>std::string_view</code> but restricted
 * to POD-compatible usage.
 *
 * <h3>Notes:</h3>
 * <ul>
 *   <li>POD layout (<code>const char* + uint64_t</code>)</li>
 *   <li><b>Deep equality</b> via <code>memcmp</code> (not pointer identity)</li>
 *   <li><b>Constexpr/consteval hash</b> supported (unlike <code>bytes_view</code>)</li>
 *   <li>Designed as a safe view for <code>immutable_str</code> and POD containers</li>
 * </ul>
 *
 * @note Unlike <code>bytes_view</code>, its <code>hash()</code> is constexpr-safe,
 *       enabling compile-time usage such as <code>str_template::cstr</code>.
 */


#pragma once

#include <cstdint>
#include <cstring> // for memcmp, memcpy
#include <type_traits>

#include "jh/utils/hash_fn.h"

namespace jh::pod {

    /**
     * @brief Read-only string view with POD layout.
     *
     * Holds a raw pointer and a length (not null-terminated).
     * Provides slicing, comparison, and constexpr hashing, all
     * while remaining fully POD (<code>const char* + uint64_t</code>).
     *
     * <h4>Key differences from std::string_view:</h4>
     * <ul>
     *   <li>Always POD layout (aggregate struct, no constructors)</li>
     *   <li><code>operator==</code> performs <b>deep comparison</b> (memcmp)</li>
     *   <li><code>hash()</code> is <b>constexpr-safe</b> and usable in consteval contexts</li>
     *   <li>No exceptions, no allocator awareness</li>
     * </ul>
     *
     * <h4>Initialization semantics:</h4>
     * <ul>
     *   <li>This type is an aggregate; initialization normally requires both
     *       a <code>const char*</code> and an explicit length.</li>
     *   <li>If only a <code>const char*</code> is provided (without <code>len</code>),
     *       then <code>len</code> defaults to <code>0</code>, producing an empty view.</li>
     *   <li>A special helper <code>from_literal()</code> exists for string literals:
     *       it deduces the array size at compile time and creates a view of length <code>N-1</code>
     *       (excluding the null terminator).</li>
     *   <li>At runtime, you must always provide both pointer and size explicitly
     *       — this class never computes length automatically.</li>
     * </ul>
     *
     * <h4>Usage Model:</h4>
     * <ul>
     *   <li>Typically used as a safe view for <code>immutable_str</code></li>
     *   <li>Can represent string literals or arena-allocated strings</li>
     *   <li>Compile-time hashing supports <code>str_template::cstr</code></li>
     * </ul>
     */
    struct string_view final {
        const char *data;       ///< Pointer to string data (not null-terminated)
        std::uint64_t len;      ///< Number of valid bytes in the view

        using value_type = char;                                 ///< Character type.
        using size_type = std::uint64_t;                         ///< Size type (64-bit).
        using difference_type [[maybe_unused]] = std::ptrdiff_t; ///< Difference type.
        using reference = value_type &;                          ///< Reference to character.
        using const_reference = const value_type &;              ///< Const reference to character.
        using pointer = value_type *;                            ///< Pointer to character.
        using const_pointer = const value_type *;                ///< Const pointer to character.

        /**
         * @brief Construct a <code>string_view</code> from a string literal.
         *
         * @tparam N Size of the string literal including the null terminator.
         * @param lit Reference to the string literal (must be null-terminated).
         * @return A <code>string_view</code> pointing to the literal characters
         *         with <code>size() == N - 1</code>.
         *
         * <h4>Semantics:</h4>
         * <ul>
         *   <li><code>N</code> always counts the null terminator.</li>
         *   <li>The resulting view excludes the null terminator, so length is <code>N - 1</code>.</li>
         *   <li>Empty string literal <code>""</code> is valid (<code>N == 1</code>, view <tt>length = 0</tt>).</li>
         * </ul>
         *
         * @note This overload guarantees constexpr evaluation and can be
         *       used in <code>consteval</code> contexts.
         */
        template<std::size_t N>
        requires (N > 0)
        [[nodiscard]] static constexpr string_view from_literal(const char (&lit)[N]) noexcept {
            return {lit, static_cast<std::uint64_t>(N - 1)};
        }

        /// @brief Index access (no bounds checking).
        constexpr const_reference operator[](const std::uint64_t index) const noexcept {
            return data[index];
        }

        /// @brief Pointer to the beginning of data.
        [[nodiscard]] constexpr const_pointer begin() const noexcept { return data; }

        /**
         * @brief Pointer to the end of data (<code>data + len</code>).
         *
         * @note This is not null-terminated. Use <code>len</code> for bounds.
         */
        [[nodiscard]] constexpr const_pointer end() const noexcept { return data + len; }

        /// @brief View length in bytes.
        [[nodiscard]] constexpr size_type size() const noexcept { return len; }

        /// @brief Whether the view is empty (<code>len == 0</code>).
        [[nodiscard]] constexpr bool empty() const noexcept { return len == 0; }

        /**
         * @brief Compare two views for byte-wise equality.
         *
         * Performs a <strong>deep comparison</strong> of contents using <code>memcmp</code>,
         * rather than checking pointer identity.
         *
         * @param rhs Another <code>string_view</code> to compare.
         * @return <code>true</code> if contents are equal, <code>false</code> otherwise.
         */
        constexpr bool operator==(const string_view &rhs) const noexcept {
            return len == rhs.len && std::memcmp(data, rhs.data, len) == 0;
        }

        /**
         * @brief Returns a substring starting at <code>offset</code>, for <code>length</code> bytes.
         *
         * <ul>
         *   <li>If <code>length == 0</code>, the view extends to the end.</li>
         *   <li>If <code>offset > len</code>, returns an empty view.</li>
         * </ul>
         *
         * @param offset Starting byte index (0-based).
         * @param length Number of bytes (<code>0</code> = sentinel = to end).
         * @return A new <code>string_view</code> into the specified subrange.
         */
        [[nodiscard]] constexpr string_view sub(const std::uint64_t offset,
                                                const std::uint64_t length = 0) const noexcept {
            if (offset > len) return {nullptr, 0}; // out-of-range → empty
            const std::uint64_t remaining = len - offset;
            const std::uint64_t real_len = length == 0 || length > remaining ? remaining : length;
            return {data + offset, real_len};
        }
        /**
         * @brief Lexical comparison (similar to <code>strcmp()</code>).
         * @return <code>&lt;0</code> if <tt>this &lt; rhs</tt>,
         *         <code>0</code> if <tt>equal</tt>,
         *         <code>&gt;0</code> if <tt>this &gt; rhs</tt>.
         */
        [[nodiscard]] constexpr int compare(const string_view &rhs) const noexcept {
            const std::uint64_t min_len = len < rhs.len ? len : rhs.len;

            if (std::is_constant_evaluated()) {
                // constexpr path: manual loop
                for (std::uint64_t i = 0; i < min_len; i++) {
                    if (data[i] < rhs.data[i]) return -1;
                    if (data[i] > rhs.data[i]) return 1;
                }
            } else {
                // runtime path: use memcmp
                if (int cmp = std::memcmp(data, rhs.data, min_len); cmp != 0) {
                    return cmp;
                }
            }

            return static_cast<int>(len) - static_cast<int>(rhs.len);
        }

        /// @brief Check whether this view starts with the given <code>prefix</code>.
        [[nodiscard]] bool starts_with(const string_view &prefix) const noexcept {
            return len >= prefix.len && std::memcmp(data, prefix.data, prefix.len) == 0;
        }

        /// @brief Check whether this view ends with the given <code>suffix</code>.
        [[nodiscard]] bool ends_with(const string_view &suffix) const noexcept {
            return len >= suffix.len &&
                   std::memcmp(data + (len - suffix.len), suffix.data, suffix.len) == 0;
        }

        /**
         * @brief Returns the index of the first occurrence of a character.
         *
         * @param ch Target character to search for.
         * @return Offset index if found, or <code>-1</code> (as <code>uint64_t</code>) if not found.
         */
        [[nodiscard]] std::uint64_t find(const char ch) const noexcept {
            for (std::uint64_t i = 0; i < len; ++i)
                if (data[i] == ch) return i;
            return static_cast<std::uint64_t>(-1); // not found
        }

        /**
         * @brief Hash the view content using a selectable non-cryptographic algorithm.
         *
         * Provides stable 64-bit hashing over the view contents.
         *
         * @param hash_method Algorithm to use for hashing (default: <code>fnv1a64</code>).
         * @return 64-bit hash of the view data, or <code>-1</code> if <code>data == nullptr</code>.
         *
         * @note <ul>
         *   <li>This is <strong>not cryptographic</strong>; do not use it for security-sensitive logic.</li>
         *   <li>If <code>data</code> is null, the return value is <code>-1</code> (sentinel).</li>
         *   <li>Hashing is based only on contents and length, not on pointer identity.</li>
         *   <li>
         *     Unlike <code>bytes_view::hash</code>, this function is <strong>valid in consteval contexts</strong>.
         *     <ul>
         *       <li><code>bytes_view</code> relies on <code>reinterpret_cast</code>, so it cannot be evaluated at compile time.</li>
         *       <li><code>string_view</code> operates directly on characters, so compile-time hashing of string literals
         *           is both <strong>well-defined</strong> and <strong>semantically meaningful</strong>.</li>
         *       <li>This design enables features such as <code>str_template::cstr</code> to compute hashes fully at compile time.</li>
         *     </ul></li>
         * </ul>
         */
        [[nodiscard]] constexpr std::uint64_t
        hash(jh::utils::hash_fn::c_hash hash_method = jh::utils::hash_fn::c_hash::fnv1a64) const noexcept {
            if (!data) return static_cast<std::uint64_t>(-1);
            return utils::hash_fn::hash(hash_method, data, len);
        }

        /**
         * @brief Copy the view into a C-style null-terminated buffer.
         *
         * @warning This is not POD-safe. Intended for debugging or interop only.
         *
         * @param buffer Output character buffer.
         * @param max_len Maximum bytes to write (including null terminator).
         */
        void copy_to(char *buffer, const std::uint64_t max_len) const noexcept {
            const std::uint64_t n = len < max_len - 1 ? len : max_len - 1;
            std::memcpy(buffer, data, n);
            buffer[n] = '\0';
        }
    };
} // namespace jh::pod
