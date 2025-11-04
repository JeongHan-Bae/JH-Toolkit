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
 * @file hash.h (meta)
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
 * Their pointers can therefore be safely used as <code>const char*</code> for hashing
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

namespace jh::meta {

    /// Compile-time selectable hash algorithm tag (FNV, DJB2, SDBM, etc.)
    enum class c_hash : std::uint8_t {
        fnv1a64 = 0,  ///< FNV-1a 64-bit hash
        fnv1_64 = 1,  ///< FNV-1 64-bit hash
        djb2 = 2,     ///< DJB2 hash (classic string hash)
        sdbm = 3      ///< SDBM hash (used in readdir, DBM)
    };

    /// FNV-1a 64-bit hash implementation (default choice)
    constexpr std::uint64_t fnv1a64(const char *data, const std::uint64_t size) noexcept {
        std::uint64_t h = 14695981039346656037ull;
        for (std::uint64_t i = 0; i < size; ++i) {
            h ^= static_cast<std::uint8_t>(data[i]);
            h *= 1099511628211ull;
        }
        return h;
    }

    /// FNV-1 64-bit hash (multiply before xor)
    constexpr std::uint64_t fnv1_64(const char *data, const std::uint64_t size) noexcept {
        std::uint64_t h = 14695981039346656037ull;
        for (std::uint64_t i = 0; i < size; ++i) {
            h *= 1099511628211ull;
            h ^= static_cast<std::uint8_t>(data[i]);
        }
        return h;
    }

    /// DJB2 hash (hash * 33 + c)
    constexpr std::uint64_t djb2(const char *str, const std::uint64_t size) noexcept {
        std::uint64_t hash = 5381;
        for (std::uint64_t i = 0; i < size; ++i) {
            hash = ((hash << 5) + hash) + static_cast<std::uint8_t>(str[i]);
        }
        return hash;
    }

    /// SDBM hash (used in several DB engines)
    constexpr std::uint64_t sdbm(const char *str, const std::uint64_t size) noexcept {
        std::uint64_t hash = 0;
        for (std::uint64_t i = 0; i < size; ++i) {
            hash = static_cast<std::uint8_t>(str[i]) + (hash << 6) + (hash << 16) - hash;
        }
        return hash;
    }

    /// Dispatch to selected hash algorithm based on c_hash
    constexpr std::uint64_t hash(const c_hash algo, const char *data, const std::uint64_t size) noexcept {
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
}
