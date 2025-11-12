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
 * @file base64_common.h (detail)
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief Internal base64/base64url common utilities and tables.
 *
 * <p>
 * This file provides the core low-level utilities used by the Base64 and Base64URL
 * encoding and decoding processes. It defines <code>constexpr</code> tables,
 * compile-time safe functions, and low-level unchecked encode/decode routines
 * designed for performance and static safety.
 * </p>
 *
 * <p><strong>Notes:</strong></p>
 * <ul>
 *   <li>This file is <strong>an internal component</strong>, <strong>not intended for external use</strong>.</li>
 *   <li>All functions are declared with <code>constexpr</code> and <code>noexcept</code>,
 *       enabling the compiler to perform advanced optimization, block analysis,
 *       and automatic vectorization.</li>
 *   <li>All internal calls are guaranteed to be validated before reaching these
 *       functions, ensuring no out-of-bound memory access and preserving strict
 *       semantic correctness.</li>
 *   <li>The <code>size</code> arguments always represent <strong>actual, valid buffer lengths</strong>.</li>
 * </ul>
 *
 * <p><strong>Warning:</strong></p>
 * <ul>
 *   <li>External users must not directly include or invoke any interface in this file.</li>
 *   <li>The internal behavior and ABI may change without notice.</li>
 *   <li>Invoking any of these functions directly from external code is considered
 *       <strong>undefined behavior (UB)</strong>.</li>
 * </ul>
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */


#pragma once

#include <cstdint>
#include <cstddef>
#include "jh/metax/char.h"
#include "jh/pods/array.h"
#include <algorithm>

namespace jh::detail::base64_common {

    template<jh::meta::any_char Char>
    [[nodiscard]] constexpr int base64_check(const Char* src, std::uint64_t n) noexcept {
        // pad count or -1 if invalid
        if (n == 0 || n % 4 != 0) return -1;

        for (std::uint64_t i = 0; i < n - 2; ++i)
            if (!jh::meta::is_base64_core(src[i])) return -1;

        const char c3 = src[n - 2];
        const char c4 = src[n - 1];

        if (c3 == '=' && c4 == '=') return 2;
        if (jh::meta::is_base64_core(c3) && c4 == '=') return 1;
        if (jh::meta::is_base64_core(c3) && jh::meta::is_base64_core(c4)) return 0;

        return -1;
    }

    template<jh::meta::any_char Char>
    [[nodiscard]] constexpr bool is_base64(const Char* src, std::uint64_t n) noexcept {
        return base64_check(src, n) != -1;
    }

    template<jh::meta::any_char Char>
    [[nodiscard]] constexpr int base64url_check(const Char* src, std::uint64_t n) noexcept {
        // pad count or -1 if invalid

        if (n == 0) return -1;

        if (n % 4 == 0) {
            for (std::uint64_t i = 0; i < n - 2; i++)
                if (!jh::meta::is_base64url_core(src[i])) return -1;

            const char c3 = src[n - 2];
            const char c4 = src[n - 1];
            if (c3 == '=' && c4 == '=') return 2;
            if (jh::meta::is_base64url_core(c3) && c4 == '=') return 1;
            if (jh::meta::is_base64url_core(c3) && jh::meta::is_base64url_core(c4)) return 0;
            return -1;
        }

        if (n % 4 == 1) return -1;
        for (std::uint64_t i = 0; i < n; i++)
            if (!jh::meta::is_base64url_core(src[i])) return -1;
        return 0;
    }

    template<jh::meta::any_char Char>
    [[nodiscard]] constexpr bool is_base64url(const Char* src, std::uint64_t n) noexcept {
        return base64url_check(src, n) != -1;
    }

    constexpr auto make_base64_encode_table() noexcept {
        jh::pod::array<char, 64> t{};
        for (int i = 0; i < 26; ++i) t[i] = static_cast<char>('A' + i);
        for (int i = 0; i < 26; ++i) t[i + 26] = static_cast<char>('a' + i);
        for (int i = 0; i < 10; ++i) t[i + 52] = static_cast<char>('0' + i);
        t[62] = '+';
        t[63] = '/';
        return t;
    }

    constexpr auto make_base64url_encode_table() noexcept {
        jh::pod::array<char, 64> t{};
        for (int i = 0; i < 26; ++i) t[i] = static_cast<char>('A' + i);
        for (int i = 0; i < 26; ++i) t[i + 26] = static_cast<char>('a' + i);
        for (int i = 0; i < 10; ++i) t[i + 52] = static_cast<char>('0' + i);
        t[62] = '-';
        t[63] = '_';
        return t;
    }

