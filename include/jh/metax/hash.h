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
 * @file hash.h (metax)
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief constexpr-safe, compile-time hash algorithms for meta utilities.
 *
 * Provides a minimal set of constexpr 64-bit hash functions usable in compile-time
 * contexts, such as type reflection, lookup maps, or <code>consteval</code> identifiers.
 * All implementations avoid heap and STL dependencies.
 *
 * @details
 * Only <code>const char*</code> input is accepted directly.
 *
 * <p>
 * On most platforms, <code>uint8_t</code> and <code>int8_t</code> are type aliases of
 * <code>unsigned char</code> and <code>signed char</code>, respectively.
 * Their pointers can therefore be safely used as <code>const Char*</code> for hashing
 * and do not require any cast.
 *
 * However, types such as <code>bool</code> and <code>std::byte</code> are not character
 * types and cannot implicitly represent textual or contiguous byte sequences.
 * They must be explicitly converted via
 * <code>reinterpret_cast&lt;const char*&gt;</code> when used for hashing.
 * This enforces semantic intent and prevents misuse of non-character memory
 * representations in compile-time contexts.
 * </p>
 *
 * For safe reinterpretation of POD objects or contiguous buffers,
 * see <code>jh::pod::bytes_view</code>.
 *
 * @version 1.3.x
 * @date 2025
 */

#pragma once

#include <cstdint>
#include <concepts>

namespace jh::meta {

    /**
     * @brief Concept representing <tt>character-semantic</tt> 1-byte integral types.
     *
     * This concept includes only the fundamental character types that the
     * implementation treats as one-byte integral entities directly usable as
     * textual or byte-sequence data.
     *
     * Specifically, it accepts:
     * <ul>
     *   <li><code>char</code></li>
     *   <li><code>signed char</code></li>
     *   <li><code>unsigned char</code></li>
     * </ul>
     *
     * These are the <tt>clean</tt> core character types without cv-qualifiers or references,
     * and are guaranteed to be exactly 1 byte in size (<tt>sizeof(T) == 1</tt>).
     *
     * @note
     * <code>char8_t</code> is <b>not</b> included.
     * It is a distinct built-in type introduced by <code>__cpp_char8_t</code> for UTF-8
     * character support, not considered equivalent to any form of <code>char</code>.
     * It represents <tt>UTF-8 code units</tt>, not raw bytes, and therefore requires
     * explicit conversion when hashing.
     *
     * @details
     * This constraint ensures that only raw character data (in the sense of
     * 1-byte memory representation types) participates in compile-time hashing.
     * Non-character or higher-level types such as:
     * <ul>
     *   <li><code>bool</code></li>
     *   <li><code>std::byte</code></li>
     *   <li><code>char8_t</code></li>
     * </ul>
     * must be explicitly converted via
     * <code>reinterpret_cast&lt;const char&#42;&gt;</code>.
     *
     * The intent is to enforce semantic correctness and guarantee that hashing
     * remains <tt>constexpr</tt>-safe, type-clean, and free of undefined behavior
     * across translation units.
     */
    template<typename T>
    concept any_char =
    std::same_as<T, char> ||
    std::same_as<T, signed char> ||
    std::same_as<T, unsigned char>;

    /// @brief Compile-time selectable hash algorithm tag (FNV, DJB2, SDBM, etc.)
    enum class c_hash : std::uint8_t {
        fnv1a64 = 0,  ///< FNV-1a 64-bit hash
        fnv1_64 = 1,  ///< FNV-1 64-bit hash
        djb2 = 2,     ///< DJB2 hash (classic string hash)
        sdbm = 3      ///< SDBM hash (used in readdir, DBM)
    };

    /// @brief FNV-1a 64-bit hash implementation (default choice)
    template<any_char Char>
    constexpr std::uint64_t fnv1a64(const Char *data, const std::uint64_t size) noexcept {
        std::uint64_t h = 14695981039346656037ull;
        for (std::uint64_t i = 0; i < size; ++i) {
            h ^= static_cast<std::uint8_t>(data[i]);
            h *= 1099511628211ull;
        }
        return h;
    }

    /// @brief FNV-1 64-bit hash (multiply before xor)
    template<any_char Char>
    constexpr std::uint64_t fnv1_64(const Char *data, const std::uint64_t size) noexcept {
        std::uint64_t h = 14695981039346656037ull;
        for (std::uint64_t i = 0; i < size; ++i) {
            h *= 1099511628211ull;
            h ^= static_cast<std::uint8_t>(data[i]);
        }
        return h;
    }

    /// @brief DJB2 hash (hash * 33 + c)
    template<any_char Char>
    constexpr std::uint64_t djb2(const Char *str, const std::uint64_t size) noexcept {
        std::uint64_t hash = 5381;
        for (std::uint64_t i = 0; i < size; ++i) {
            hash = ((hash << 5) + hash) + static_cast<std::uint8_t>(str[i]);
        }
        return hash;
    }

    /// @brief SDBM hash (used in several DB engines)
    template<any_char Char>
    constexpr std::uint64_t sdbm(const Char *str, const std::uint64_t size) noexcept {
        std::uint64_t hash = 0;
        for (std::uint64_t i = 0; i < size; ++i) {
            hash = static_cast<std::uint8_t>(str[i]) + (hash << 6) + (hash << 16) - hash;
        }
        return hash;
    }

    /// @brief Dispatch to selected hash algorithm based on c_hash
    template<any_char Char>
    constexpr std::uint64_t hash(const c_hash algo, const Char *data, const std::uint64_t size) noexcept {
        switch (algo) {
            case c_hash::fnv1a64:
                return fnv1a64(data, size);
            case c_hash::fnv1_64:
                return fnv1_64(data, size);
            case c_hash::djb2:
                return djb2(data, size);
            case c_hash::sdbm:
                return sdbm(data, size);
        }
        return static_cast<std::uint64_t>(-1); // Illegal
    }
} // namespace jh::meta
