#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "jh/test/cucumber/table.hpp"

namespace jh::test::cucumber {
    enum class StepKind {
        given,
        when,
        then,
        unknown
    };

    struct SourceLocation {
        std::size_t line{};
        std::size_t column{};
    };

    struct Step {
        StepKind kind{StepKind::unknown};
        std::string text;
        std::optional<DataTable> table;
        SourceLocation location{};
    };

    struct Scenario {
        std::string name;
        std::vector<Step> steps;
        SourceLocation location{};
    };

    struct Feature {
        std::string name;
        std::vector<Scenario> scenarios;
        std::string uri;
    };

    class StepContext final {
        std::vector<std::string> failures_;

    public:
        bool expect(bool condition, std::string_view message = {}) {
            if (!condition) fail(message.empty() ? "expectation failed" : message);
            return condition;
        }

        void fail(std::string_view message) {
            failures_.emplace_back(message);
        }

        [[nodiscard]] bool failed() const noexcept {
            return !failures_.empty();
        }

        [[nodiscard]] std::span<const std::string> failures() const noexcept {
            return failures_;
        }
    };
}
