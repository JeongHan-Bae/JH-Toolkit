#include "jh/test/cucumber/gherkin.hpp"

#include <fstream>
#include <iterator>
#include <string>
#include <unordered_map>
#include <utility>

#include <cucumber/gherkin/parser.hpp>
#include <cucumber/gherkin/pickle_compiler.hpp>
#include <cucumber/messages/all.hpp>

namespace jh::test::cucumber {
    namespace gherkin_upstream = ::cucumber::gherkin;
    namespace messages = ::cucumber::messages;

    namespace {
        SourceLocation convert_location(const messages::location& location) noexcept {
            return {
                location.line,
                location.column.value_or(0)
            };
        }

        void remember_steps(
            const std::vector<messages::step>& steps,
            std::unordered_map<std::string, SourceLocation>& locations
        ) {
            for (const auto& step : steps) {
                locations.emplace(step.id, convert_location(step.location));
            }
        }

        std::unordered_map<std::string, SourceLocation> collect_step_locations(
            const messages::feature& feature
        ) {
            std::unordered_map<std::string, SourceLocation> locations;

            for (const auto& child : feature.children) {
                if (child.background) {
                    remember_steps(child.background->steps, locations);
                }
                if (child.scenario) {
                    remember_steps(child.scenario->steps, locations);
                }
                if (!child.rule) continue;

                for (const auto& rule_child : child.rule->children) {
                    if (rule_child.background) {
                        remember_steps(rule_child.background->steps, locations);
                    }
                    if (rule_child.scenario) {
                        remember_steps(rule_child.scenario->steps, locations);
                    }
                }
            }

            return locations;
        }

        StepKind convert_step_kind(
            const std::optional<messages::pickle_step_type>& type
        ) noexcept {
            if (!type) return StepKind::unknown;

            switch (*type) {
                case messages::pickle_step_type::CONTEXT: return StepKind::given;
                case messages::pickle_step_type::ACTION: return StepKind::when;
                case messages::pickle_step_type::OUTCOME: return StepKind::then;
                default: return StepKind::unknown;
            }
        }

        void set_diagnostic(
            GherkinDiagnostic* diagnostic,
            std::string_view uri,
            std::string_view message,
            SourceLocation location = {}
        ) {
            if (!diagnostic) return;
            diagnostic->uri.assign(uri);
            diagnostic->message.assign(message);
            diagnostic->location = location;
        }
    }

    jh::meta::expected<Feature, gherkin_error> parse_feature(
        const std::string_view uri,
        const std::string_view source,
        GherkinDiagnostic* diagnostic
    ) {
        messages::gherkin_document document;
        try {
            gherkin_upstream::parser<> parser;
            document = parser.parse(uri, source);
        } catch (const gherkin_upstream::parser_error& error) {
            const auto& location = error.location();
            set_diagnostic(
                diagnostic,
                uri,
                error.what(),
                convert_location(location)
            );
            return jh::meta::unexpected(gherkin_error::syntax_error);
        }

        if (!document.feature) {
            set_diagnostic(diagnostic, uri, "Gherkin document contains no Feature.");
            return jh::meta::unexpected(gherkin_error::missing_feature);
        }

        const auto locations = collect_step_locations(*document.feature);
        gherkin_upstream::pickle_compiler compiler;
        gherkin_upstream::pickles pickles;
        try {
            pickles = compiler.compile(document, std::string{uri});
        } catch (const gherkin_upstream::parser_error& error) {
            const auto& location = error.location();
            set_diagnostic(
                diagnostic,
                uri,
                error.what(),
                convert_location(location)
            );
            return jh::meta::unexpected(gherkin_error::syntax_error);
        }

        Feature result;
        result.name = document.feature->name;
        result.uri.assign(uri);
        result.scenarios.reserve(pickles.size());

        for (const auto& pickle : pickles) {
            Scenario scenario;
            scenario.name = pickle.name;
            if (pickle.location) scenario.location = convert_location(*pickle.location);
            scenario.steps.reserve(pickle.steps.size());

            for (const auto& pickle_step : pickle.steps) {
                Step step;
                step.kind = convert_step_kind(pickle_step.type);
                step.text = pickle_step.text;

                if (!pickle_step.ast_node_ids.empty()) {
                    const auto found = locations.find(pickle_step.ast_node_ids.front());
                    if (found != locations.end()) step.location = found->second;
                }
                if (step.location.line == 0) step.location = scenario.location;

                if (pickle_step.argument) {
                    if (pickle_step.argument->doc_string) {
                        set_diagnostic(
                            diagnostic,
                            uri,
                            "DocString step attachments are not supported yet.",
                            step.location
                        );
                        return jh::meta::unexpected(gherkin_error::unsupported_doc_string);
                    }

                    if (pickle_step.argument->data_table) {
                        std::vector<std::vector<std::string>> rows;
                        rows.reserve(pickle_step.argument->data_table->rows.size());
                        for (const auto& source_row : pickle_step.argument->data_table->rows) {
                            std::vector<std::string> cells;
                            cells.reserve(source_row.cells.size());
                            for (const auto& cell : source_row.cells) {
                                cells.push_back(cell.value);
                            }
                            rows.push_back(std::move(cells));
                        }

                        auto table = DataTable::from_rows(std::move(rows));
                        if (!table) {
                            set_diagnostic(
                                diagnostic,
                                uri,
                                "Gherkin DataTable rows have an invalid shape.",
                                step.location
                            );
                            return jh::meta::unexpected(gherkin_error::invalid_data_table);
                        }
                        step.table = std::move(*table);
                    }
                }

                scenario.steps.push_back(std::move(step));
            }

            result.scenarios.push_back(std::move(scenario));
        }

        return result;
    }

    jh::meta::expected<Feature, gherkin_error> parse_feature_file(
        const std::string_view path,
        GherkinDiagnostic* diagnostic
    ) {
        std::ifstream file{std::string{path}, std::ios::binary};
        if (!file) {
            set_diagnostic(diagnostic, path, "Unable to open feature file.");
            return jh::meta::unexpected(gherkin_error::io_failure);
        }

        std::string source(
            std::istreambuf_iterator<char>{file},
            std::istreambuf_iterator<char>{}
        );
        if (file.bad()) {
            set_diagnostic(diagnostic, path, "Unable to read feature file.");
            return jh::meta::unexpected(gherkin_error::io_failure);
        }

        return parse_feature(path, source, diagnostic);
    }
}
