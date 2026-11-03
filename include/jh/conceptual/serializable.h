/**
 * @copyright
 * Copyright 2025 JeongHan-Bae &lt;mastropseudo\@gmail.com&gt;
 * <br>
 * Licensed under the Apache License, Version 2.0 (the "License"); <br>
 * you may not use this file except in compliance with the License.<br>
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
 * @file serializable.h
 * @brief Detects complete string or stream round-trip protocols.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 */

#pragma once

#include <concepts>
#include <istream>
#include <ostream>
#include <string>

namespace jh::concepts {

    namespace detail {
        template<typename T>
        concept string_roundtrip = std::constructible_from<T, const std::string &> &&
            requires(const T &value) {
                { static_cast<std::string>(value) } -> std::same_as<std::string>;
            };

        template<typename T>
        concept stream_roundtrip = std::default_initializable<T> &&
            requires(const T &value, T &target, std::ostream &out, std::istream &in) {
                { out << value } -> std::same_as<std::ostream &>;
                { in >> target } -> std::same_as<std::istream &>;
            };
    } // namespace detail

    /**
     * @brief Type supporting a complete string or stream serialization round trip.
     *
     * A string round trip converts a value to <code>std::string</code> and
     * constructs a value from <code>const std::string&amp;</code>. A stream round
     * trip writes to <code>std::ostream</code> and extracts into a default
     * constructed value from <code>std::istream</code>. Deserialization starts
     * with a <code>std::string</code>; the Stream path creates an input stream
     * from that text and consumes the stream. When both exist, the string
     * protocol has priority. A reconstructed value that borrows the
     * input string requires that string to outlive the value's use. A stream
     * extraction result must remain valid after the temporary input stream
     * used by <code>jh::serio::deserialize</code> is destroyed. This concept
     * checks expressions and cannot detect borrowing inside a user-defined
     * type.
     */
    template<typename T>
    concept serializable = detail::string_roundtrip<T> || detail::stream_roundtrip<T>;

} // namespace jh::concepts
