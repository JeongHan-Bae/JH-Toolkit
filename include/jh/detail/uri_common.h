/**
 * @copyright
 * Copyright 2025 JeongHan-Bae &lt;mastropseudo\@gmail.com&gt;
 * <br>
 * Licensed under the Apache License, Version 2.0 (the "License"); <br>
 * you may not use this file except in compliance with the License.<br>
 * You may obtain a copy of the License at<br>
 * <br>
 *     http://www.apache.org/licenses/LICENSE-2.0<br>
 * <br>
 * Unless required by applicable law or agreed to in writing, software<br>
 * distributed under the License is distributed on an "AS IS" BASIS,<br>
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.<br>
 * See the License for the specific language governing permissions and<br>
 * limitations under the License.<br>
 * <br>
 * Full license: <a href="https://github.com/JeongHan-Bae/JH-Toolkit?tab=Apache-2.0-1-ov-file#readme">GitHub</a>
 */
/**
 * @file uri_common.h
 * @author JeongHan-Bae
 * <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 * @brief Internal URI percent-encoding utilities and lookup tables.
 *
 * <p>
 * This file provides the core low-level utilities used by the URI encoding
 * and decoding implementation. It defines <code>constexpr</code> lookup tables,
 * compile-time safe generators, and low-level unchecked encode/decode routines
 * designed for high performance and predictable behavior.
 * </p>
 *
 * <p>
 * The utilities here implement the internal mechanics of percent-encoding
 * as defined by URI standards. Characters not classified as valid URI
 * characters are transformed into their <code>\%HH</code> hexadecimal
 * representation during encoding, and decoded back to raw bytes during
 * decoding.
 * </p>
 *
 * <p><strong>Notes:</strong></p>
 * <ul>
 *   <li>This file is <strong>an internal component</strong>, <strong>not intended for external use</strong>.</li>
 *   <li>All functions are declared with <code>constexpr</code> and <code>noexcept</code>,
 *       allowing the compiler to perform aggressive optimization and
 *       compile-time evaluation when possible.</li>
 *   <li>Lookup tables are generated using <code>consteval</code> to guarantee
 *       compile-time construction and eliminate runtime initialization cost.</li>
 *   <li>All internal callers must guarantee input validity before invoking
 *       these functions, ensuring no out-of-bound memory access and strict
 *       semantic correctness.</li>
 *   <li>The <code>size</code> arguments always represent
 *       <strong>actual, valid buffer lengths</strong>.</li>
 * </ul>
 *
 * <p><strong>Warning:</strong></p>
 * <ul>
 *   <li>External users must not directly include or invoke any interface
 *       declared in this file.</li>
 *   <li>The internal behavior and ABI may change without notice.</li>
 *   <li>Direct use of these functions outside the intended internal
 *       call chain is considered <strong>undefined behavior (UB)</strong>.</li>
 * </ul>
 *
 * @version <pre>1.4.1</pre>
 * @date <pre>2025</pre>
 */


#pragma once

#include <cstdint>
#include <cstddef>
#include "jh/metax/char.h"
#include "jh/pods/array.h"

namespace jh::detail::uri_common {

    static constexpr jh::pod::array hex_encode_table = {"0123456789ABCDEF"};

    consteval jh::pod::array<std::uint8_t, 256> make_hex_decode_table() {
        jh::pod::array<std::uint8_t, 256> t{};

        for (std::size_t i = 0; i < 256; ++i)
            t[i] = 0xFF;

        for (std::size_t i = 0; i < 10; ++i)
            t[static_cast<std::size_t>('0') + i] = static_cast<std::uint8_t>(i);

        for (std::size_t i = 0; i < 6; ++i) {
            t[static_cast<std::size_t>('A') + i] = static_cast<std::uint8_t>(10 + i);
            t[static_cast<std::size_t>('a') + i] = static_cast<std::uint8_t>(10 + i);
        }

        return t;
    }

    static constexpr auto hex_decode_table = make_hex_decode_table();

    template<jh::meta::any_char Char>
    constexpr std::uint64_t calculate_encoded_length(const Char *src, std::uint64_t n) noexcept {
        std::uint64_t non_uri_count = 0;

        for (std::uint64_t i = 0; i < n; ++i) {
            if (!jh::meta::is_uri_char(src[i]))
                ++non_uri_count;
        }

        return n + 2 * non_uri_count;
    }

    template<jh::meta::any_char Char>
    constexpr void uri_encode_unchecked(
            const Char *src, std::uint64_t n,
            std::uint8_t *dst, std::uint64_t m
    ) noexcept {
        std::uint64_t j = 0;

        for (std::uint64_t i = 0; i < n; ++i) {
            const Char c = src[i];

            if (jh::meta::is_uri_char(c)) {
                if (j < m)
                    dst[j++] = static_cast<std::uint8_t>(c);
            } else {
                if (j + 3 <= m) {
                    dst[j++] = '%';

                    const auto encoded = static_cast<std::uint8_t>(c);

                    dst[j++] = static_cast<std::uint8_t>(
                            hex_encode_table[(encoded >> 4) & 0x0F]);

                    dst[j++] = static_cast<std::uint8_t>(
                            hex_encode_table[encoded & 0x0F]);
                }
            }
        }
    }

    template<jh::meta::any_char Char>
    constexpr std::uint64_t calculate_decoded_length(const Char *src, std::uint64_t n) noexcept {
        std::uint64_t percent_count = 0;

        for (std::uint64_t i = 0; i < n; ++i) {
            const Char c = src[i];

            if (c == '%') {
                if (i + 2 >= n)
                    return static_cast<std::uint64_t>(-1);

                if (jh::meta::is_hex_char(src[i + 1]) &&
                    jh::meta::is_hex_char(src[i + 2])) {

                    ++percent_count;
                    i += 2;
                } else {
                    return static_cast<std::uint64_t>(-1);
                }
            } else if (!jh::meta::is_uri_char(c)) {
                return static_cast<std::uint64_t>(-1);
            }
        }

        return n - 2 * percent_count;
    }

    template<jh::meta::any_char Char>
    constexpr void uri_decode_unchecked(
            const Char *src, std::uint64_t n,
            std::uint8_t *dst, std::uint64_t m
    ) noexcept {
        std::uint64_t j = 0;

        for (std::uint64_t i = 0; i < n; ++i) {
            const Char c = src[i];

            if (c == '%') {
                if (i + 2 < n) {
                    const auto hi =
                            hex_decode_table[static_cast<std::uint8_t>(src[i + 1])];
                    const auto lo =
                            hex_decode_table[static_cast<std::uint8_t>(src[i + 2])];

                    const auto decoded =
                            static_cast<std::uint8_t>((hi << 4) | lo);

                    if (j < m)
                        dst[j++] = decoded;

                    i += 2;
                }
            } else {
                if (j < m)
                    dst[j++] = static_cast<std::uint8_t>(c);
            }
        }
    }

} // namespace jh::detail::uri_common
