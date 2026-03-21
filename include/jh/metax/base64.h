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
 * @brief Compile-time Base64 and Base64URL codec for the <code>jh::meta</code> subsystem.
 *
 * @details
 * <p>
 * This header provides a fully constexpr-capable Base64 and Base64URL codec that
 * operates entirely at compile time. Encoded text is supplied as a non-type
 * template parameter (<tt>NTTP</tt>) through the <code>TStr</code> compile-time string type.
 * <br>
 * The <code>jh::meta</code> subsystem focuses on compile-time reflection, static string
 * manipulation, and deterministic constexpr evaluation. Its Base64 facilities allow
 * encoded data to be decoded or generated during compilation, enabling static asset
 * embedding, protocol constant generation, and zero-runtime preprocessing.
 * </p>
 *
 * <h3>Design overview</h3>
 * <ul>
 *   <li>Implements <b>Base64</b> (RFC 4648 &sect;4) with padding.</li>
 *   <li>Implements <b>Base64URL</b> (RFC 4648 &sect;5) with optional padding.</li>
 *   <li>All constraints (<code>is_base64</code>, <code>is_base64url</code>) are validated at compile time.</li>
 *   <li>Decode functions return <code>jh::pod::array&lt;std::uint8_t, N&gt;</code>.</li>
 *   <li>Encode functions return <code>t_str&lt;M&gt;</code> with a built-in null terminator.</li>
 *   <li>Supports round-trip transformations between <code>t_str</code> and byte buffers.</li>
 * </ul>
 *
 * <h3>Usage</h3>
 *
 * <p>Compile-time decoding using a literal <tt>NTTP</tt>:</p>
 * @code
 * constexpr auto decoded = jh::meta::decode_base64&lt;"Qm9i"&gt;();   // "Bob"
 * @endcode
 *
 * <p>Compile-time decoding using a macro-defined literal:</p>
 * @code
 * #ifndef BASE64_STR_DEMO
 * #define BASE64_STR_DEMO "SGkh"
 * #endif
 * constexpr auto decoded = jh::meta::decode_base64&lt;BASE64_STR_DEMO&gt;();
 * // "Hi!"
 * @endcode
 *
 * <p>Full round-trip example using t_str and byte buffers:</p>
 * @code
 * // original "Hello"
 * constexpr jh::meta::t_str str{"Hello"};
 *
 * // string &rarr; bytes
 * constexpr auto bytes = str.to_bytes();
 *
 * // bytes &rarr; base64 literal
 * constexpr auto encoded = jh::meta::encode_base64(bytes);
 *
 * // base64 &rarr; bytes
 * constexpr auto decoded = jh::meta::decode_base64&lt;encoded&gt;();
 *
 * // bytes &rarr; t_str
 * constexpr auto restored =
 *     jh::meta::t_str&lt;decoded.size() + 1&gt;::from_bytes(decoded);
 * @endcode
 *
 * <h3>Subsystem role</h3>
 * <p>
 * Within the <code>jh::meta</code> subsystem, this file provides the compile-time
 * counterpart to the runtime Base64 codec used in the serialization module.
 * While the runtime interface focuses on interoperability and safety,
 * the compile-time interface focuses on deterministic evaluation, static
 * transformation of encoded resources, and <tt>NTTP</tt>-based metaprogramming.
 * </p>
 *
 * @version <pre>1.4.x</pre>
 * @date    <pre>2025</pre>
 */

#pragma once

#include <cstdint>
#include "jh/pods/array.h"
#include "jh/metax/t_str.h"
#include "jh/detail/base64_common.h"

namespace jh::meta {
    namespace detail {
        /**
         * @brief Compute the decoded byte length of a Base64 <code>TStr</code> literal.
         *
         * @tparam S
         *     A <code>TStr</code> literal that satisfies <code>S.is_base64()</code>.
         *
         * @details
         * This internal helper determines the exact number of bytes produced by
         * decoding a Base64-encoded <code>TStr</code>. The calculation depends on the
         * full Base64 structure, including the number of padding characters, and is
         * <b>not</b> a simple function of <code>S.size()</code>.
         *
         * <p>
         * The result is used by the public <code>decode_base64</code> interface to
         * instantiate a <code>jh::pod::array</code> of the correct size at compile time.
         * Since this function is part of the internal <code>detail</code> namespace,
         * it is not intended to be used directly by application code.
         * </p>
         *
         * @return
         *     The number of decoded bytes that the Base64 literal represents.
         */
        template<TStr S>
        requires (S.is_base64())
        constexpr std::uint16_t decoded_len_base64() {
            auto pad = jh::detail::base64_common::base64_check(S.val(), S.size());
            return jh::detail::base64_common::decoded_len_base64(S.size(), pad);
        }

