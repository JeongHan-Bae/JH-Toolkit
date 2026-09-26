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
 * @file runner.hpp
 * @brief Cucumber feature execution and result reporting.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 */

#pragma once

#include <algorithm>
#include <filesystem>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "jh/test/cucumber/definition.hpp"
#include "jh/test/cucumber/gherkin.hpp"

namespace jh::test::cucumber {
    /// @brief Classifies a failure recorded for a feature or scenario step.
    enum class FailureKind {
        /// @brief Feature parsing failed.
        parse_error,
        /// @brief No step definition matched a scenario step.
        undefined_step,
        /// @brief A matched step failed validation or an assertion.
        step_failure
    };

    /// @brief Describes why feature execution could not select a unique definition.
    enum class run_error {
        /// @brief Multiple step definitions accept the same step.
        ambiguous_step
    };

    /// @brief Stores a feature or step failure with its source location and messages.
    struct StepFailure {
        /// @brief Category of failure.
        FailureKind kind{FailureKind::step_failure};
        /// @brief Text of the failed step, or empty for a feature-level failure.
        std::string step_text;
        /// @brief Source location associated with the failure.
        SourceLocation location{};
        /// @brief Human-readable failure details.
        std::vector<std::string> messages;
    };

    /// @brief Holds the outcome of one scenario execution.
    struct ScenarioResult {
        /// @brief Scenario name.
        std::string name;
        /// @brief Failures recorded for this scenario.
        std::vector<StepFailure> failures;

        /**
         * @brief Reports whether the scenario completed without failures.
         * @return True when the scenario has no recorded failures.
         */
        [[nodiscard]] bool passed() const noexcept {
            return failures.empty();
        }
    };

    /// @brief Holds the parsed feature identity and outcomes of its scenarios.
    struct RunResult {
        /// @brief URI or path identifying the feature source.
        std::string uri;
        /// @brief Parsed feature name, when available.
        std::string feature_name;
        /// @brief Outcome of each runnable scenario.
        std::vector<ScenarioResult> scenarios;
        /// @brief Feature-level failures, including file parsing failures.
        std::vector<StepFailure> failures;

        /**
         * @brief Reports whether the feature and every scenario completed without failures.
         * @return True when there are no feature-level or scenario failures.
         */
        [[nodiscard]] bool passed() const noexcept {
            return failures.empty() && std::ranges::all_of(
                scenarios,
                [](const ScenarioResult& scenario) { return scenario.passed(); }
            );
        }
    };

    namespace detail {
        template<class Definition, class... Args>
        jh::meta::expected<RunResult, run_error>
        run_feature(const Feature& feature, Args&... args) {
            using object_type = typename Definition::object_type;
            static_assert(std::is_constructible_v<object_type, Args&...>,
                          "StepDefinition object must be constructible from the run arguments.");

            RunResult result;
            result.uri = feature.uri;
            result.feature_name = feature.name;

            for (const auto& scenario : feature.scenarios) {
                ScenarioResult scenario_result;
                scenario_result.name = scenario.name;
                // Reuse run-scoped dependencies while keeping each scenario's
                // step object independent; do not move the arguments per scenario.
                object_type object{args...};

                for (const auto& step : scenario.steps) {
                    StepContext context;
                    const auto dispatch = Definition::dispatch(object, step, context);
                    if (!dispatch) {
                        return jh::meta::unexpected(run_error::ambiguous_step);
                    }

                    if (dispatch->status == dispatch_status::undefined_step) {
                        scenario_result.failures.push_back({
                            FailureKind::undefined_step,
                            step.text,
                            step.location,
                            {"No step definition matched this step."}
                        });
                        break;
                    }

                    if (dispatch->status == dispatch_status::invalid_arguments) {
                        scenario_result.failures.push_back({
                            FailureKind::step_failure,
                            step.text,
                            step.location,
                            {"Matching step definitions rejected the captured values or DataTable attachment."}
                        });
                        break;
                    }

                    if (context.failed()) {
                        StepFailure failure{
                            FailureKind::step_failure,
                            step.text,
                            step.location,
                            {}
                        };
                        for (const auto& message : context.failures()) {
                            failure.messages.push_back(message);
                        }
                        scenario_result.failures.push_back(std::move(failure));
                        break;
                    }
                }

                result.scenarios.push_back(std::move(scenario_result));
            }
            return result;
        }
    }

    /**
     * @brief Executes a parsed feature using the specified step definitions.
     * @tparam Definition StepDefinition used to dispatch steps.
     * @tparam Args Types of arguments used to construct the step object for each scenario.
     * @param feature Parsed feature to execute.
     * @param args Arguments supplied to each scenario's step object.
     * @return All scenario results, or <code>run_error::ambiguous_step</code> if multiple definitions accept a step.
     */
    template<class Definition, class... Args>
    [[nodiscard]] jh::meta::expected<RunResult, run_error>
    run(const Feature& feature, Args&&... args) {
        return detail::run_feature<Definition>(feature, args...);
    }

