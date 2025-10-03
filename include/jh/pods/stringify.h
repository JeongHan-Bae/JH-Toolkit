/**
 * \verbatim
 * Copyright 2025 JeongHan-Bae <mastropseudo@gmail.com>
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
 * @file stringify.h (pods)
 * @brief Stream output adapters (<code>operator<<</code>) for POD containers and utilities.
 *
 * This header provides inline <code>operator<<</code> overloads for types in <code>jh::pod</code>,
 * producing human-readable, debug-friendly representations on <code>std::ostream</code>.
 *
 * <h3>Important Notes:</h3>
 * <ul>
 *   <li>These outputs are intended for <b>debugging, inspection, and logging</b>.</li>
 *   <li><b>They do not define a stable serialization format</b> — output may change across compiler,
 *       platform, or version differences.</li>
 *   <li>Do <b>not</b> use these printers for persistence, network protocols, or ABI-sensitive data.</li>
 *   <li>If you need true serialization:
 *       <ul>
 *         <li>Implement your own <code>operator<<</code> / <code>to_string</code> overloads.</li>
 *         <li>Or use <code>jh::utils::base64</code> helpers, which provide proper encode/decode.</li>
 *       </ul>
 *   </li>
 *   <li>All overloads are declared <code>inline</code>. In C++17+, this does <b>not</b> force inlining,
 *       but provides <b>weak linkage semantics</b>, allowing safe inclusion in multiple translation units
 *       without ODR violations, and optional overriding elsewhere.</li>
 * </ul>
 *
 * <h3>jh::utils::base64 (recommended for serialization)</h3>
 * Provides a minimal and consistent interface for encoding and decoding binary data.
 *
 * <ul>
 *   <li><code>std::string encode(const uint8_t *data, std::size_t len) noexcept;</code><br/>
 *       Encode raw bytes into a Base64 string.</li>
 *
 *   <li><code>std::vector&lt;uint8_t&gt; decode(const std::string &input);</code><br/>
 *       Decode a Base64 string into a new byte buffer.</li>
 *
 *   <li><code>jh::pod::bytes_view decode(const std::string &input, std::vector&lt;uint8_t&gt;& buffer);</code><br/>
 *       Decode into a provided <code>std::vector&lt;uint8_t&gt;</code> and return a
 *       <code>bytes_view</code> pointing into it (useful for flat binary operations).</li>
 *
 *   <li><code>jh::pod::string_view decode(const std::string &input, std::string& buffer);</code><br/>
 *       Decode into a provided <code>std::string</code> and return a
 *       <code>string_view</code> pointing into it (useful for text-like operations).</li>
 * </ul>
 *
 * These APIs are suitable for real serialization and deserialization,
 * unlike the human-readable printers in this file.
 *
 * These APIs provide predictable serialization and deserialization.
 * Use them if you need to persist or exchange data between systems.
 *
 * <h3>Examples of Debug Printers:</h3>
 * <ul>
 *   <li><code>pod::array&lt;T, N&gt;</code> → <code>[1, 2, 3]</code></li>
 *   <li><code>pod::array&lt;char, N&gt;</code> → <code>"escaped\\tstring"</code></li>
 *   <li><code>pod::pair&lt;T1, T2&gt;</code> → <code>{a, b}</code></li>
 *   <li><code>pod::optional&lt;T&gt;</code> → <code>value</code> or <code>nullopt</code></li>
 *   <li><code>pod::bitflags&lt;N&gt;</code> → <code>0x'ABCD</code> or <code>0b'0101</code></li>
 *   <li><code>pod::bytes_view</code> → <code>base64'...'</code></li>
 *   <li><code>pod::span&lt;T&gt;</code> → <code>span&lt;int&gt;[1, 2, 3]</code></li>
 *   <li><code>pod::string_view</code> → <code>string_view"hello"</code></li>
 * </ul>
 */
#pragma once

#include "pod_like.h"
#include <ostream>
#include <sstream>
#include <iomanip>
#include "../utils/typed.h"
#include "../utils/base64.h"
#include "../utils/type_name.h"
#include "pair.h"
#include "array.h"
#include "bits.h"
#include "bytes_view.h"
#include "span.h"
#include "string_view.h"
#include "optional.h"

namespace jh::pod {

    template<typename T>
    concept streamable = requires(std::ostream &os, const T &value) {
        { os << value } -> std::same_as<std::ostream &>;
    };

