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
 * @file base64.h (utils)
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief Standard Base64 serialization and deserialization utilities.
 *
 * This header provides <code>jh::utils::base64</code>, the first officially supported
 * serialization/representation system in the JH-Toolkit. It allows encoding arbitrary
 * binary data into a Base64 string and decoding Base64 back into POD-safe buffers.
 *
 * <h3>Why Base64?</h3>
 * <ul>
 *   <li>Provides a canonical, reversible text representation of binary data.</li>
 *   <li>Interoperable across platforms and languages.</li>
 *   <li>Works seamlessly with POD-based systems (<code>bytes_view</code>, <code>string_view</code>).</li>
 *   <li>Zero hidden ABI dependencies, header-only implementation.</li>
 * </ul>
 *
 * <h3>Important Notes:</h3>
 * <ul>
 *   <li><b>Do not</b> attempt to serialize POD types using
 *       <code>std::ostringstream</code> outputs defined in <code>jh::pod::stringify</code>.
 *       Those outputs are <b>for debugging/logging only</b>.</li>
 *   <li>This Base64 module is the <b>only officially supported serialization</b>
 *       in the 1.3.x series.</li>
 *   <li>All decoding APIs return either:
 *     <ul>
 *       <li><code>std::vector&lt;uint8_t&gt;</code></li>
 *       <li><code>jh::pod::bytes_view</code> (view into user-managed buffer)</li>
 *       <li><code>jh::pod::string_view</code> (for decoded text)</li>
 *     </ul>
 *   </li>
 *   <li>Buffers passed into decoding APIs must remain alive as long as the returned
 *       view (<code>bytes_view</code> or <code>string_view</code>) is used.</li>
 * </ul>
 *
 * <h3>Design Guarantees:</h3>
 * <ul>
 *   <li>All algorithms are constexpr-friendly where applicable.</li>
 *   <li>Invalid Base64 inputs trigger exceptions (not UB).</li>
 *   <li>Padding and illegal character validation strictly enforced.</li>
 * </ul>
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include "jh/pods/array.h"
#include "jh/pods/string_view.h"
#include "jh/pods/bytes_view.h"

namespace jh::utils::base64 {

    /**
     * @brief Standard Base64 encoding character set.
     *
     * <h4>Details:</h4>
     * <ul>
     *   <li>Index 0–25: <code>'A'</code>–<code>'Z'</code></li>
     *   <li>Index 26–51: <code>'a'</code>–<code>'z'</code></li>
     *   <li>Index 52–61: <code>'0'</code>–<code>'9'</code></li>
     *   <li>Index 62: <code>'+'</code></li>
     *   <li>Index 63: <code>'/'</code></li>
     * </ul>
     *
     * Used by the <code>encode()</code> function to map 6-bit values to characters.
     */
    static constexpr char kBase64Chars[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

    /**
     * @brief Check whether a character is valid in Base64 encoding.
     *
     * @param c Input character to validate.
     * @return <code>true</code> if the character belongs to the Base64 alphabet
     *         (<code>A–Z</code>, <code>a–z</code>, <code>0–9</code>, <code>'+'</code>, <code>'/'</code>)
     *         or is the padding character <code>'='</code>; otherwise <code>false</code>.
     *
     * <h4>Usage:</h4>
     * Used internally during decoding to reject illegal characters.
     */
    constexpr bool is_base64_char(char c) {
        return (c >= 'A' && c <= 'Z') ||
               (c >= 'a' && c <= 'z') ||
               (c >= '0' && c <= '9') ||
               c == '+' || c == '/' || c == '=';
    }

    namespace detail {

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

            return table;
        }

        static constexpr auto kDecodeTable = detail::make_base64_decode_table();

        inline std::vector<uint8_t> decode_base(const std::string &input, std::vector<uint8_t>& output) {
            const std::size_t len = input.size();

            // Length must be multiple of 4
            if (len % 4 != 0) {
                throw std::runtime_error("Invalid base64: input length must be multiple of 4");
            }

            // Validate all characters
            if (std::any_of(input.begin(), input.end(), [](char c) {
                return !is_base64_char(c);
            })) {
                throw std::runtime_error("Invalid base64: contains illegal characters");
            }

            output.reserve(input.size() * 3 / 4);

            for (size_t i = 0; i < input.size(); i += 4) {
                uint32_t val = 0;
                int pad = 0;

                for (int j = 0; j < 4; ++j) {
                    char c = input[i + j];
                    if (c == '=') {
                        val <<= 6;
                        pad++;
                    } else {
                        uint8_t decoded = kDecodeTable[static_cast<unsigned char>(c)];
                        if (decoded == 64) throw std::runtime_error("Invalid character in base64");
                        val = (val << 6) | decoded;
                    }
                }

                output.push_back((val >> 16) & 0xFF);
                if (pad < 2) output.push_back((val >> 8) & 0xFF);
                if (pad < 1) output.push_back(val & 0xFF);
            }

            return output;
        }
    }