        /**
         * @brief Compute the decoded byte length of a Base64URL <code>TStr</code> literal.
         *
         * @tparam S
         *     A <code>TStr</code> literal that satisfies <code>S.is_base64url()</code>.
         *
         * @details
         * This internal helper calculates the decoded length of Base64URL-encoded
         * text at compile time. Base64URL allows both padded and non-padded forms,
         * so the returned size depends on the complete structure of <code>S</code>,
         * including whether padding is present. As a result, the decoded size cannot
         * be inferred from <code>S.size()</code> alone.
         *
         * <p>
         * The computed value is used by <code>decode_base64url</code> to construct a
         * <code>jh::pod::array</code> of the correct size in a fully constexpr manner.
         * This function resides in the <code>detail</code> namespace and is intended
         * only as an implementation utility.
         * </p>
         *
         * @return
         *     The number of bytes produced by decoding the Base64URL literal.
         */
        template<TStr S>
        requires (S.is_base64url())
        constexpr std::uint16_t decoded_len_base64url() {
            constexpr auto n = S.size();
            constexpr int pad = jh::detail::base64_common::base64url_check(S.val(), n);
            if constexpr (pad > 0) {
                // padded Base64URL : same as padded Base64
                return jh::detail::base64_common::decoded_len_base64(n, static_cast<uint8_t>(pad));
            } else {
                // pad == 0 : unpadded Base64URL
                return jh::detail::base64_common::decoded_len_base64url_no_pad(n);
            }
        }
    } // namespace detail

    /**
     * @brief Decode a Base64-encoded <code>TStr</code> literal at compile time.
     *
     * @tparam S
     *     A <code>TStr</code> literal representing a Base64-encoded string.
     *     The template is enabled only when <code>S.is_base64()</code> is true.
     *
     * @details
     * <p>
     * This function performs fully compile-time Base64 decoding when the encoded
     * value is supplied as a non-type template parameter (NTTP). All validation is
     * performed through the <code>TStr</code> constraint system, ensuring that
     * the input literal satisfies Base64 structural rules at compile time.
     * <br>
     * The function returns a <code>jh::pod::array</code> whose size matches the
     * decoded payload length. Since the operation is constexpr-capable, the result
     * may be used in constexpr contexts, static initialization, or as further NTTP
     * input to other compile-time facilities.
     * </p>
     *
     * @return
     *     A <code>jh::pod::array&lt;std::uint8_t, N&gt;</code> containing the decoded bytes.
     *
     * @note
     *     Since decoding occurs entirely at compile time, invalid Base64 input
     *     results in a compilation error rather than a runtime error.
     */
    template<TStr S>
    requires (S.is_base64())
    constexpr auto decode_base64() {
        constexpr std::uint16_t out_len = detail::decoded_len_base64<S>();
        constexpr std::uint64_t enc_len = S.size();

        jh::pod::array<std::uint8_t, out_len> out{};

        jh::detail::base64_common::base64_decode_unchecked(
                S.val(),
                enc_len,
                out.data,
                out_len
        );

        return out;
    }

    /**
     * @brief Decode a Base64URL-encoded <code>TStr</code> literal at compile time.
     *
     * @tparam S
     *     A <code>TStr</code> literal representing Base64URL text, with or without
     *     padding. The template participates in overload resolution only when
     *     <code>S.is_base64url()</code> is true.
     *
     * @details
     * <p>
     * This function evaluates Base64URL decoding entirely during compilation.
     * Padded and non-padded Base64URL forms are supported, and the decoded output
     * is produced as a POD-safe <code>jh::pod::array</code>.
     * </p>
     *
     * <p>
     * Length rules for padded and non-padded Base64URL are applied through the
     * <code>TStr</code> constraint rather than at runtime, guaranteeing correctness
     * before code generation.
     * </p>
     *
     * @return
     *     A <code>jh::pod::array&lt;std::uint8_t, N&gt;</code> containing the decoded bytes.
     */
    template<TStr S>
    requires (S.is_base64url())
    constexpr auto decode_base64url() {
        constexpr std::uint16_t out_len = detail::decoded_len_base64url<S>();
        constexpr std::uint64_t enc_len = S.size();

        jh::pod::array<std::uint8_t, out_len> out{};

        jh::detail::base64_common::base64_decode_unchecked(
                S.val(),
                enc_len,
                out.data,
                out_len
        );

        return out;
    }

