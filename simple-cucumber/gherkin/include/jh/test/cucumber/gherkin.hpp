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
 * @file gherkin.hpp
 * @brief Gherkin feature parsing interface.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 */

#pragma once

#include <string>
#include <string_view>

#include "jh/metax/expected.h"
#include "jh/test/cucumber/runtime.hpp"

namespace jh::test::cucumber {
    /// @brief Describes why a Gherkin feature could not be parsed.
    enum class gherkin_error {
        /// @brief The feature file could not be opened or read.
        io_failure,
        /// @brief The Gherkin parser rejected the source syntax.
        syntax_error,
        /// @brief The Gherkin document contains no Feature.
        missing_feature,
        /// @brief A step uses an unsupported DocString attachment.
        unsupported_doc_string,
        /// @brief A DataTable has rows with an invalid shape.
        invalid_data_table
    };

    /// @brief Holds source details for a Gherkin parsing failure.
    struct GherkinDiagnostic {
        /// @brief URI or path identifying the source.
        std::string uri;
        /// @brief Human-readable description of the failure.
        std::string message;
        /// @brief Source location associated with the failure, when available.
        SourceLocation location{};
    };

    /**
     * @brief Parses Gherkin source text into a feature and runnable scenarios.
     * @param uri Identifier stored with the parsed feature and diagnostic.
     * @param source Gherkin document contents.
     * @param diagnostic Optional destination for failure details.
     * @return The parsed feature, or the reason parsing failed. DocString attachments are unsupported.
     */
    [[nodiscard]] jh::meta::expected<Feature, gherkin_error>
    parse_feature(
        std::string_view uri,
        std::string_view source,
        GherkinDiagnostic* diagnostic = nullptr
    );

    /**
     * @brief Reads and parses a Gherkin feature file.
     * @param path Filesystem path of the feature file.
     * @param diagnostic Optional destination for I/O or parsing details.
     * @return The parsed feature, or the reason reading or parsing failed.
     */
    [[nodiscard]] jh::meta::expected<Feature, gherkin_error>
    parse_feature_file(
        std::string_view path,
        GherkinDiagnostic* diagnostic = nullptr
    );
}
