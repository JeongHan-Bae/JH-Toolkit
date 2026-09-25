#include <algorithm>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "fail/steps.hpp"
#include "pass/steps.hpp"

namespace {
    struct CheckResult {
        bool passed{};
        std::string description;
        std::string detail;
    };

    void verify(
        std::vector<CheckResult>& results,
        const bool condition,
        const std::string_view description,
        const std::string_view detail = {}
    ) {
        results.push_back({condition, std::string{description}, std::string{detail}});
    }

    template<class Result>
    std::string report_if_failed(const Result& result) {
        if (result.passed()) return {};
        return jh::test::cucumber::format_report(result);
    }

    std::string format_custom_report(const jh::test::cucumber::RunResult& result) {
        std::ostringstream output;
        output << (result.passed() ? "PASS " : "FAIL ") << result.feature_name << '\n';

        const auto write_failure = [&output](const jh::test::cucumber::StepFailure& failure) {
            output << "  ";
            if (failure.location.line != 0) {
                output << failure.location.line << ':' << failure.location.column << ' ';
            }
            output << failure.step_text << '\n';
            for (const auto& message : failure.messages) {
                output << "    " << message << '\n';
            }
        };

        for (const auto& failure : result.failures) write_failure(failure);
        for (const auto& scenario : result.scenarios) {
            output << (scenario.passed() ? "PASS " : "FAIL ") << scenario.name << '\n';
            for (const auto& failure : scenario.failures) write_failure(failure);
        }
        return output.str();
    }

    std::string runner_error_message(const jh::test::cucumber::run_error error) {
        switch (error) {
            case jh::test::cucumber::run_error::ambiguous_step:
                return "unexpected: ambiguous step definitions";
        }
        return "unexpected runner error";
    }

    void test_expression_helpers(std::vector<CheckResult>& results) {
        constexpr auto parsed = jh::test::cucumber::parseExpression<
            "name <string> signed <int> unsigned <uint> real <double> flag <bool>"
        >();
        static_assert(parsed.valid);
        static_assert(parsed.parameter_count == 5);

        constexpr auto literal_only = jh::test::cucumber::parseExpression<"hello">();
        static_assert(literal_only.valid);
        static_assert(literal_only.parameter_count == 0);

        const auto captures = jh::test::cucumber::match_expression<
            "name <string> signed <int> unsigned <uint> real <double> flag <bool>"
        >("name Ada Lovelace signed -7 unsigned 9 real 3.5 flag true");
        verify(results, captures.has_value(), "match_expression matches all scalar placeholders");
        if (captures) {
            verify(results, (*captures)[0] == "Ada Lovelace", "capture <string>");
            verify(results, (*captures)[1] == "-7", "capture <int>");
            verify(results, (*captures)[2] == "9", "capture <uint>");
            verify(results, (*captures)[3] == "3.5", "capture <double>");
            verify(results, (*captures)[4] == "true", "capture <bool>");
        }

        const auto mismatch = jh::test::cucumber::match_expression<"hello">("prefix hello");
        verify(results, !mismatch, "match_expression rejects an unanchored literal");
        const auto literal_match = jh::test::cucumber::match_expression<"hello">("hello");
        verify(results, literal_match.has_value(), "match_expression accepts a literal-only expression");

        const auto string = jh::test::cucumber::asString("Ada Lovelace");
        const auto integer = jh::test::cucumber::asInt("-7");
        const auto unsigned_integer = jh::test::cucumber::asUint("9");
        const auto real = jh::test::cucumber::asDouble("3.5");
        const auto boolean_true = jh::test::cucumber::asBool("true");
        const auto boolean_false = jh::test::cucumber::asBool("false");
        const auto exponent_real = jh::test::cucumber::asDouble("2e6");
        constexpr auto exponent_is_number = jh::pod::string_view{"2e6", 3}.is_number();

        verify(results, string == "Ada Lovelace", "convert <string>");
        verify(results, integer && *integer == -7, "convert <int>");
        verify(results, unsigned_integer && *unsigned_integer == 9, "convert <uint>");
        verify(results, real && *real == 3.5, "convert <double>");
        verify(results, exponent_is_number, "pod string_view accepts exponent-form decimal numbers");
        verify(results, exponent_real && *exponent_real == 2e6,
               "convert exponent-form <double>");
        verify(results, boolean_true && *boolean_true, "convert <bool> true");
        verify(results, boolean_false && !*boolean_false, "convert <bool> false");
    }