    template<typename T>
    concept streamable_pod =
    jh::pod::pod_like<T> &&
    streamable<T> &&
    !std::is_fundamental_v<T> &&
    !std::is_enum_v<T> &&
    !std::is_pointer_v<T>;

    template<streamable T, uint16_t N>
    requires(!std::is_same_v<T, char> && // forbid printing char arrays
             requires(std::ostream &os, T v) {
                 { os << v }; // T should be printable
             })
    inline std::ostream &operator<<(std::ostream &os, const jh::pod::array<T, N> &arr) {
        os << "[";
        for (uint16_t i = 0; i < N; ++i) {
            if (i != 0)
                os << ", ";
            os << arr[i];
        }
        os << "]";
        return os;
    }

    template<uint16_t N>
    inline std::ostream &operator<<(std::ostream &os, const jh::pod::array<char, N> &str) {
        os << '"';  // start escaped JSON string
        for (uint16_t i = 0; i < N && str[i] != '\0'; ++i) {
            char c = str[i];
            switch (c) {
                case '\"':
                    os << "\\\"";
                    break;
                case '\\':
                    os << "\\\\";
                    break;
                case '\b':
                    os << "\\b";
                    break;
                case '\f':
                    os << "\\f";
                    break;
                case '\n':
                    os << "\\n";
                    break;
                case '\r':
                    os << "\\r";
                    break;
                case '\t':
                    os << "\\t";
                    break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20 || static_cast<unsigned char>(c) > 0x7E) {
                        os << "\\u"
                           << std::hex << std::uppercase
                           << std::setw(4) << std::setfill('0')
                           << static_cast<int>(static_cast<unsigned char>(c))
                           << std::dec << std::nouppercase;
                    } else {
                        os << c;
                    }
            }
        }
        os << '"';  // end escaped JSON string
        return os;
    }

    inline std::ostream &operator<<(std::ostream &os, jh::typed::monostate &) {
        os << "null";
        return os;
    }

    template<streamable T1, streamable T2>
    inline std::ostream &operator<<(std::ostream &os, const jh::pod::pair<T1, T2> &t) {
        os << "{" << t.first << ", " << t.second << "}";
        return os;
    }

    template<streamable T>
    inline std::ostream &operator<<(std::ostream &os, const jh::pod::optional<T> &opt) {
        if (opt.has()) {
            os << opt.ref();
        } else {
            os << "nullopt";
        }
        return os;
    }

    template<std::uint16_t N>
    inline std::ostream &operator<<(std::ostream &os, const jh::pod::bitflags<N> &flags) {
        auto bytes = jh::pod::to_bytes(flags);
        std::ios_base::fmtflags fmt = os.flags();

        if ((fmt & std::ios_base::basefield) == std::ios_base::hex) {
            // hex mode
            os << "0x'";
            for (int i = bytes.size() - 1; i >= 0; --i) {
                os << std::hex << std::setw(2) << std::setfill('0')
                   << static_cast<int>(bytes[i]);
            }
        } else {
            // binary mode
            os << "0b'";
            for (int i = bytes.size() - 1; i >= 0; --i) {
                for (int b = 7; b >= 0; --b)
                    os << ((bytes[i] >> b) & 1);
            }
        }
        os << "'";

        return os;
    }

    inline std::ostream &operator<<(std::ostream &os, const jh::pod::bytes_view bv) {
        os << "base64'";
        const auto encoded = jh::utils::base64::encode(reinterpret_cast<const uint8_t *>(bv.data), bv.len);
        os << encoded;
        os << "'";
        return os;
    }

    template<streamable T>
    inline std::ostream &operator<<(std::ostream &os, const span<T> &sp) {
        os << "span<" << utils::type_name<T>() << ">[";
        for (std::uint64_t i = 0; i < sp.size(); ++i) {
            if (i != 0) os << ", ";
            os << sp[i];
        }
        os << "]";
        return os;
    }

    inline std::ostream &operator<<(std::ostream &os, const string_view &sv) {
        os << "string_view\"";
        const auto buffer = std::string_view{sv.data, sv.len};
        os << buffer;
        os << "\"";
        return os;
    }

    template<streamable_pod Pod>
    std::string to_string(const Pod& p) {
        std::ostringstream oss;
        oss << p;
        return oss.str();
    }
}

namespace jh::typed{
    inline std::ostream &operator<<(std::ostream &os, const jh::typed::monostate &) {
        os << "null";
        return os;
    }
}