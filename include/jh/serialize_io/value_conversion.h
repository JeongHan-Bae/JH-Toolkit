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
 * @file value_conversion.h
 * @brief Converts serializable values to and from strings.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 */

#pragma once

#include "jh/conceptual/serializable.h"

#include <istream>
#include <sstream>
#include <streambuf>
#include <stdexcept>
#include <string>
#include <string_view>

namespace jh::serio {

    namespace detail {
        /// @brief Read-only stream buffer borrowing a caller-owned string.
        class string_input_buffer final : public std::streambuf {
            char empty_ = '\0';

        public:
            explicit string_input_buffer(std::string_view text) {
                // setg requires char pointers; this buffer exposes no put area.
                char *first = text.empty() ? &empty_ : const_cast<char *>(text.data());
                setg(first, first, first + text.size());
            }
        };
    }

    /**
     * @brief Converts a value to string through the String or output Stream protocol.
     * @tparam T A serializable type.
     * @param value Value to convert.
     * @return String representation of the value.
     */
    template<jh::concepts::serializable T>
    [[nodiscard]] std::string serialize(const T &value) {
        if constexpr (jh::concepts::detail::string_roundtrip<T>) {
            return static_cast<std::string>(value);
        } else {
            std::ostringstream out;
            out << value;
            if (!out) throw std::runtime_error("value stream serialization failed");
            return out.str();
        }
    }

    /**
     * @brief Constructs a value from string through the String or input Stream protocol.
     * @tparam T A serializable type that can be returned by value.
     * @param text String representation of the value.
     * @return Reconstructed value.
     * A string-protocol result may borrow <code>text</code>; the caller must
     * keep the input alive with stable character storage while using such a
     * result. The reference parameter does not extend the input's lifetime
     * after this call. Rvalue string inputs are rejected for every result
     * type, so a caller supplies named backing storage. Borrowing inside
     * user-defined types still requires that storage to outlive the result's
     * use. A stream-protocol result
     * must remain valid after the local input stream is destroyed.
     */
    template<jh::concepts::serializable T>
    requires (jh::concepts::detail::string_roundtrip<T> || std::move_constructible<T>)
    [[nodiscard]] T deserialize(const std::string &text) {
        if constexpr (jh::concepts::detail::string_roundtrip<T>) {
            return T{text};
        } else {
            detail::string_input_buffer buffer{text};
            std::istream in{&buffer};
            T value{};
            if (!(in >> value)) throw std::runtime_error("value stream deserialization failed");
            in >> std::ws;
            if (!in.eof()) throw std::runtime_error("value stream has trailing data");
            return value;
        }
    }

    /**
     * @brief Extracts a stream-protocol value into an existing object.
     * @tparam T A type with a complete Stream protocol and no String protocol.
     * @param text String representation of the value.
     * @param value Destination object; extraction may modify it before failure.
     */
    template<typename T>
    requires (jh::concepts::detail::stream_roundtrip<T> &&
              !jh::concepts::detail::string_roundtrip<T>)
    void deserialize(const std::string &text, T &value) {
        detail::string_input_buffer buffer{text};
        std::istream in{&buffer};
        if (!(in >> value)) throw std::runtime_error("value stream deserialization failed");
        in >> std::ws;
        if (!in.eof()) throw std::runtime_error("value stream has trailing data");
    }

    /// @brief Rejects a temporary string as deserialization backing storage.
    template<jh::concepts::serializable T>
    T deserialize(std::string &&) = delete;

    /// @brief Rejects a const temporary string as deserialization backing storage.
    template<jh::concepts::serializable T>
    T deserialize(const std::string &&) = delete;

    /// @brief Rejects temporary backing storage for Stream extraction into an object.
    template<typename T>
    requires (jh::concepts::detail::stream_roundtrip<T> &&
              !jh::concepts::detail::string_roundtrip<T>)
    void deserialize(std::string &&, T &) = delete;

    /// @brief Rejects const temporary backing storage for Stream extraction.
    template<typename T>
    requires (jh::concepts::detail::stream_roundtrip<T> &&
              !jh::concepts::detail::string_roundtrip<T>)
    void deserialize(const std::string &&, T &) = delete;

} // namespace jh::serio