    void run_passing_features(
        const std::filesystem::path& tdd_dir,
        std::vector<CheckResult>& results
    ) {
        const auto counters = jh::test::cucumber::run_file<test::pass::CounterDefinition>(
            tdd_dir / "pass/features/counter.feature"
        );
        verify(results, counters.has_value(), "counter feature has no ambiguous bindings",
               counters ? std::string{} : runner_error_message(counters.error()));
        if (counters) {
            verify(results, counters->passed(), "typed steps and scenario state", report_if_failed(*counters));
            verify(results, counters->scenarios.size() == 3, "Scenario Outline expands into pickles");
        }

        test::pass::SignedUnsignedSteps::signed_calls = 0;
        test::pass::SignedUnsignedSteps::unsigned_calls = 0;
        const auto signed_unsigned =
            jh::test::cucumber::run_file<test::pass::SignedUnsignedDefinition>(
                tdd_dir / "pass/features/signed_unsigned.feature"
            );
        verify(results, signed_unsigned.has_value(), "negative signed value has one valid binding",
               signed_unsigned ? std::string{} : runner_error_message(signed_unsigned.error()));
        if (signed_unsigned) {
            verify(results, signed_unsigned->passed(), "negative value selects <int>",
                   report_if_failed(*signed_unsigned));
        }
        verify(results,
               test::pass::SignedUnsignedSteps::signed_calls == 1 &&
                   test::pass::SignedUnsignedSteps::unsigned_calls == 0 &&
                   test::pass::SignedUnsignedSteps::signed_value == -2,
               "-2 matches <int> and rejects <uint>");

        const auto exponent_double =
            jh::test::cucumber::run_file<test::pass::ExponentDoubleDefinition>(
                tdd_dir / "pass/features/exponent_double.feature"
            );
        verify(results, exponent_double.has_value(), "exponent double has a valid binding",
               exponent_double ? std::string{} : runner_error_message(exponent_double.error()));
        if (exponent_double) {
            verify(results, exponent_double->passed(), "2e6 matches and converts as <double>",
                   report_if_failed(*exponent_double));
        }

        const auto table = jh::test::cucumber::run_file<test::pass::DataTableDefinition>(
            tdd_dir / "pass/features/data_table.feature"
        );
        verify(results, table.has_value(), "DataTable feature has no ambiguous bindings",
               table ? std::string{} : runner_error_message(table.error()));
        if (table) {
            verify(results, table->passed(), "DataTable attachment and cell conversion", report_if_failed(*table));
        }

        const auto const_steps = jh::test::cucumber::run_file<test::pass::ConstDefinition>(
            tdd_dir / "pass/features/const_steps.feature"
        );
        verify(results, const_steps.has_value(), "const feature has no ambiguous bindings",
               const_steps ? std::string{} : runner_error_message(const_steps.error()));
        if (const_steps) {
            verify(results, const_steps->passed(), "const Given, When and Then bindings",
                   report_if_failed(*const_steps));
        }

        const auto static_steps = jh::test::cucumber::run_file<test::pass::StaticDefinition>(
            tdd_dir / "pass/features/static_steps.feature"
        );
        verify(results, static_steps.has_value(), "static feature has no ambiguous bindings",
               static_steps ? std::string{} : runner_error_message(static_steps.error()));
        if (static_steps) {
            verify(results, static_steps->passed(), "static and constexpr static Given, When and Then bindings",
                   report_if_failed(*static_steps));
        }

        const auto constexpr_instance =
            jh::test::cucumber::run_file<test::pass::InstanceConstexprDefinition>(
                tdd_dir / "pass/features/constexpr_instance_steps.feature"
            );
        verify(results, constexpr_instance.has_value(), "constexpr instance feature has no ambiguous bindings",
               constexpr_instance ? std::string{} : runner_error_message(constexpr_instance.error()));
        if (constexpr_instance) {
            verify(results, constexpr_instance->passed(),
                   "constexpr non-static Given, When and Then bindings",
                   report_if_failed(*constexpr_instance));
        }
    }

