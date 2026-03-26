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
 * @file uri.h
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 * @brief High-level URI percent-encoding and decoding interface for the JH Toolkit.
 *
 * <p>
 * This header provides a modern, safe, and portable implementation of
 * <b>URI percent-encoding</b> and <b>percent-decoding</b>.
 * It belongs to the <b>JH Toolkit Serialization I/O module</b>
 * (<code>jh::serio</code>) and provides text-safe encoding utilities
 * for URI and URL components.
 * </p>
 *
 * <h3>Design overview</h3>
 * <ul>
 *   <li>Implements URI percent-encoding according to <b>RFC 3986</b>.</li>
 *   <li>Provides both <b>unchecked</b> and <b>validated</b> encoding/decoding interfaces.</li>
 *   <li>Supports fast encoding using precomputed character classification.</li>
 *   <li>Validation-aware APIs ensure UTF-8 correctness and prevent control characters.</li>
 *   <li>Fully portable and suitable for network, HTTP, and web serialization.</li>
 * </ul>
 *
 * @note
 * Unlike the <tt>base64</tt> module, although the <tt>uri</tt> module could theoretically
 * support constexpr at the implementation level, it does not provide a compile-time solution
 * like <code>jh::meta::base64</code>.
 * <br>
 * This is because, in practical use, <code>jh::meta::base64</code> enables embedding NTTP strings
 * via macros and parsing them into binary resources.
 * <br>
 * However, URI processing does not require this capability. The data handled by URI is
 * semantically UTF-8 strings, which should only be utilized within the runtime
 * serialization module <code>jh::serio</code>.
 * <p></p>
 * @note
 * This header participates in the <b>Dual-Mode Header</b> system of the
 * JH Toolkit.
 * <ul>
 *   <li>Linked through <b>jh::jh-toolkit</b> &mdash; the module behaves
 *       as a <b>header-only</b> implementation compiled in user
 *       translation units.</li>
 *   <li>Linked through <b>jh::jh-toolkit-static</b> &mdash; the module
 *       uses a <b>precompiled implementation</b> built with aggressive
 *       optimization (typically <code>-O3</code>).</li>
 * </ul>
 * This design allows fast incremental builds while preserving the
 * option of using a fully optimized static implementation.
 *
 * @version <pre>1.4.1</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <string>
#include <vector>
#include <stdexcept>


/**
 * @brief Implements URI percent-encoding and decoding utilities.
 *
 * <p>
 * URI percent-encoding converts unsafe characters into a textual
 * representation using the <code>\%XX</code> hexadecimal format.
 * This ensures that arbitrary data can be safely embedded within
 * URI strings and transported through text-based protocols such
 * as HTTP, REST APIs, or web forms.
 * </p>
 *
 * <p>
 * This namespace provides high-level wrappers built on top of the
 * optimized primitives in <code>jh::detail::uri_common</code>.
 * </p>
 *
 * <ul>
 *   <li><b>Basic APIs</b> assume the user passes any arbitrary string,
 *       allowing non-UTF-8 encoding or arbitrary binary data.</li>
 *   <li><b>Safe APIs</b> assume that user input is always UTF-8 semantics
 *       and throw errors when encountering control characters or malformed
 *       UTF-8 concatenations.</li>
 * </ul>
 *
 * Whether to use the unchecked version has no impact on performance.
 * The additional checks in the <code>*_safe</code> functions are nearly
 * equivalent to traversing a <code>string_view</code>.
 *
 * @note Although we support encoding and decoding for data representing arbitrary
 *       binary and its precent-encoding, for this particular semantics, we
 *       recommend base64 encoding from <code>jh::serio::base64</code> and
 *       <code>jh::serio::base64url</code>.
 *       URI percent-encoding is primarily intended for encoding textual data.
 * @see jh::serio::base64
 * @see jh::serio::base64url
 */
namespace jh::serio::uri {

    /**
     * @brief Encode a string into URI percent-encoded form.
     *
     * <p>
     * Characters that are not part of the unreserved URI character set
     * are converted into their percent-encoded representation
     * (<code>\%XX</code>).
     * </p>
     *
     * <p>
     * This function performs <b>no input validation</b> and assumes that
     * the input string is already legal UTF-8.
     * </p>
     *
     * @param input Input string to encode.
     * @return A percent-encoded URI string.
     *
     * @note
     * <ul>
     *   <li>This function assumes the source intentionally permits non-UTF-8 encoding.</li>
     *   <li>Use <code>encode_safe()</code> when input validation is required.</li>
     * </ul>
     */
    [[nodiscard]] std::string encode(const std::string_view &input);