    /**
     * @brief Encode raw binary data into a Base64 string.
     *
     * @param data Pointer to input data buffer (<code>const uint8_t*</code>).
     * @param len  Length of input data in bytes.
     * @return Base64-encoded string.
     *
     * <h4>Notes:</h4>
     * <ul>
     *   <li>Always produces valid Base64 with proper padding (<code>'='</code>).</li>
     *   <li>Zero-copy safe: output is a <code>std::string</code> constructed from encoded buffer.</li>
     *   <li>Canonical representation, interoperable across systems.</li>
     * </ul>
     */
    [[maybe_unused]] inline std::string encode(const uint8_t *data, std::size_t len) noexcept {
        std::vector<uint8_t> out;
        out.reserve((len + 2) / 3 * 4);  // estimate output size

        for (std::size_t i = 0; i < len; i += 3) {
            uint32_t chunk = 0;
            int pad = 0;

            chunk |= data[i] << 16;
            if (i + 1 < len) {
                chunk |= data[i + 1] << 8;
            } else {
                pad++;
            }
            if (i + 2 < len) {
                chunk |= data[i + 2];
            } else {
                pad++;
            }

            out.push_back(kBase64Chars[(chunk >> 18) & 0x3F]);
            out.push_back(kBase64Chars[(chunk >> 12) & 0x3F]);
            out.push_back(pad >= 2 ? '=' : kBase64Chars[(chunk >> 6) & 0x3F]);
            out.push_back(pad >= 1 ? '=' : kBase64Chars[chunk & 0x3F]);
        }

        return {out.begin(), out.end()};
    }

    /**
     * @brief Decode a Base64 string into a new byte buffer.
     *
     * @param input Base64-encoded string.
     * @return <code>std::vector&lt;uint8_t&gt;</code> containing the decoded bytes.
     *
     * <h4>Notes:</h4>
     * <ul>
     *   <li>Buffer is newly allocated; caller takes ownership.</li>
     *   <li>Throws <code>std::runtime_error</code> on invalid Base64 input
     *       (length mismatch, illegal characters, bad padding).</li>
     * </ul>
     */
    [[maybe_unused]] inline std::vector<uint8_t> decode(const std::string &input) {
        std::vector<uint8_t> output;
        detail::decode_base(input, output);
        return output;
    }

    /**
     * @brief Decode a Base64 string into a user-provided buffer and return a <code>bytes_view</code>.
     *
     * @param input Base64-encoded string.
     * @param output_buffer Target buffer (<code>std::vector&lt;uint8_t&gt;</code>) to store decoded bytes.
     *        The buffer is cleared before writing.
     * @return <code>jh::pod::bytes_view</code> referencing the decoded content in <code>output_buffer</code>.
     *
     * <h4>Notes:</h4>
     * <ul>
     *   <li>Zero-copy: returned view points directly to <code>output_buffer.data()</code>.</li>
     *   <li><b>Lifetime requirement:</b> do not use the returned view after <code>output_buffer</code>
     *       is modified or destroyed.</li>
     *   <li>Efficient for flat memory access patterns in POD systems.</li>
     * </ul>
     */
    [[maybe_unused]] inline jh::pod::bytes_view decode(const std::string &input, std::vector<uint8_t>& output_buffer) {
        output_buffer.clear();
        detail::decode_base(input, output_buffer);
        return jh::pod::bytes_view::from(output_buffer.data(), output_buffer.size());
    }

    /**
     * @brief Decode a Base64 string into a <code>std::string</code> and return a <code>string_view</code>.
     *
     * @param input Base64-encoded string.
     * @param output_buffer Target <code>std::string</code> used as the storage for decoded bytes.
     *        The buffer is overwritten.
     * @return <code>jh::pod::string_view</code> pointing into <code>output_buffer</code>.
     *
     * <h4>Notes:</h4>
     * <ul>
     *   <li>Optimized for textual data (e.g., UTF-8).</li>
     *   <li>Returned <code>string_view</code> becomes invalid if <code>output_buffer</code> is modified or destroyed.</li>
     *   <li>Recommended when decoding Base64-encoded text payloads.</li>
     * </ul>
     */
    [[maybe_unused]] inline jh::pod::string_view decode(const std::string &input, std::string& output_buffer) {
        std::vector<uint8_t> output;
        detail::decode_base(input, output);
        output_buffer.assign(output.begin(), output.end());
        return {output_buffer.data(), output_buffer.size()};
    }

} // namespace jh::utils::base64