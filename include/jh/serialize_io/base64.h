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
 * @file base64.h
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 * @brief High-level Base64 and Base64URL serialization interface for the JH Toolkit.
 *
 * <p>
 * This header provides a modern, type-safe, and constexpr-enabled Base64 / Base64URL
 * codec implementation. It is part of the <b>JH Toolkit Serialization I/O module</b>
 * (shortened as <code>jh::serio</code>), and serves as the first officially supported
 * serialization format within the toolkit.
 * </p>
 *
 * <h3>Design overview</h3>
 * <ul>
 *   <li>Implements <b>Base64</b> (RFC 4648 &sect;4) and <b>Base64URL</b> (RFC 4648 &sect;5).</li>
 *   <li>Cross-language interoperable binary-to-text serialization format.</li>
 *   <li>All encoding/decoding logic relies on constexpr-verified lookup tables.</li>
 *   <li>All interfaces perform pre-validation and throw clear <code>std::runtime_error</code> on error.</li>
 *   <li>Fully portable across compilers and platforms.</li>
 * </ul>
 *
 * <h3>Usage</h3>
 * @code
 * std::vector&lt;uint8_t&gt; raw = {0x01, 0x02, 0x03};
 * std::string encoded = jh::serio::base64::encode(raw.data(), raw.size());
 * auto decoded = jh::serio::base64::decode(encoded);
 * @endcode
 *
 * @version <pre>1.3.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <stdexcept>

#include "jh/pods/string_view.h"
#include "jh/pods/bytes_view.h"
#include "jh/detail/base64_common.h"

namespace jh::serio {

    /**
     * @brief Implements standard Base64 (RFC 4648 &sect;4) encoding and decoding.
     *
     * <p>
     * The Base64 codec is designed for <b>binary-to-text serialization</b>.
     * It ensures cross-language interoperability and safe transmission of binary
     * payloads through text-based channels such as JSON, HTTP, XML, or protocol buffers.
     * </p>
     *
     * <p>
     * This namespace provides safe, high-level wrappers over the constexpr-verified
     * low-level primitives in <code>jh::detail::base64_common</code>.
     * </p>
     */
    namespace base64 {

        using namespace jh::detail::base64_common;

        /**
         * @brief Encode raw binary data into a Base64 string.
         *
         * @param data Pointer to the binary buffer to encode.
         * @param len  Number of bytes to encode.
         * @return A Base64-encoded string with padding ('=').
         *
         * @throw std::invalid_argument if @p data is null and @p len > 0.
         *
         * @note This function always produces a padded Base64 output.
         */
        [[nodiscard]] inline std::string encode(const uint8_t *data, std::size_t len) {
            if (data == nullptr && len > 0)
                throw std::invalid_argument("encode(): null pointer with non-zero length");

            const auto encoded_len = encoded_len_base64(len);
            std::vector<char> buffer(encoded_len);

            base64_encode_unchecked<false>(data, len, buffer.data(), true);

            return {
                    std::make_move_iterator(buffer.begin()),
                    std::make_move_iterator(buffer.end())
            };
        }

        /**
         * @brief Decode a Base64 string into a byte vector.
         *
         * @param input The Base64-encoded input string.
         * @return A vector of decoded bytes.
         *
         * @throw std::runtime_error if input is not a valid Base64 string.
         */
        [[nodiscard]] inline std::vector<uint8_t> decode(const std::string &input) {
            const auto n = input.size();
            if (n == 0)
                return {};

            const int pad = base64_check(input.data(), n);
            if (pad == -1)
                throw std::runtime_error("Invalid Base64: bad length, illegal characters, or bad padding");

            const auto decoded_len = decoded_len_base64(n, static_cast<uint8_t>(pad));

            std::vector<uint8_t> output(decoded_len);
            base64_decode_unchecked(input.data(), n, output.data(), decoded_len);
            return output;
        }

        /**
         * @brief Decode a Base64-encoded string into raw bytes.
         *
         * This overload operates in <strong>byte mode</strong>.
         *
         * @param input Base64-encoded input string.
         * @param output_buffer Output buffer storing the decoded bytes.
         * @return A <code>jh::pod::bytes_view</code> referencing the decoded data.
         *
         * @note
         * <ul>
         *   <li>The returned view is <strong>non-owning</strong> and bound to <code>output_buffer</code>.</li>
         *   <li>The view represents a contiguous, POD-safe region of memory and provides
         *       access utilities for direct reinterpretation, inspection, and hashing.</li>
         *   <li><strong>Available operations in <code>bytes_view</code>:</strong></li>
         *   <ul>
         *     <li><code>at&lt;T&gt;(offset = 0)</code> &mdash; reinterpret a subregion as a reference to <code>T</code> (no bounds checking).</li>
         *     <li><code>fetch&lt;T&gt;(offset = 0)</code> &mdash; safely fetch a pointer to <code>T</code>; returns <code>nullptr</code> if out of range.</li>
         *     <li><code>clone&lt;T&gt;()</code> &mdash; clone the entire view into a value of type <code>T</code>; returns <code>T&#123;&#125;</code> on size mismatch.</li>
         *     <li><code>hash(jh::meta::c_hash hash_method = jh::meta::c_hash::fnv1a64)</code> &mdash; compute a 64-bit hash of the byte content.</li>
         *   </ul>
         *   <li>These operations ensure POD-safe reinterpretation and provide zero-overhead access to binary data.</li>
         * </ul>
         */
        inline jh::pod::bytes_view decode(
                const std::string &input,
                std::vector<uint8_t> &output_buffer
        ) {
            output_buffer = decode(input);
            return jh::pod::bytes_view::from(output_buffer.data(), output_buffer.size());
        }

