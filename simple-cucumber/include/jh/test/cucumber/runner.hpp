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
    enum class FailureKind {
        parse_error,
        undefined_step,
        step_failure
    };

    enum class run_error {
        ambiguous_step
    };

    struct StepFailure {
        FailureKind kind{FailureKind::step_failure};
        std::string step_text;
        SourceLocation location{};
        std::vector<std::string> messages;
    };

    struct ScenarioResult {
        std::string name;
        std::vector<StepFailure> failures;

        [[nodiscard]] bool passed() const noexcept {
            return failures.empty();
        }
    };

    struct RunResult {
        std::string uri;
        std::string feature_name;
        std::vector<ScenarioResult> scenarios;
        std::vector<StepFailure> failures;

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

    template<class Definition, class... Args>
    [[nodiscard]] jh::meta::expected<RunResult, run_error>
    run(const Feature& feature, Args&&... args) {
        return detail::run_feature<Definition>(feature, args...);
    }

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

    template<class Definition, class... Args>
    [[nodiscard]] jh::meta::expected<RunResult, run_error>
    run(const std::filesystem::path& path, Args&&... args) {
        return run_file<Definition>(path, args...);
    }

    template<class Definition, class... Args>
    [[nodiscard]] jh::meta::expected<RunResult, run_error>
    run(const std::string_view path, Args&&... args) {
        return run_file<Definition>(std::filesystem::path{std::string{path}}, args...);
    }

    template<class Definition, class... Args>
    [[nodiscard]] jh::meta::expected<RunResult, run_error>
    run(const char* path, Args&&... args) {
        return run_file<Definition>(std::filesystem::path{path}, args...);
    }

    template<class Definition, class... Args>
    [[nodiscard]] jh::meta::expected<RunResult, run_error>
    run(const std::string& path, Args&&... args) {
        return run_file<Definition>(std::filesystem::path{path}, args...);
    }

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
