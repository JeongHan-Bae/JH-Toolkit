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
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.<br>
 * See the License for the specific language governing permissions and<br>
 * limitations under the License.<br>
 * <br>
 * Full license: <a href="https://github.com/JeongHan-Bae/JH-Toolkit?tab=Apache-2.0-1-ov-file#readme">GitHub</a>
 */
/**
 * @file scalar.hpp
 * @brief Cucumber step parameter conversion helpers.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 */

#pragma once

#include <charconv>
#include <cstdint>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>

#include "jh/metax/expected.h"
#include "jh/pods/string_view.h"

namespace jh::test::cucumber {
    /// @brief Describes why conversion from captured step text failed.
    enum class conversion_error {
        /// @brief The text is empty, malformed, or contains unconsumed characters.
        invalid_syntax,
        /// @brief The numeric value cannot be represented by the requested type.
        out_of_range
    };

    /**
     * @brief Restricts supported scalar conversions to the Cucumber binding types.
     * @tparam T Candidate scalar type.
     */
    template<typename T>
    concept scalar_type =
        std::is_same_v<T, std::string_view> ||
        std::is_same_v<T, std::string> ||
        std::is_same_v<T, std::int64_t> ||
        std::is_same_v<T, std::uint64_t> ||
        std::is_same_v<T, double> ||
        std::is_same_v<T, bool>;

    /**
     * @brief Copies captured text into an owned string.
     * @param value Text to copy.
     * @return An owning string containing @p value.
     */
    [[nodiscard]] inline std::string asString(const std::string_view value) {
        if (value.empty()) return {};
        return std::string{value};
    }

    /**
     * @brief Converts complete base-10 text to a signed 64-bit integer.
     * @param value Text to convert.
     * @return The converted value, or a syntax or range error.
     */
    [[nodiscard]] inline jh::meta::expected<std::int64_t, conversion_error>
    asInt(const std::string_view value) noexcept {
        if (value.empty()) {
            return jh::meta::unexpected(conversion_error::invalid_syntax);
        }
        std::int64_t result{};
        const auto [end, error] = std::from_chars(
            value.data(),
            value.data() + value.size(),
            result,
            10
        );
        if (error == std::errc::result_out_of_range) {
            return jh::meta::unexpected(conversion_error::out_of_range);
        }
        if (error != std::errc{} || end != value.data() + value.size()) {
            return jh::meta::unexpected(conversion_error::invalid_syntax);
        }
        return result;
    }

    /**
     * @brief Converts complete base-10 text to an unsigned 64-bit integer.
     * @param value Text to convert.
     * @return The converted value, or a syntax or range error.
     */
    [[nodiscard]] inline jh::meta::expected<std::uint64_t, conversion_error>
    asUint(const std::string_view value) noexcept {
        if (value.empty()) {
            return jh::meta::unexpected(conversion_error::invalid_syntax);
        }
        std::uint64_t result{};
        const auto [end, error] = std::from_chars(
            value.data(),
            value.data() + value.size(),
            result,
            10
        );
        if (error == std::errc::result_out_of_range) {
            return jh::meta::unexpected(conversion_error::out_of_range);
        }
        if (error != std::errc{} || end != value.data() + value.size()) {
            return jh::meta::unexpected(conversion_error::invalid_syntax);
        }
        return result;
    }

    /**
     * @brief Converts complete general-format numeric text to a double.
     * @param value Text to convert.
     * @return The converted value, or a syntax or range error.
     */
    [[nodiscard]] inline jh::meta::expected<double, conversion_error>
    asDouble(const std::string_view value) noexcept {
        if (value.empty()) {
            return jh::meta::unexpected(conversion_error::invalid_syntax);
        }
        double result{};
        const auto [end, error] = std::from_chars(
            value.data(),
            value.data() + value.size(),
            result,
            std::chars_format::general
        );
        if (error == std::errc::result_out_of_range) {
            return jh::meta::unexpected(conversion_error::out_of_range);
        }
        if (error != std::errc{} || end != value.data() + value.size()) {
            return jh::meta::unexpected(conversion_error::invalid_syntax);
        }
        return result;
    }

    /**
     * @brief Converts the exact text <code>true</code> or <code>false</code> to a boolean.
     * @param value Text to convert.
     * @return The converted value, or <code>conversion_error::invalid_syntax</code> for any other text.
     */
    [[nodiscard]] inline jh::meta::expected<bool, conversion_error>
    asBool(const std::string_view value) noexcept {
        if (value == "true") return true;
        if (value == "false") return false;
        return jh::meta::unexpected(conversion_error::invalid_syntax);
    }

    /**
     * @brief Converts captured text to one of the supported scalar types.
     * @tparam T Requested scalar type satisfying <code>scalar_type</code>.
     * @param value Text to convert.
     * @return The converted value, or the conversion error for numeric and boolean types.
     * A string view result refers to @p value; a string result owns a copy.
     */
    template<scalar_type T>
    [[nodiscard]] inline jh::meta::expected<T, conversion_error>
    as(const std::string_view value) {
        if constexpr (std::is_same_v<T, std::string_view>) {
            return value;
        } else if constexpr (std::is_same_v<T, std::string>) {
            return asString(value);
        } else if constexpr (std::is_same_v<T, std::int64_t>) {
            return asInt(value);
        } else if constexpr (std::is_same_v<T, std::uint64_t>) {
            return asUint(value);
        } else if constexpr (std::is_same_v<T, double>) {
            return asDouble(value);
        } else if constexpr (std::is_same_v<T, bool>) {
            return asBool(value);
        }
    }

    namespace detail {
        template<scalar_type T>
        [[nodiscard]] inline bool scalar_value_is_valid(const std::string_view value) noexcept {
            if constexpr (
                std::is_same_v<T, std::string_view> ||
                std::is_same_v<T, std::string>
            ) {
                return true;
            } else if constexpr (std::is_same_v<T, double>) {
                const jh::pod::string_view pod_value{value.data(), value.size()};
                return pod_value.is_number() && asDouble(value).has_value();
            } else {
                return as<T>(value).has_value();
            }
        }
    }

    /**
     * @brief Returns the Cucumber placeholder name corresponding to a scalar type.
     * @tparam T Supported scalar type.
     * @return Placeholder name such as <code>&lt;int&gt;</code>.
     */
    template<scalar_type T>
    [[nodiscard]] constexpr std::string_view scalar_type_name() noexcept {
        if constexpr (std::is_same_v<T, std::string_view>) return "<string>";
        if constexpr (std::is_same_v<T, std::string>) return "<string>";
        if constexpr (std::is_same_v<T, std::int64_t>) return "<int>";
        if constexpr (std::is_same_v<T, std::uint64_t>) return "<uint>";
        if constexpr (std::is_same_v<T, double>) return "<double>";
        if constexpr (std::is_same_v<T, bool>) return "<bool>";
    }
}
