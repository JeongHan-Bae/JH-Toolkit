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
 * @file runtime.hpp
 * @brief Runtime types for parsed Gherkin features and step assertions.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 */

#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "jh/test/cucumber/table.hpp"

namespace jh::test::cucumber {
    /// @brief Identifies the semantic role of a Gherkin step.
    enum class StepKind {
        /// @brief Describes context or a precondition.
        given,
        /// @brief Describes an action.
        when,
        /// @brief Describes an expected outcome.
        then,
        /// @brief Indicates that the step role is unavailable.
        unknown
    };

    /// @brief Stores the source line and column for a parsed item.
    struct SourceLocation {
        /// @brief One-based source line, or zero when unavailable.
        std::size_t line{};
        /// @brief One-based source column, or zero when unavailable.
        std::size_t column{};
    };

    /// @brief Stores a parsed step and its optional DataTable attachment.
    struct Step {
        /// @brief Semantic role inherited from the Gherkin step keyword.
        StepKind kind{StepKind::unknown};
        /// @brief Text matched against a step definition.
        std::string text;
        /// @brief DataTable attached to this step, when present.
        std::optional<DataTable> table;
        /// @brief Source location of this step.
        SourceLocation location{};
    };

    /// @brief Stores one runnable scenario and its parsed steps.
    struct Scenario {
        /// @brief Scenario name, including any outline example values.
        std::string name;
        /// @brief Steps in execution order.
        std::vector<Step> steps;
        /// @brief Source location of this scenario.
        SourceLocation location{};
    };

    /// @brief Stores the name, runnable scenarios, and source URI of a feature.
    struct Feature {
        /// @brief Feature name.
        std::string name;
        /// @brief Runnable scenarios produced from the feature.
        std::vector<Scenario> scenarios;
        /// @brief URI or path used to identify the feature source.
        std::string uri;
    };

    /// @brief Collects assertion failures reported by a step definition.
    class StepContext final {
        std::vector<std::string> failures_;

    public:
        /**
         * @brief Checks a condition and records a failure when it is false.
         * @param condition Condition to check.
         * @param message Failure message, or an empty view to use a default message.
         * @return The checked condition.
         */
        bool expect(bool condition, std::string_view message = {}) {
            if (!condition) fail(message.empty() ? "expectation failed" : message);
            return condition;
        }

        /**
         * @brief Records a failure for the current step.
         * @param message Description of the failure.
         */
        void fail(std::string_view message) {
            failures_.emplace_back(message);
        }

        /**
         * @brief Reports whether this context contains any failures.
         * @return True when at least one failure has been recorded.
         */
        [[nodiscard]] bool failed() const noexcept {
            return !failures_.empty();
        }

        /**
         * @brief Returns a read-only view of the recorded failure messages.
         * @return Failure messages in the order they were recorded.
         */
        [[nodiscard]] std::span<const std::string> failures() const noexcept {
            return failures_;
        }
    };
}