        /**
         * @brief Decode a Base64-encoded string into textual data.
         *
         * This overload operates in <strong>string mode</strong>.
         *
         * @param input Base64-encoded input string.
         * @param output_buffer Output buffer storing the decoded string.
         * @return A <code>jh::pod::string_view</code> referencing the decoded text.
         *
         * @note
         * <ul>
         *   <li>The returned view is <strong>non-owning</strong> and bound to <code>output_buffer</code>.</li>
         *   <li>The view represents a lightweight, read-only window over the decoded text.</li>
         *   <li><strong>Available operations in <code>string_view</code>:</strong></li>
         *   <ul>
         *     <li><code>operator[](std::uint64_t index)</code> &mdash; direct character access (no bounds checking).</li>
         *     <li><code>sub(std::uint64_t offset, std::uint64_t length = 0)</code> &mdash; create a substring view.</li>
         *     <li><code>compare(const string_view &amp;rhs)</code> &mdash; perform lexical comparison (similar to <code>strcmp</code>).</li>
         *     <li><code>starts_with(const string_view &amp;prefix)</code> &mdash; check if the view starts with the specified prefix.</li>
         *     <li><code>ends_with(const string_view &amp;suffix)</code> &mdash; check if the view ends with the specified suffix.</li>
         *     <li><code>find(char ch)</code> &mdash; locate the first occurrence of a character; returns <code>-1</code> if not found.</li>
         *     <li><code>hash(jh::meta::c_hash hash_method = jh::meta::c_hash::fnv1a64)</code> &mdash; compute a stable 64-bit hash of the contents.</li>
         *     <li><code>operator std::string_view()</code> or <code>to_std()</code> &mdash; explicit conversion to <code>std::string_view</code> for interoperability.</li>
         *     <li><code>operator&lt;=&gt;(const string_view &amp;rhs)</code> &mdash; perform lexicographical three-way comparison (returns <code>std::strong_ordering</code>).</li>
         *   </ul>
         *   <li>All string operations are <strong>constexpr</strong> and <strong>noexcept</strong>, providing zero-overhead interoperability
         *       with the standard library and compile-time evaluation where applicable.</li>
         * </ul>
         */
        inline jh::pod::string_view decode(
                const std::string &input,
                std::string &output_buffer
        ) {
            auto temp = decode(input);
            output_buffer = std::string(
                    std::make_move_iterator(temp.begin()),
                    std::make_move_iterator(temp.end())
            );
            return {output_buffer.data(), output_buffer.size()};
        }
    } // namespace base64


    /**
     * @brief Implements Base64URL (RFC 4648 &sect;5) &mdash; the URL-safe variant of Base64.
     *
     * <p>
     * Base64URL replaces <code>'+'</code> with <code>'-'</code> and
     * <code>'/'</code> with <code>'_'</code> to ensure safe embedding in
     * URLs and filenames without escaping.
     * </p>
     *
     * <p>
     * This variant supports both padded (<code>=</code>) and non-padded forms.
     * Non-padded encoding is the default behavior to comply with JWT and modern web APIs.
     * </p>
     */
    namespace base64url {

        using namespace jh::detail::base64_common;

        /**
         * @brief Encode raw binary data into a Base64URL string.
         *
         * @param data Pointer to input buffer.
         * @param len  Length of input buffer.
         * @param pad  Whether to include '=' padding (default: false).
         * @return Encoded Base64URL string.
         *
         * @throw std::invalid_argument if @p data is null and @p len > 0.
         *
         * @note When @p pad = false, the output omits trailing '=' characters.
         */
        [[nodiscard]] inline std::string encode(const uint8_t *data, std::size_t len, bool pad = false) {
            if (data == nullptr && len > 0)
                throw std::invalid_argument("encode(): null pointer with non-zero length");

            const auto encoded_len = pad
                                     ? encoded_len_base64(len)
                                     : encoded_len_base64url_no_pad(len);

            std::vector<char> buffer(encoded_len);

            base64_encode_unchecked<true>(data, len, buffer.data(), pad);

            return {
                    std::make_move_iterator(buffer.begin()),
                    std::make_move_iterator(buffer.end())
            };
        }

