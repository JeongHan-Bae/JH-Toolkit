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
 * @file expression.hpp
 * @brief Compile-time Cucumber step expression parsing and matching.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 */

#pragma once

#include <array>
#include <cstddef>
#include <string_view>

#include "jh/metax/t_str.h"
#include "jh/metax/expected.h"

namespace jh::test::cucumber {
    /// @brief Identifies the value type accepted by a step-expression placeholder.
    enum class ParameterType {
        /// @brief Captures text as a string view.
        string,
        /// @brief Captures a signed decimal integer.
        integer,
        /// @brief Captures an unsigned decimal integer.
        unsigned_integer,
        /// @brief Captures a floating-point number.
        real,
        /// @brief Captures the text <code>true</code> or <code>false</code>.
        boolean
    };

    /// @brief Distinguishes literal text from a typed placeholder in an expression.
    enum class ExpressionTokenKind {
        /// @brief A fixed portion of the expression.
        literal,
        /// @brief A typed placeholder whose text is captured.
        parameter
    };

    /// @brief Describes one literal or placeholder within a step expression.
    struct ExpressionToken {
        /// @brief Token category.
        ExpressionTokenKind kind{};
        /// @brief Zero-based byte offset of the literal text or placeholder name in the expression.
        std::size_t offset{};
        /// @brief Number of bytes occupied by the literal text or placeholder name.
        std::size_t length{};
        /// @brief Placeholder value type; meaningful for parameter tokens.
        ParameterType parameter{};
    };

    /// @brief Reports why an expression did not match input text.
    enum class expression_error {
        /// @brief The input text does not match the complete expression.
        no_match
    };

    /**
     * @brief Stores the parsed tokens and placeholders of a compile-time expression.
     * @tparam Capacity Maximum number of tokens and parameters stored in the specification.
     */
    template<std::size_t Capacity>
    struct ExpressionSpec {
        /// @brief Parsed literal and placeholder tokens in expression order.
        std::array<ExpressionToken, Capacity> tokens{};
        /// @brief Placeholder types in capture order.
        std::array<ParameterType, Capacity> parameters{};
        /// @brief Number of populated entries in <code>tokens</code>.
        std::size_t token_count{};
        /// @brief Number of populated entries in <code>parameters</code>.
        std::size_t parameter_count{};
        /// @brief Whether the expression contains a valid token sequence.
        bool valid{};
    };

    namespace detail {
        constexpr ParameterType parse_parameter_type(const std::string_view name) noexcept {
            if (name == "string") return ParameterType::string;
            if (name == "int") return ParameterType::integer;
            if (name == "uint") return ParameterType::unsigned_integer;
            if (name == "double") return ParameterType::real;
            return ParameterType::boolean;
        }

        constexpr bool is_parameter_type(const std::string_view name) noexcept {
            return name == "string" || name == "int" || name == "uint" ||
                   name == "double" || name == "bool";
        }

        template<std::size_t Capacity>
        constexpr void append_token(
            ExpressionSpec<Capacity>& spec,
            const ExpressionToken token
        ) noexcept {
            spec.tokens[spec.token_count++] = token;
            if (token.kind == ExpressionTokenKind::parameter) {
                spec.parameters[spec.parameter_count++] = token.parameter;
            }
        }

        template<jh::meta::TStr Expression>
        consteval auto parse_expression() {
            constexpr auto text = Expression.view();
            ExpressionSpec<Expression.size() + 1> spec{};

            if (text.empty()) return spec;

            std::size_t cursor = 0;
            std::size_t literal_start = 0;
            bool previous_was_parameter = false;

            while (cursor < text.size()) {
                const auto ch = text[cursor];
                if (ch == '>') return spec;
                if (ch != '<') {
                    ++cursor;
                    continue;
                }

                if (cursor > literal_start) {
                    append_token(spec, {
                        ExpressionTokenKind::literal,
                        literal_start,
                        cursor - literal_start,
                        ParameterType::string
                    });
                    previous_was_parameter = false;
                } else if (previous_was_parameter) {
                    return spec;
                }

                const auto close = text.find('>', cursor + 1);
                if (close == std::string_view::npos) return spec;

                const auto name = text.substr(cursor + 1, close - cursor - 1);
                if (!is_parameter_type(name)) return spec;

                append_token(spec, {
                    ExpressionTokenKind::parameter,
                    cursor + 1,
                    name.size(),
                    parse_parameter_type(name)
                });
                previous_was_parameter = true;
                cursor = close + 1;
                literal_start = cursor;
            }

            if (literal_start < text.size()) {
                append_token(spec, {
                    ExpressionTokenKind::literal,
                    literal_start,
                    text.size() - literal_start,
                    ParameterType::string
                });
            } else if (spec.token_count == 0) {
                append_token(spec, {
                    ExpressionTokenKind::literal,
                    0,
                    text.size(),
                    ParameterType::string
                });
            }

            spec.valid = spec.token_count != 0;
            return spec;
        }

