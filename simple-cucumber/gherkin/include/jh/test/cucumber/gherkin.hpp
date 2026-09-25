#pragma once

#include <string>
#include <string_view>

#include "jh/metax/expected.h"
#include "jh/test/cucumber/runtime.hpp"

namespace jh::test::cucumber {
    enum class gherkin_error {
        io_failure,
        syntax_error,
        missing_feature,
        unsupported_doc_string,
        invalid_data_table
    };

    struct GherkinDiagnostic {
        std::string uri;
        std::string message;
        SourceLocation location{};
    };

    [[nodiscard]] jh::meta::expected<Feature, gherkin_error>
    parse_feature(
        std::string_view uri,
        std::string_view source,
        GherkinDiagnostic* diagnostic = nullptr
    );

    [[nodiscard]] jh::meta::expected<Feature, gherkin_error>
    parse_feature_file(
        std::string_view path,
        GherkinDiagnostic* diagnostic = nullptr
    );
}