        /**
         * @brief Decode a Base64URL string into a byte vector.
         *
         * @param input Base64URL string (with or without padding).
         * @return A vector of decoded bytes.
         *
         * @throw std::runtime_error if input is not valid Base64URL.
         */
        [[nodiscard]] inline std::vector<uint8_t> decode(const std::string &input) {
            const auto n = input.size();
            if (n == 0)
                return {};

            const int pad = base64url_check(input.data(), n);
            if (pad == -1)
                throw std::runtime_error("Invalid Base64URL: bad length or illegal characters");

            const auto decoded_len = (pad > 0)
                                     ? decoded_len_base64(n, static_cast<uint8_t>(pad))
                                     : decoded_len_base64url_no_pad(n);

            std::vector<uint8_t> output(decoded_len);
            base64_decode_unchecked(input.data(), n, output.data(), decoded_len);
            return output;
        }

        /**
         * @brief Decode a Base64URL-encoded string into raw bytes.
         *
         * This overload operates in <strong>byte mode</strong>.
         *
         * @param input Base64URL-encoded input string (padded or non-padded).
         * @param output_buffer Output buffer storing the decoded bytes.
         * @return A <code>jh::pod::bytes_view</code> referencing the decoded data.
         *
         * @note
         * <ul>
         *   <li>The returned view is <strong>non-owning</strong> and bound to <code>output_buffer</code>.</li>
         *   <li>The view represents a contiguous, POD-safe region of memory and provides
         *       access utilities for direct reinterpretation, inspection, and hashing.</li>
         *   <li><strong>Available operations in <code>bytes_view</code>:</strong></li>
         *   <ul>
         *     <li><code>at&lt;T&gt;(offset = 0)</code> &mdash; reinterpret a subregion as a reference to <code>T</code> (no bounds checking).</li>
         *     <li><code>fetch&lt;T&gt;(offset = 0)</code> &mdash; safely fetch a pointer to <code>T</code>; returns <code>nullptr</code> if out of range.</li>
         *     <li><code>clone&lt;T&gt;()</code> &mdash; clone the entire view into a value of type <code>T</code>; returns <code>T&#123;&#125;</code> on size mismatch.</li>
         *     <li><code>hash(jh::meta::c_hash hash_method = jh::meta::c_hash::fnv1a64)</code> &mdash; compute a 64-bit hash of the byte content.</li>
         *   </ul>
         *   <li>These operations ensure POD-safe reinterpretation and provide zero-overhead access to binary data.</li>
         * </ul>
         */
        inline jh::pod::bytes_view decode(
                const std::string &input,
                std::vector<uint8_t> &output_buffer
        ) {
            output_buffer = decode(input);
            return jh::pod::bytes_view::from(output_buffer.data(), output_buffer.size());
        }

        /**
         * @brief Decode a Base64URL-encoded string into textual data.
         *
         * This overload operates in <strong>string mode</strong>.
         *
         * @param input Base64URL-encoded input string (padded or non-padded).
         * @param output_buffer Output buffer storing the decoded string.
         * @return A <code>jh::pod::string_view</code> referencing the decoded text.
         *
         * @note
         * <ul>
         *   <li>The returned view is <strong>non-owning</strong> and bound to <code>output_buffer</code>.</li>
         *   <li>The view represents a lightweight, read-only window over the decoded text.</li>
         *   <li><strong>Available operations in <code>string_view</code>:</strong></li>
         *   <ul>
         *     <li><code>operator[](std::uint64_t index)</code> &mdash; direct character access (no bounds checking).</li>
         *     <li><code>sub(std::uint64_t offset, std::uint64_t length = 0)</code> &mdash; create a substring view.</li>
         *     <li><code>compare(const string_view &amp;rhs)</code> &mdash; perform lexical comparison (similar to <code>strcmp</code>).</li>
         *     <li><code>starts_with(const string_view &amp;prefix)</code> &mdash; check if the view starts with the specified prefix.</li>
         *     <li><code>ends_with(const string_view &amp;suffix)</code> &mdash; check if the view ends with the specified suffix.</li>
         *     <li><code>find(char ch)</code> &mdash; locate the first occurrence of a character; returns <code>-1</code> if not found.</li>
         *     <li><code>hash(jh::meta::c_hash hash_method = jh::meta::c_hash::fnv1a64)</code> &mdash; compute a stable 64-bit hash of the contents.</li>
         *     <li><code>operator std::string_view()</code> or <code>to_std()</code> &mdash; explicit conversion to <code>std::string_view</code> for interoperability.</li>
         *     <li><code>operator&lt;=&gt;(const string_view &amp;rhs)</code> &mdash; perform lexicographical three-way comparison (returns <code>std::strong_ordering</code>).</li>
         *   </ul>
         *   <li>All string operations are <strong>constexpr</strong> and <strong>noexcept</strong>, providing zero-overhead interoperability
         *       with the standard library and compile-time evaluation where applicable.</li>
         * </ul>
         */
        inline jh::pod::string_view decode(
                const std::string &input,
                std::string &output_buffer
        ) {
            auto temp = decode(input);
            output_buffer = std::string(
                    std::make_move_iterator(temp.begin()),
                    std::make_move_iterator(temp.end())
            );
            return {output_buffer.data(), output_buffer.size()};
        }
    } // namespace base64url

} // namespace jh::serio