    consteval auto make_base64_decode_table() {
        jh::pod::array<uint8_t, 256> table{};

        // initialize as illegal
        for (int i = 0; i < 256; ++i) {
            table[i] = 64;
        }

        // A–Z
        for (int i = 'A'; i <= 'Z'; ++i) {
            table[i] = i - 'A';
        }

        // a–z
        for (int i = 'a'; i <= 'z'; ++i) {
            table[i] = i - 'a' + 26;
        }

        // 0–9
        for (int i = '0'; i <= '9'; ++i) {
            table[i] = i - '0' + 52;
        }

        table['+'] = 62;
        table['/'] = 63;
        table['-'] = 62;  // URL-safe
        table['_'] = 63;  // URL-safe

        return table;
    }

    static constexpr auto encode_table = make_base64_encode_table();
    static constexpr auto encode_table_url = make_base64url_encode_table();


    static constexpr auto decode_table = make_base64_decode_table();

    constexpr std::uint64_t encoded_len_base64(std::uint64_t raw_len) noexcept {
        /// base64 and base64url with pad
        return ((raw_len + 2) / 3) * 4;
    }

    constexpr std::uint64_t encoded_len_base64url_no_pad(std::uint64_t raw_len) noexcept {
        /// base64url with no pad
        return ((raw_len * 4) + 2) / 3;
    }

    constexpr std::uint64_t decoded_len_base64(std::uint64_t enc_len, std::uint8_t pad) noexcept {
        /// base64 and base64url with pad
        return (enc_len / 4) * 3 - pad;
    }

    constexpr std::uint64_t decoded_len_base64url_no_pad(std::uint64_t enc_len) noexcept {
        /// base64url with no pad
        switch (enc_len % 4) {
            case 0: return (enc_len / 4) * 3;
            case 2: return (enc_len / 4) * 3 + 1;
            case 3: return (enc_len / 4) * 3 + 2;
            default: return 0; // Illegal
        }
    }

    template<bool URLMode = false>
    constexpr std::uint64_t base64_encode_unchecked(
            const std::uint8_t* src, std::uint64_t n,
            char* dst, bool pad = true
    ) noexcept {
        using namespace jh::detail::base64_common;

        const auto& table = URLMode ? encode_table_url : encode_table;
        std::uint64_t i = 0, j = 0;

        while (i + 2 < n) {
            const uint32_t triple = (src[i] << 16) | (src[i + 1] << 8) | src[i + 2];
            dst[j++] = table[(triple >> 18) & 0x3F];
            dst[j++] = table[(triple >> 12) & 0x3F];
            dst[j++] = table[(triple >> 6) & 0x3F];
            dst[j++] = table[triple & 0x3F];
            i += 3;
        }

        if (i < n) {
            const uint32_t rem = n - i;
            uint32_t triple = src[i] << 16;
            if (rem == 2) triple |= src[i + 1] << 8;

            dst[j++] = table[(triple >> 18) & 0x3F];
            dst[j++] = table[(triple >> 12) & 0x3F];

            if (rem == 2) {
                dst[j++] = table[(triple >> 6) & 0x3F];
                if (pad) dst[j++] = '=';
            } else { // rem == 1
                if (pad) {
                    dst[j++] = '=';
                    dst[j++] = '=';
                }
            }
        }
        return j;
    }

    template<jh::meta::any_char Char>
    constexpr void base64_decode_unchecked(
            const Char* src, std::uint64_t n,
            std::uint8_t* dst, std::uint64_t m
    ) noexcept {
        /// any base64 or base64url
        using namespace jh::detail::base64_common;
        std::uint64_t j = 0;
        for (std::uint64_t i = 0; i < n; i += 4) {
            const uint32_t a = decode_table[src[i]];
            const uint32_t b = decode_table[src[i + 1]];
            const uint32_t c = (i + 2 < n) ? decode_table[src[i + 2]] : 0;
            const uint32_t d = (i + 3 < n) ? decode_table[src[i + 3]] : 0;

            const uint32_t triple = (a << 18) | (b << 12) | (c << 6) | d;

            if (j < m) dst[j++] = static_cast<uint8_t>((triple >> 16) & 0xFF);
            if (j < m) dst[j++] = static_cast<uint8_t>((triple >> 8) & 0xFF);
            if (j < m) dst[j++] = static_cast<uint8_t>(triple & 0xFF);
        }
    }
}
