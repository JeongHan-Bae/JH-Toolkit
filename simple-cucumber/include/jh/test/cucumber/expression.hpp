#pragma once

#include <array>
#include <cstddef>
#include <string_view>

#include "jh/metax/t_str.h"
#include "jh/metax/expected.h"

namespace jh::test::cucumber {
    enum class ParameterType {
        string,
        integer,
        unsigned_integer,
        real,
        boolean
    };

    enum class ExpressionTokenKind {
        literal,
        parameter
    };

    struct ExpressionToken {
        ExpressionTokenKind kind{};
        std::size_t offset{};
        std::size_t length{};
        ParameterType parameter{};
    };

    enum class expression_error {
        no_match
    };

    template<std::size_t Capacity>
    struct ExpressionSpec {
        std::array<ExpressionToken, Capacity> tokens{};
        std::array<ParameterType, Capacity> parameters{};
        std::size_t token_count{};
        std::size_t parameter_count{};
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

    template<jh::meta::TStr Expression>
    [[nodiscard]] consteval auto parseExpression() {
        return detail::parse_expression<Expression>();
    }

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