    void run_injected_service_features(
        const std::filesystem::path& tdd_dir,
        std::vector<CheckResult>& results
    ) {
        const auto feature_path = tdd_dir / "pass/features/injected_user_service.feature";

        const auto in_memory = jh::test::cucumber::run_file<test::pass::InMemoryUserDefinition>(
            feature_path,
            test::pass::InMemoryUserService{}
        );
        verify(results, in_memory.has_value(), "run_file constructs non-default Steps from an adapter",
               in_memory ? std::string{} : runner_error_message(in_memory.error()));
        if (in_memory) {
            verify(results, in_memory->passed(), "in-memory service adapter works across scenarios",
                   report_if_failed(*in_memory));
            verify(results, in_memory->scenarios.size() == 2,
                   "injected service is reused with fresh Steps objects per scenario");

            const auto default_report = jh::test::cucumber::format_report(*in_memory);
            verify(results,
                   default_report.find("PASS: create and sign in a user") != std::string::npos &&
                       default_report.find("PASS: create another user with a fresh Steps object") !=
                           std::string::npos,
                   "default report prints every passing scenario");

            const auto custom_report = format_custom_report(*in_memory);
            verify(results,
                   custom_report.find("PASS step definitions with injected services") != std::string::npos &&
                       custom_report.find("PASS create and sign in a user") != std::string::npos,
                   "custom report formats successful RunResult fields");
        }

        const auto feature = jh::test::cucumber::parse_feature_file(feature_path.string());
        verify(results, feature.has_value(), "parse injected-service feature for run() test");
        if (feature) {
            std::string run_lifecycle;
            run_lifecycle.reserve(64);
            const auto database = jh::test::cucumber::run<test::pass::DatabaseUserDefinition>(
                *feature,
                test::pass::DatabaseUserService{run_lifecycle}
            );
            verify(results, database.has_value(), "run constructs non-default Steps from an adapter",
                   database ? std::string{} : runner_error_message(database.error()));
            if (database) {
                verify(results, database->passed(),
                       "database service adapter works through run()", report_if_failed(*database));
                verify(results, database->scenarios.size() == 2,
                       "run reuses injected service with fresh Steps objects per scenario");
            }
            verify(results,
                   run_lifecycle ==
                       "connect to database\ndisconnect from database\n",
                   "database adapter constructor and destructor bracket run()");
        }

        std::string path_lifecycle;
        path_lifecycle.reserve(64);
        const auto database_path =
            jh::test::cucumber::run<test::pass::DatabaseUserDefinition>(
                feature_path,
                test::pass::DatabaseUserService{path_lifecycle}
            );
        verify(results, database_path.has_value(), "run(path, args...) constructs injected Steps",
               database_path ? std::string{} : runner_error_message(database_path.error()));
        if (database_path) {
            verify(results, database_path->passed(), "run(path, args...) executes both scenarios",
                   report_if_failed(*database_path));
        }
        verify(results,
               path_lifecycle == "connect to database\ndisconnect from database\n",
               "database adapter constructor and destructor bracket run(path, args...)");
    }

