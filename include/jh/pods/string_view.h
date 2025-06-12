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
 * @file string_view.h (pods)
 * @author JeongHan-Bae <mastropseudo@gmail.com>
 * @brief POD-safe minimal string_view implementation with hashing and view utilities.
 *
 * This header defines a strict `jh::pod::string_view` — a lightweight, read-only, non-owning string
 * abstraction for memory-safe viewing of immutable strings in POD-only containers like `pod_stack`,
 * `data_sink`, or `arena`.
 *
 * ## Design Goals:
 * - Fully POD (`const char* + uint64_t`)
 * - No ownership, no heap, no exception
 * - STL-compatible behaviors (`==`, `sub`, `find`, `starts_with`, etc.)
 * - Compile-time `hash()` for enum-like dispatch or ID tagging
 * - Suitable for parsing, token buckets, AST modeling, serialization
 *
 * @note This type should only be used when **lifetime and immutability of the data is externally guaranteed.**
 *       It is especially useful when paired with `immutable_str`, `mmap`, or `arena`-based allocations.
 */

#pragma once

#include <cstdint>
#include <cstring> // for memcmp, memcpy

#include "../utils/hash_fn.h"

namespace jh::pod {

    /**
     * @brief Read-only, immutable string view for POD-only environments.
     *
     * `string_view` holds a raw pointer and length (not null-terminated),
     * and provides basic comparison, slicing, hashing, and utility access
     * — all without breaking POD rules.
     */
    struct string_view final {
        const char *data;       ///< Pointer to string data (not null-terminated)
        std::uint64_t len;      ///< Number of valid bytes in the view

        using value_type = char;
        using size_type [[maybe_unused]] = std::uint64_t;
        using difference_type [[maybe_unused]] = std::ptrdiff_t;
        using reference [[maybe_unused]] = value_type &;
        using const_reference [[maybe_unused]] = const value_type &;
        using pointer [[maybe_unused]] = value_type *;
        using const_pointer [[maybe_unused]] = const value_type *;

        // === Iteration & Size ===

        /// @brief Index access (no bounds checking).
        constexpr const_reference operator[](const std::uint64_t index) const noexcept {
            return data[index];
        }

        /// @brief Pointer to the beginning of data.
        [[nodiscard]] constexpr const_pointer begin() const noexcept { return data; }

        /** @brief Pointer to end of data (`data + len`).
         *
         * @note This is not null-terminated. Use `len` for bounds.
         */
        [[nodiscard]] constexpr const_pointer end() const noexcept { return data + len; }

        /// @brief View length in bytes.
        [[nodiscard]] constexpr size_type size() const noexcept { return len; }

        /// @brief Whether the view is empty (`len == 0`).
        [[nodiscard]] constexpr bool empty() const noexcept { return len == 0; }

        /// @brief Compare two views for byte-wise equality.
        constexpr bool operator==(const string_view &rhs) const noexcept {
            return len == rhs.len && std::memcmp(data, rhs.data, len) == 0;
        }

        /**
         * @brief Returns a substring starting at `offset`, for `length` bytes.
         *
         * If `length == 0`, returns a view to the end of the string.
         * If `offset > len`, returns empty view (out-of-range).
         *
         * @param offset Starting byte index (0-based).
         * @param length Number of bytes (0 = sentinel = to end).
         * @return A new `string_view` into the specified subrange.
         */
        [[nodiscard]] constexpr string_view sub(const std::uint64_t offset,
                                                const std::uint64_t length = 0) const noexcept {
            if (offset > len) return {nullptr, 0}; // out-of-range → empty
            const std::uint64_t remaining = len - offset;
            const std::uint64_t real_len = length == 0 || length > remaining ? remaining : length;
            return {data + offset, real_len};
        }

        // === ASCII Comparison ===

        /**
         * @brief Lexical comparison (like `strcmp()`).
         * @return <0 if this < rhs, 0 if equal, >0 if this > rhs
         */
        [[nodiscard]] int compare(const string_view &rhs) const noexcept {
            const std::uint64_t min_len = len < rhs.len ? len : rhs.len;
            if (const int cmp = std::memcmp(data, rhs.data, min_len); cmp != 0) return cmp;
            return static_cast<int>(len) - static_cast<int>(rhs.len);
        }


        /// @brief Whether this view starts with the given `prefix`.
        [[nodiscard]] bool starts_with(const string_view &prefix) const noexcept {
            return len >= prefix.len && std::memcmp(data, prefix.data, prefix.len) == 0;
        }

        /// @brief Whether this view ends with the given `suffix`.
        [[nodiscard]] bool ends_with(const string_view &suffix) const noexcept {
            return len >= suffix.len &&
                   std::memcmp(data + (len - suffix.len), suffix.data, suffix.len) == 0;
        }

        /**
         * @brief Returns index of first occurrence of character.
         * @param ch Target character to locate.
         * @return Offset index if found; `-1` (as uint64_t) if not found.
         */
        [[nodiscard]] std::uint64_t find(const char ch) const noexcept {
            for (std::uint64_t i = 0; i < len; ++i)
                if (data[i] == ch) return i;
            return static_cast<std::uint64_t>(-1); // not found
        }

        /**
         * @brief Hash the byte view content using a selectable non-cryptographic algorithm.
         *
         * Provides stable 64-bit hashing over the view contents, using a selectable
         * algorithm from `jh::utils::hash_fn::c_hash`. Suitable for use in hashing,
         * lookup tables, unique identifiers, etc.
         *
         * @param hash_method Algorithm to use for hashing (default: FNV-1a 64-bit).
         * @return 64-bit hash of the view data, or `-1` if `data == nullptr`.
         *
         * @note This is not a cryptographic hash. Do not use it for security-related logic.
         * @note If `data` is null, the return value is `-1` as a sentinel.
         * @note The hash is based only on the byte contents and length, not on type information.
         */
        [[nodiscard]] constexpr std::uint64_t
        hash(jh::utils::hash_fn::c_hash hash_method = jh::utils::hash_fn::c_hash::fnv1a64) const noexcept {
            if (!data) return static_cast<std::uint64_t>(-1);
            return utils::hash_fn::hash(hash_method, data, len);
        }

        /**
         * @brief Copies the view into a C-style null-terminated buffer.
         * @warning This is not POD-safe. Use for debugging/interop only.
         * @param buffer Output char buffer.
         * @param max_len Max bytes to write (including null terminator).
         */
        void copy_to(char *buffer, const std::uint64_t max_len) const noexcept {
            const std::uint64_t n = len < max_len - 1 ? len : max_len - 1;
            std::memcpy(buffer, data, n);
            buffer[n] = '\0';
        }
    };
} // namespace jh::pod