    /**
     * @brief Decode a percent-encoded URI string.
     *
     * <p>
     * This function converts percent-encoded sequences (<code>\%XX</code>)
     * back into their original byte representation.
     * </p>
     *
     * <p>
     * The input is validated to ensure that all percent-encoded
     * sequences are syntactically correct.
     * </p>
     *
     * @param input Percent-encoded URI string.
     * @return Decoded URI string.
     *
     * @throw std::runtime_error
     * Thrown if the input contains malformed percent-encoding.
     */
    [[nodiscard]] std::string decode(const std::string_view &input);

    /**
     * @brief Encode a string into URI percent-encoded form with legality validation.
     *
     * <p>
     * This variant verifies that the input string represents a
     * <b>legal textual payload</b> before performing the encoding.
     * </p>
     *
     * <ul>
     *   <li>The input must be valid UTF-8.</li>
     *   <li>No control characters are permitted.</li>
     * </ul>
     *
     * @param input Input string to encode.
     * @return A percent-encoded URI string.
     *
     * @throw std::runtime_error
     * Thrown if the input string contains invalid UTF-8 or control characters.
     *
     * @note
     * Use this function when you want to ensure that the input is a well-formed UTF-8 string.
     * <ul>
     *   <li>This function provides a stronger safety guarantee than
     *       <code>encode()</code>.</li>
     *   <li>Recommended for external or untrusted input.</li>
     * </ul>
     */
    [[nodiscard]] std::string encode_safe(const std::string_view &input);

    /**
     * @brief Decode a percent-encoded URI string with output validation.
     *
     * <p>
     * This function first performs percent-decoding and then verifies
     * that the resulting string is a <b>legal textual representation</b>.
     * </p>
     *
     * <ul>
     *   <li>The decoded output must be valid UTF-8.</li>
     *   <li>No control characters are permitted.</li>
     * </ul>
     *
     * @param input Percent-encoded URI string.
     * @return Decoded URI string.
     *
     * @throw std::runtime_error
     * If the input contains malformed percent-encoding.
     * <br>
     * Or if the decoded result is not a legal UTF-8 string.
     *
     * @note
     * This function should be used when decoding data from
     * untrusted sources such as URLs, HTTP parameters, or
     * user input.
     */
    [[nodiscard]] std::string decode_safe(const std::string_view &input);

} // namespace jh::serio::uri

#include "jh/macros/header_begin.h"

#if JH_INTERNAL_SHOULD_DEFINE

#include "jh/pods/string_view.h"
#include "jh/detail/uri_common.h"
#include "jh/metax/char.h"

namespace jh::serio::uri {


    [[nodiscard]] JH_INLINE std::string encode(const std::string_view &input) {

        std::uint64_t encoded_len =
                jh::detail::uri_common::calculate_encoded_length(input.data(), input.size());

        std::vector<std::uint8_t> buffer(encoded_len);

        jh::detail::uri_common::uri_encode_unchecked(
                input.data(),
                input.size(),
                buffer.data(),
                encoded_len
        );

        return {buffer.begin(), buffer.end()};
    }

    [[nodiscard]] JH_INLINE std::string decode(const std::string_view &input) {

        std::uint64_t decoded_len =
                jh::detail::uri_common::calculate_decoded_length(input.data(), input.size());

        if (decoded_len == static_cast<std::uint64_t>(-1))
            throw std::runtime_error(
                    "Invalid URI: contains invalid percent-encoding."
            );

        std::vector<std::uint8_t> buffer(decoded_len);

        jh::detail::uri_common::uri_decode_unchecked(
                input.data(),
                input.size(),
                buffer.data(),
                decoded_len
        );

        return {buffer.begin(), buffer.end()};
    }

    [[nodiscard]] JH_INLINE std::string encode_safe(const std::string_view &input) {

        if (!jh::pod::string_view{input.data(), input.size()}.is_legal())
            throw std::runtime_error(
                    "Invalid input: non-UTF-8 or contains control characters."
            );

        return encode(input);
    }

    [[nodiscard]] JH_INLINE std::string decode_safe(const std::string_view &input) {

        auto output = decode(input);

        if (!jh::pod::string_view{output.data(), output.size()}.is_legal())
            throw std::runtime_error(
                    "Decoded output is not legal: non-UTF-8 or contains control characters."
            );

        return output;
    }

} // namespace jh::serio::uri

#endif // JH_INTERNAL_SHOULD_DEFINE

#include "jh/macros/header_end.h"
