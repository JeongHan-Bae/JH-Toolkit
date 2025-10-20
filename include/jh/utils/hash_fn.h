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
 * @file hash_fn.h
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief constexpr-safe hashing utilities with static algorithm selection.
 *
 * Provides multiple 64-bit non-cryptographic hash functions for IDs, bucketing,
 * and lightweight indexing. These algorithms are constexpr-friendly and avoid
 * heap or STL dependencies. Not suitable for cryptographic use.
 *
 * <h3>Supported Algorithms (<code>jh::utils::hash_fn::c_hash</code>):</h3>
 * <ul>
 *   <li><code>c_hash::fnv1a64</code> — FNV-1a 64-bit hash (xor then multiply, default)</li>
 *   <li><code>c_hash::fnv1_64</code> — FNV-1 64-bit hash (multiply then xor)</li>
 *   <li><code>c_hash::djb2</code> — DJB2 hash (classic string hash: h * 33 + c)</li>
 *   <li><code>c_hash::sdbm</code> — SDBM hash (used in DB engines, e.g. readdir)</li>
 * </ul>
 *
 * <h3>Pointer Type Policy:</h3>
 * <ul>
 *   <li>All functions take <code>const char*</code> as the canonical input type.</li>
 *   <li>The following are accepted transparently:
 *     <ul>
 *       <li><code>const char*</code></li>
 *       <li><code>const unsigned char*</code> / <code>const uint8_t*</code></li>
 *       <li><code>const signed char*</code> / <code>const int8_t*</code></li>
 *     </ul>
 *     Regardless of platform-defined <code>char</code> signedness, all input bytes
 *     are normalized internally via <code>static_cast&lt;uint8_t&gt;</code>.</li>
 *   <li><code>std::byte*</code> is <b>not</b> supported directly:
 *       must be explicitly cast with <code>reinterpret_cast&lt;const char*&gt;</code>.</li>
 *   <li>This restriction ensures constexpr/consteval safety. At runtime,
 *       reinterpret_cast is fully optimized and safe.</li>
 * </ul>
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <cstdint>

namespace jh::utils::hash_fn {

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