    /**
     * @brief Reads and executes a feature file using the specified step definitions.
     * @tparam Definition StepDefinition used to dispatch steps.
     * @tparam Args Types of arguments used to construct the step object for each scenario.
     * @param path Filesystem path of the feature file.
     * @param args Arguments supplied to each scenario's step object.
     * @return Run results; file and parse failures are stored in <code>RunResult</code>, while ambiguous steps are returned as errors.
     */
    template<class Definition, class... Args>
    [[nodiscard]] jh::meta::expected<RunResult, run_error>
    run_file(const std::filesystem::path& path, Args&&... args) {
        GherkinDiagnostic diagnostic;
        auto feature = parse_feature_file(path.string(), &diagnostic);
        if (!feature) {
            RunResult result;
            result.uri = path.string();
            result.failures.push_back({
                FailureKind::parse_error,
                {},
                diagnostic.location,
                {diagnostic.message}
            });
            return result;
        }
        return run<Definition>(*feature, args...);
    }

    /**
     * @brief Reads and executes the feature file at a filesystem path.
     * @tparam Definition StepDefinition used to dispatch steps.
     * @tparam Args Types of arguments used to construct the step object for each scenario.
     * @param path Filesystem path of the feature file.
     * @param args Arguments supplied to each scenario's step object.
     * @return Run results or an ambiguous-step error.
     */
    template<class Definition, class... Args>
    [[nodiscard]] jh::meta::expected<RunResult, run_error>
    run(const std::filesystem::path& path, Args&&... args) {
        return run_file<Definition>(path, args...);
    }

    /**
     * @brief Reads and executes the feature file identified by a string-view path.
     * @tparam Definition StepDefinition used to dispatch steps.
     * @tparam Args Types of arguments used to construct the step object for each scenario.
     * @param path Feature file path.
     * @param args Arguments supplied to each scenario's step object.
     * @return Run results or an ambiguous-step error.
     */
    template<class Definition, class... Args>
    [[nodiscard]] jh::meta::expected<RunResult, run_error>
    run(const std::string_view path, Args&&... args) {
        return run_file<Definition>(std::filesystem::path{std::string{path}}, args...);
    }

    /**
     * @brief Reads and executes the feature file identified by a C-string path.
     * @tparam Definition StepDefinition used to dispatch steps.
     * @tparam Args Types of arguments used to construct the step object for each scenario.
     * @param path Null-terminated feature file path.
     * @param args Arguments supplied to each scenario's step object.
     * @return Run results or an ambiguous-step error.
     */
    template<class Definition, class... Args>
    [[nodiscard]] jh::meta::expected<RunResult, run_error>
    run(const char* path, Args&&... args) {
        return run_file<Definition>(std::filesystem::path{path}, args...);
    }

    /**
     * @brief Reads and executes the feature file identified by a string path.
     * @tparam Definition StepDefinition used to dispatch steps.
     * @tparam Args Types of arguments used to construct the step object for each scenario.
     * @param path Feature file path.
     * @param args Arguments supplied to each scenario's step object.
     * @return Run results or an ambiguous-step error.
     */
    template<class Definition, class... Args>
    [[nodiscard]] jh::meta::expected<RunResult, run_error>
    run(const std::string& path, Args&&... args) {
        return run_file<Definition>(std::filesystem::path{path}, args...);
    }

    /**
     * @brief Formats feature and scenario results as a human-readable report.
     * @param result Run outcome to format.
     * @return Report text; this function does not write to a stream.
     */
    [[nodiscard]] inline std::string format_report(const RunResult& result) {
        std::ostringstream output;
        if (!result.feature_name.empty()) output << "Feature: " << result.feature_name << '\n';
        if (!result.uri.empty()) output << "File: " << result.uri << '\n';

        for (const auto& failure : result.failures) {
            output << "FAIL";
            if (failure.location.line != 0) {
                output << " at " << failure.location.line;
                if (failure.location.column != 0) output << ':' << failure.location.column;
            }
            output << ": ";
            if (!failure.step_text.empty()) output << failure.step_text << ": ";
            for (std::size_t i = 0; i < failure.messages.size(); ++i) {
                if (i != 0) output << "; ";
                output << failure.messages[i];
            }
            output << '\n';
        }

        for (const auto& scenario : result.scenarios) {
            output << (scenario.passed() ? "PASS: " : "FAIL: ") << scenario.name << '\n';
            for (const auto& failure : scenario.failures) {
                output << "  ";
                if (failure.location.line != 0) {
                    output << failure.location.line;
                    if (failure.location.column != 0) output << ':' << failure.location.column;
                    output << ' ';
                }
                output << failure.step_text << '\n';
                for (const auto& message : failure.messages) {
                    output << "    " << message << '\n';
                }
            }
        }

        return output.str();
    }
}
