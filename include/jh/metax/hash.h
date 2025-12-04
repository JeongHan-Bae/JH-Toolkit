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
 * <h4>Supported constexpr hash algorithms</h4>
 * <ul>
 *   <li><b>FNV-1a 64</b> &mdash; Simple, fast, and widely used for identifiers.</li>
 *   <li><b>FNV-1 64</b>  &mdash; Variant with multiply-before-xor order.</li>
 *   <li><b>DJB2</b>      &mdash; Classic string hash, small code footprint.</li>
 *   <li><b>SDBM</b>      &mdash; Hash from old DB engines (used by readdir, ndbm).</li>
 *   <li><b>Murmur64</b>  &mdash; Seedless constexpr variant of MurmurHash3.</li>
 *   <li><b>xxHash64</b>  &mdash; Deterministic constexpr-safe xxHash-like algorithm.</li>
 * </ul>
 *
 * @details
 * All hash functions accept any character type satisfying <code>any_char</code>,
 * namely <code>char</code>, <code>signed char</code>, and <code>unsigned char</code>.
 * These can be passed directly as <code>const Char*</code> without casting.
 *
 * Other types, such as <code>bool</code> or <code>std::byte</code>,
 * are not character types and must be explicitly converted via
 * <code>reinterpret_cast&lt;const char*&gt;</code> if used for hashing.
 * This restriction ensures semantic clarity and prevents accidental misuse
 * of non-textual memory as string data in compile-time contexts.
 *
 * <p>
 * For safe reinterpretation of POD objects or contiguous buffers,
 * see <code>jh::pod::bytes_view</code>.
 * </p>
 *
 * @version 1.3.x
 * @date 2025
 */

#pragma once

#include <cstdint>
#include <concepts>
#include "jh/metax/char.h"

namespace jh::meta {

    /// @brief Compile-time selectable hash algorithm tag (FNV, DJB2, SDBM, etc.)
    enum class c_hash : std::uint8_t {
        fnv1a64 = 0,  ///< FNV-1a 64-bit hash
        fnv1_64 = 1,  ///< FNV-1 64-bit hash
        djb2 = 2,     ///< DJB2 hash (classic string hash)
        sdbm = 3,     ///< SDBM hash (used in readdir, DBM)
        murmur64 = 4, ///< constexpr-safe MurmurHash variant (seedless)
        xxhash64 = 5  ///< constexpr xxHash64 variant (seedless)
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

    /// @brief constexpr MurmurHash-like 64-bit variant (seedless)
    template<any_char Char>
    constexpr std::uint64_t murmur64(const Char *data, const std::uint64_t size) noexcept {
        std::uint64_t h = 0x87c37b91114253d5ull;
        constexpr std::uint64_t c1 = 0x87c37b91114253d5ull;
        constexpr std::uint64_t c2 = 0x4cf5ad432745937full;
        for (std::uint64_t i = 0; i < size; ++i) {
            std::uint64_t k = static_cast<std::uint8_t>(data[i]);
            k *= c1;
            k = (k << 31) | (k >> (64 - 31));
            k *= c2;
            h ^= k;
            h = ((h << 27) | (h >> (64 - 27))) * 5 + 0x52dce729;
        }
        // Finalization (avalanche)
        h ^= size;
        h ^= (h >> 33);
        h *= 0xff51afd7ed558ccdull;
        h ^= (h >> 33);
        h *= 0xc4ceb9fe1a85ec53ull;
        h ^= (h >> 33);
        return h;
    }

    /// @brief constexpr xxHash-like 64-bit variant (seedless)
    template<any_char Char>
    constexpr std::uint64_t xxhash64(const Char *data, std::uint64_t len) noexcept {
        constexpr std::uint64_t PRIME1 = 11400714785074694791ull;
        constexpr std::uint64_t PRIME2 = 14029467366897019727ull;
        constexpr std::uint64_t PRIME3 = 1609587929392839161ull;
        constexpr std::uint64_t PRIME5 = 2870177450012600261ull;
        std::uint64_t h64 = PRIME5 + len;
        // no seed, simple accumulation
        for (std::uint64_t i = 0; i < len; ++i) {
            h64 += static_cast<std::uint8_t>(data[i]) * PRIME5;
            h64 = (h64 << 11) | (h64 >> (64 - 11));
            h64 *= PRIME1;
        }
        // final avalanche (same as xxHash)
        h64 ^= h64 >> 33;
        h64 *= PRIME2;
        h64 ^= h64 >> 29;
        h64 *= PRIME3;
        h64 ^= h64 >> 32;
        return h64;
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
            case c_hash::murmur64:
                return murmur64(data, size);
            case c_hash::xxhash64:
                return xxhash64(data, size);
        }
        return static_cast<std::uint64_t>(-1); // Illegal
    }
} // namespace jh::meta
