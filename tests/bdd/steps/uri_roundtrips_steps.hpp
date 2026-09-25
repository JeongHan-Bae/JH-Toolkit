#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "jh/serio"
#include "jh/test/cucumber/cucumber.hpp"

namespace jh_toolkit_bdd {
    class UriSteps final {
        struct Component final {
            std::string encoded;
            std::string expected;
            std::string actual;
        };

        std::string original_;
        std::string recovered_;
        std::string encoded_;
        std::vector<Component> encoded_components_;
        std::vector<std::string> malformed_components_;
        std::vector<bool> malformed_rejected_;

    public:
        void given_component(std::string_view text) { original_.assign(text); }

        void given_empty_component()
        {
            original_.clear();
            recovered_.clear();
            encoded_.clear();
        }

        void encode_component()
        {
            encoded_ = jh::serio::uri::encode(original_);
        }

        void decode_component()
        {
            recovered_ = jh::serio::uri::decode(encoded_);
        }

        void encoded_component_matches(std::string_view expected, jh::test::cucumber::StepContext& context) const
        {
            context.expect(encoded_ == expected, "URI encoding differs from the expected representation");
        }

        void empty_component_encodes_to_empty(jh::test::cucumber::StepContext& context) const
        {
            context.expect(encoded_.empty(), "an empty URI component should encode to empty text");
        }

        void given_encoded_components(
            const jh::test::cucumber::DataTable& table,
            jh::test::cucumber::StepContext& context)
        {
            encoded_components_.clear();
            for (const auto& row : table.rows()) {
                const auto encoded = row.at("encoded");
                const auto expected = row.at("expected");
                if (!context.expect(encoded.has_value() && expected.has_value(),
                                    "URI table needs encoded and expected columns")) return;
                encoded_components_.push_back({std::string{*encoded}, std::string{*expected}, {}});
            }
        }

        void decode_encoded_components()
        {
            for (auto& component : encoded_components_) {
                component.actual = jh::serio::uri::decode(component.encoded);
            }
        }

        void decoded_components_match(jh::test::cucumber::StepContext& context) const
        {
            bool matches = !encoded_components_.empty();
            for (const auto& component : encoded_components_) {
                matches = matches && component.actual == component.expected;
            }
            context.expect(matches, "decoded URI values should match their expected text");
        }

        void given_malformed_components(
            const jh::test::cucumber::DataTable& table,
            jh::test::cucumber::StepContext& context)
        {
            malformed_components_.clear();
            malformed_rejected_.clear();
            for (const auto& row : table.rows()) {
                const auto encoded = row.at("encoded");
                if (!context.expect(encoded.has_value(), "malformed URI table needs an encoded column")) return;
                malformed_components_.emplace_back(*encoded);
            }
        }

        void given_raw_control_character()
        {
            malformed_components_.clear();
            malformed_rejected_.clear();
            malformed_components_.emplace_back("abc");
            malformed_components_.back().push_back('\x01');
            malformed_components_.back() += "def";
        }

        void decode_malformed_components()
        {
            malformed_rejected_.clear();
            for (const auto& input : malformed_components_) {
                bool rejected = false;
                try {
                    static_cast<void>(jh::serio::uri::decode(input));
                } catch (const std::runtime_error&) {
                    rejected = true;
                }
                malformed_rejected_.push_back(rejected);
            }
        }

        void malformed_components_are_rejected(jh::test::cucumber::StepContext& context) const
        {
            bool all_rejected = !malformed_components_.empty()
                && malformed_rejected_.size() == malformed_components_.size();
            for (const bool rejected : malformed_rejected_) all_rejected = all_rejected && rejected;
            context.expect(all_rejected, "every malformed URI component should be rejected");
        }

        void component_matches(jh::test::cucumber::StepContext& context) const
        {
            context.expect(recovered_ == original_, "URI decoding changed the original component");
        }
    };

    using UriDefinition = jh::test::cucumber::StepDefinition<
        UriSteps,
        jh::test::cucumber::Given<
            "a URI component containing <string>", &UriSteps::given_component>,
        jh::test::cucumber::Given<
            "an empty URI component", &UriSteps::given_empty_component>,
        jh::test::cucumber::Given<
            "the following encoded URI component pairs", &UriSteps::given_encoded_components>,
        jh::test::cucumber::Given<
            "the following malformed percent-encoded values", &UriSteps::given_malformed_components>,
        jh::test::cucumber::Given<
            "encoded URI text containing a raw control character", &UriSteps::given_raw_control_character>,
        jh::test::cucumber::When<
            "it is percent encoded", &UriSteps::encode_component>,
        jh::test::cucumber::When<
            "the encoded component is decoded", &UriSteps::decode_component>,
        jh::test::cucumber::When<
            "each encoded URI component is decoded", &UriSteps::decode_encoded_components>,
        jh::test::cucumber::When<
            "each malformed URI component is decoded", &UriSteps::decode_malformed_components>,
        jh::test::cucumber::Then<
            "the encoded URI component is <string>", &UriSteps::encoded_component_matches>,
        jh::test::cucumber::Then<
            "an empty input produces no encoded URI text", &UriSteps::empty_component_encodes_to_empty>,
        jh::test::cucumber::Then<
            "each decoded component matches its expected text", &UriSteps::decoded_components_match>,
        jh::test::cucumber::Then<
            "each malformed URI component is rejected", &UriSteps::malformed_components_are_rejected>,
        jh::test::cucumber::Then<
            "the recovered URI component matches the original", &UriSteps::component_matches>
    >;
}