    void run_expected_failures(
        const std::filesystem::path& tdd_dir,
        std::vector<CheckResult>& results
    ) {
        test::fail::ConversionSteps::invocation_count = 0;
        const auto conversion = jh::test::cucumber::run_file<test::fail::ConversionDefinition>(
            tdd_dir / "fail/features/conversion_failure.feature"
        );
        verify(results, conversion.has_value(), "conversion failure is not a runner-level ambiguity",
               conversion ? std::string{} : runner_error_message(conversion.error()));
        if (conversion) {
            verify(results, !conversion->passed(), "invalid scalar conversion is reported",
                   conversion->passed() ? "scenario passed unexpectedly" : std::string{});

            const auto default_report = jh::test::cucumber::format_report(*conversion);
            verify(results,
                   default_report.find("FAIL: report an invalid integer capture") != std::string::npos &&
                       default_report.find("the integer is 2.7") != std::string::npos &&
                       default_report.find("could not be converted to <int>") != std::string::npos,
                   "default report prints failed scenario, step, and reason");

            const auto custom_report = format_custom_report(*conversion);
            verify(results,
                   custom_report.find("FAIL typed conversion errors") != std::string::npos &&
                       custom_report.find("FAIL report an invalid integer capture") != std::string::npos &&
                       custom_report.find("the integer is 2.7") != std::string::npos,
                   "custom report formats failed RunResult fields");

            verify(results, conversion->scenarios.size() == 1,
                   "conversion failure keeps its scenario result", report_if_failed(*conversion));

            if (conversion->scenarios.size() == 1 && conversion->scenarios[0].failures.size() == 1) {
                const auto& failure = conversion->scenarios[0].failures[0];
                verify(results, failure.kind == jh::test::cucumber::FailureKind::step_failure,
                       "conversion failure is a step failure");
                const bool identifies_integer = std::ranges::any_of(
                    failure.messages,
                    [](const std::string& message) {
                        return message.find("<int>") != std::string::npos;
                    }
                );
                verify(results, identifies_integer,
                       "conversion failure identifies its expected scalar type");
            } else {
                verify(results, false, "conversion failure has one diagnostic", report_if_failed(*conversion));
            }
        } else {
            verify(results, false, "conversion failure has a RunResult value");
        }
        verify(results, test::fail::ConversionSteps::invocation_count == 0,
               "failed conversion does not invoke its step method");

        const auto undefined = jh::test::cucumber::run_file<test::fail::UndefinedDefinition>(
            tdd_dir / "fail/features/undefined_step.feature"
        );
        verify(results, undefined.has_value(), "undefined step is not a runner-level ambiguity",
               undefined ? std::string{} : runner_error_message(undefined.error()));
        if (undefined) {
            verify(results, !undefined->passed(), "undefined step is reported",
                   undefined->passed() ? "scenario passed unexpectedly" : std::string{});
            const bool has_undefined_failure =
                undefined->scenarios.size() == 1 &&
                undefined->scenarios[0].failures.size() == 1 &&
                undefined->scenarios[0].failures[0].kind ==
                    jh::test::cucumber::FailureKind::undefined_step;
            verify(results, has_undefined_failure, "undefined step has the expected failure kind",
                   report_if_failed(*undefined));
        }

        test::fail::TableAttachmentSteps::without_table_calls = 0;
        const auto unexpected_table =
            jh::test::cucumber::run_file<test::fail::UnexpectedDataTableDefinition>(
                tdd_dir / "fail/features/unexpected_data_table.feature"
            );
        verify(results, unexpected_table.has_value(),
               "unexpected DataTable attachment is a runtime step failure",
               unexpected_table ? std::string{} : runner_error_message(unexpected_table.error()));
        if (unexpected_table) {
            const bool reports_unexpected_table =
                !unexpected_table->passed() &&
                unexpected_table->scenarios.size() == 1 &&
                unexpected_table->scenarios[0].failures.size() == 1 &&
                unexpected_table->scenarios[0].failures[0].kind ==
                    jh::test::cucumber::FailureKind::step_failure &&
                !unexpected_table->scenarios[0].failures[0].messages.empty() &&
                unexpected_table->scenarios[0].failures[0].messages[0] ==
                    "step has an unexpected DataTable attachment.";
            verify(results, reports_unexpected_table,
                   "table on a binding without const DataTable& has a runtime diagnostic",
                   report_if_failed(*unexpected_table));
        }
        verify(results, test::fail::TableAttachmentSteps::without_table_calls == 0,
               "step without DataTable parameter is not invoked when a table is attached");

        test::fail::TableAttachmentSteps::with_table_calls = 0;
        const auto missing_table =
            jh::test::cucumber::run_file<test::fail::RequiredDataTableDefinition>(
                tdd_dir / "fail/features/missing_data_table.feature"
            );
        verify(results, missing_table.has_value(),
               "missing DataTable attachment is a runtime step failure",
               missing_table ? std::string{} : runner_error_message(missing_table.error()));
        if (missing_table) {
            const bool reports_missing_table =
                !missing_table->passed() &&
                missing_table->scenarios.size() == 1 &&
                missing_table->scenarios[0].failures.size() == 1 &&
                missing_table->scenarios[0].failures[0].kind ==
                    jh::test::cucumber::FailureKind::step_failure &&
                !missing_table->scenarios[0].failures[0].messages.empty() &&
                missing_table->scenarios[0].failures[0].messages[0] ==
                    "step definition requires a DataTable attachment.";
            verify(results, reports_missing_table,
                   "missing table for const DataTable& has a runtime diagnostic",
                   report_if_failed(*missing_table));
        }
        verify(results, test::fail::TableAttachmentSteps::with_table_calls == 0,
               "step with const DataTable& is not invoked when the table is missing");

        test::fail::AmbiguousSteps::invocation_count = 0;
        const auto ambiguous = jh::test::cucumber::run_file<test::fail::AmbiguousDefinition>(
            tdd_dir / "fail/features/ambiguous_step.feature"
        );
        verify(results, !ambiguous.has_value(),
               "overlapping bindings return jh::meta::unexpected",
               ambiguous ? "runner returned a value for an ambiguous step" : std::string{});
        if (!ambiguous) {
            verify(results, ambiguous.error() == jh::test::cucumber::run_error::ambiguous_step,
                   "unexpected identifies step-definition ambiguity");
        }
        verify(results, test::fail::AmbiguousSteps::invocation_count == 0,
               "ambiguous step does not invoke either matching definition");

        test::fail::AmbiguousScalarSteps::invocation_count = 0;
        const auto ambiguous_scalar =
            jh::test::cucumber::run_file<test::fail::AmbiguousScalarDefinition>(
                tdd_dir / "fail/features/ambiguous_scalar.feature"
            );
        verify(results, !ambiguous_scalar.has_value(),
               "2 is ambiguous across <string>, <int> and <uint>",
               ambiguous_scalar ? "runner returned a value for an ambiguous step" : std::string{});
        if (!ambiguous_scalar) {
            verify(results, ambiguous_scalar.error() == jh::test::cucumber::run_error::ambiguous_step,
                   "typed scalar ambiguity returns the ambiguity error");
        }
        verify(results, test::fail::AmbiguousScalarSteps::invocation_count == 0,
               "typed scalar ambiguity does not invoke any matching definition");
    }
}

int main() {
    const std::filesystem::path tdd_dir{JH_SIMPLE_CUCUMBER_TDD_DIR};
    std::vector<CheckResult> results;

    test_expression_helpers(results);
    run_passing_features(tdd_dir, results);
    run_injected_service_features(tdd_dir, results);
    run_expected_failures(tdd_dir, results);

    const auto passed_count = std::ranges::count_if(
        results,
        [](const CheckResult& result) { return result.passed; }
    );
    const auto failed_count = results.size() - passed_count;
    std::cout << "simple-cucumber TDD: " << passed_count << " passed, "
              << failed_count << " failed\n";

    std::size_t failure_number = 0;
    for (const auto& result : results) {
        if (result.passed) continue;
        std::cerr << "Failure " << ++failure_number << ": " << result.description << '\n';
        if (!result.detail.empty()) std::cerr << result.detail << '\n';
    }

    return failed_count == 0 ? 0 : 1;
}
