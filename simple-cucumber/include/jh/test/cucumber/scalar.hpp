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
    enum class conversion_error {
        invalid_syntax,
        out_of_range
    };

    template<typename T>
    concept scalar_type =
        std::is_same_v<T, std::string_view> ||
        std::is_same_v<T, std::string> ||
        std::is_same_v<T, std::int64_t> ||
        std::is_same_v<T, std::uint64_t> ||
        std::is_same_v<T, double> ||
        std::is_same_v<T, bool>;

    [[nodiscard]] inline std::string asString(const std::string_view value) {
        if (value.empty()) return {};
        return std::string{value};
    }

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

    [[nodiscard]] inline jh::meta::expected<bool, conversion_error>
    asBool(const std::string_view value) noexcept {
        if (value == "true") return true;
        if (value == "false") return false;
        return jh::meta::unexpected(conversion_error::invalid_syntax);
    }

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