    /**
     * @brief Encode a byte buffer into a Base64 literal at compile time.
     *
     * @tparam N
     *     The size of the input byte buffer. The encoded size is determined
     *     automatically based on <code>N</code>.
     *
     * @param raw
     *     A <code>jh::pod::array&lt;std::uint8_t, N&gt;</code> representing the binary
     *     payload to encode.
     *
     * @details
     * <p>
     * The function produces padded Base64 output (that is, with trailing '='
     * characters when required). The resulting text is stored in a
     * <code>TStr&lt;M&gt;</code> compile-time string literal that includes a null
     * terminator.
     * </p>
     *
     * <p>
     * Because the output literal is returned as <code>TStr</code>, it may be used
     * directly as an NTTP in subsequent compile-time operations such as:
     * <ul>
     *   <li>constexpr decoding,</li>
     *   <li>compile-time hashing,</li>
     *   <li>static data construction,</li>
     *   <li>constexpr reflection pipelines.</li>
     * </ul>
     * </p>
     *
     * @return
     *     A <code>TStr&lt;M&gt;</code> containing the padded Base64 representation,
     *     terminated with a null character.
     */
    template<std::uint16_t N>
    constexpr auto encode_base64(const jh::pod::array<std::uint8_t, N> &raw) {
        constexpr std::uint64_t raw_len = N;
        constexpr std::uint64_t enc_len =
                jh::detail::base64_common::encoded_len_base64(raw_len);

        // total string size = encoded_length + null
        constexpr std::uint64_t s_len = enc_len + 1;

        jh::pod::array<char, s_len> out_char{};

        jh::detail::base64_common::base64_encode_unchecked<false>(
                raw.data,
                raw_len,
                out_char.data,
                true // padded
        );

        out_char.data[enc_len] = '\0';

        return TStr<s_len>(out_char);
    }

    /**
     * @brief Encode a byte buffer into a Base64URL literal at compile time.
     *
     * @tparam N
     *     The size of the input byte buffer.
     *
     * @tparam PadT
     *     A boolean tag type controlling whether padding is emitted.
     *     The caller supplies an instance of <code>std::false_type{}</code>
     *     to generate unpadded Base64URL, or <code>std::true_type{}</code>
     *     to generate padded Base64URL.
     *
     * @param raw
     *     A <code>jh::pod::array&lt;std::uint8_t, N&gt;</code> representing the input bytes.
     *
     * @param pad_tag
     *     A value of type <code>PadT</code>. The caller must pass either
     *     <code>std::false_type{}</code> (no padding) or <code>std::true_type{}</code>
     *     (padding enabled). This parameter determines the encoding behavior and
     *     enables automatic type deduction for <code>PadT</code>.
     *
     * @details
     * <p>
     * This function encodes the provided byte buffer into a Base64URL literal.
     * Unlike the standard Base64 variant, padding is optional in Base64URL.
     * The padding behavior is determined solely by the <code>pad_tag</code> parameter.
     * </p>
     *
     * <p>
     * The resulting encoded text is returned as a <code>TStr&lt;M&gt;</code> instance
     * that includes a null terminator. This enables further usage as an NTTP value
     * in downstream compile-time operations.
     * </p>
     *
     * @return
     *     A <code>TStr&lt;M&gt;</code> containing either padded or non-padded Base64URL
     *     text depending on <code>pad_tag</code>.
     *
     * @note
     *     Use the following forms:
     *     <ul>
     *       <li><code>encode_base64url(bytes, std::false_type{})</code> or
     *           default <code>encode_base64url(bytes)</code>for no padding</li>
     *       <li><code>encode_base64url(bytes, std::true_type{})</code> for padded output</li>
     *     </ul>
     */
    template<std::uint16_t N, class PadT = std::false_type>
    constexpr auto encode_base64url(const jh::pod::array<std::uint8_t, N> &raw, PadT = {}) {
        constexpr std::uint64_t raw_len = N;

        constexpr std::uint64_t enc_len_pad =
                jh::detail::base64_common::encoded_len_base64(raw_len);

        constexpr std::uint64_t enc_len_nopad =
                jh::detail::base64_common::encoded_len_base64url_no_pad(raw_len);

        constexpr std::uint64_t enc_len = PadT::value ? enc_len_pad : enc_len_nopad;
        constexpr std::uint64_t s_len = enc_len + 1;

        jh::pod::array<char, s_len> out_char{};

        jh::detail::base64_common::base64_encode_unchecked<true>(
                raw.data,
                raw_len,
                out_char.data,
                PadT::value
        );

        out_char.data[enc_len] = '\0';

        return TStr<s_len>(out_char);
    }

} // namespace jh::meta