        template<jh::meta::TStr Expression>
        inline constexpr auto expression_spec = parse_expression<Expression>();
    }

    /**
     * @brief Parses a step expression during constant evaluation.
     * @tparam Expression Compile-time expression containing literal text and supported placeholders.
     * Supported placeholders are <code>&lt;string&gt;</code>, <code>&lt;int&gt;</code>,
     * <code>&lt;uint&gt;</code>, <code>&lt;double&gt;</code>, and <code>&lt;bool&gt;</code>.
     * Invalid or empty expressions produce a specification whose <code>valid</code> member is false.
     * @return Parsed expression specification.
     */
    template<jh::meta::TStr Expression>
    [[nodiscard]] consteval auto parseExpression() {
        return detail::parse_expression<Expression>();
    }

    /**
     * @brief Matches input text against a compile-time step expression.
     * @tparam Expression Compile-time expression with supported typed placeholders.
     * @param text Complete step text to match.
     * @return Captured placeholder text in expression order, or <code>expression_error::no_match</code>.
     * Returned string views refer to @p text and do not own its storage.
     */
    template<jh::meta::TStr Expression>
    [[nodiscard]] constexpr jh::meta::expected<
        std::array<std::string_view, detail::expression_spec<Expression>.parameter_count>,
        expression_error
    > match_expression(const std::string_view text) noexcept {
        constexpr auto spec = detail::expression_spec<Expression>;
        static_assert(spec.valid, "Invalid Cucumber step expression.");

        using captures_type = std::array<std::string_view, spec.parameter_count>;
        captures_type captures{};

        std::size_t pattern_index = 0;
        std::size_t text_offset = 0;
        std::size_t capture_index = 0;

        if (spec.tokens[0].kind == ExpressionTokenKind::literal) {
            const auto& token = spec.tokens[0];
            const auto literal = Expression.view().substr(token.offset, token.length);
            if (!text.starts_with(literal)) {
                return jh::meta::unexpected(expression_error::no_match);
            }
            text_offset = literal.size();
            ++pattern_index;

            if (pattern_index == spec.token_count) {
                if (text_offset != text.size() || capture_index != captures.size()) {
                    return jh::meta::unexpected(expression_error::no_match);
                }
                return captures;
            }
        }

        while (pattern_index < spec.token_count) {
            const auto& parameter = spec.tokens[pattern_index];
            if (parameter.kind != ExpressionTokenKind::parameter) {
                return jh::meta::unexpected(expression_error::no_match);
            }

            if (pattern_index + 1 == spec.token_count) {
                captures[capture_index] = text.substr(text_offset);
                ++capture_index;
                text_offset = text.size();
                ++pattern_index;
                break;
            }

            const auto& delimiter = spec.tokens[pattern_index + 1];
            const auto literal = Expression.view().substr(delimiter.offset, delimiter.length);

            if (pattern_index + 2 == spec.token_count) {
                if (text.size() < text_offset + literal.size()) {
                    return jh::meta::unexpected(expression_error::no_match);
                }

                const auto delimiter_offset = text.size() - literal.size();
                if (delimiter_offset < text_offset ||
                    text.substr(delimiter_offset) != literal) {
                    return jh::meta::unexpected(expression_error::no_match);
                }

                captures[capture_index] = text.substr(
                    text_offset,
                    delimiter_offset - text_offset
                );
                ++capture_index;
                text_offset = text.size();
                pattern_index += 2;
                break;
            }

            const auto delimiter_offset = text.find(literal, text_offset);
            if (delimiter_offset == std::string_view::npos) {
                return jh::meta::unexpected(expression_error::no_match);
            }

            captures[capture_index] = text.substr(
                text_offset,
                delimiter_offset - text_offset
            );
            ++capture_index;
            text_offset = delimiter_offset + literal.size();
            pattern_index += 2;
        }

        if (pattern_index != spec.token_count ||
            text_offset != text.size() ||
            capture_index != captures.size()) {
            return jh::meta::unexpected(expression_error::no_match);
        }
        return captures;
    }
}
