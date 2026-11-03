/**
 * @file value_conversion_roundtrip_steps.hpp
 * @brief BDD steps for string-backed value conversion.
 */

#pragma once

#include "jh/serio"
#include "jh/test/cucumber/cucumber.hpp"

#include <string>
#include <string_view>
#include <type_traits>

namespace jh_toolkit_bdd {
    struct JsonViewLike final {
        std::string_view text;

        explicit JsonViewLike(const std::string& source) : text(source) {}
        JsonViewLike(const JsonViewLike&) = delete;
        JsonViewLike(JsonViewLike&&) = delete;
        explicit operator std::string() const { return std::string{text}; }
    };

    static_assert(!std::is_move_constructible_v<JsonViewLike>);

    class ValueConversionRoundtripSteps final {
        std::string source_;
        std::string serialized_;
        std::string_view restored_view_;
        int parsed_ = 0;

    public:
        void given_live_text() {
            source_ = "customer:42";
        }

        void given_json_text() {
            source_ = R"({"key":"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"})";
        }

        void given_numeric_text() {
            source_ = "42";
        }

        void deserialize_view() {
            restored_view_ = jh::serio::deserialize<std::string_view>(source_);
        }

        void serialize_json_view() {
            const JsonViewLike value{source_};
            serialized_ = jh::serio::serialize(value);
        }

        void reconstruct_json_view() {
            const auto value = jh::serio::deserialize<JsonViewLike>(serialized_);
            restored_view_ = value.text;
        }

        void deserialize_numeric_text() {
            parsed_ = jh::serio::deserialize<int>(source_);
        }

        void view_borrows_source(jh::test::cucumber::StepContext& context) const {
            context.expect(restored_view_.data() == source_.data() &&
                           restored_view_ == source_,
                           "string view did not borrow the live source text");
        }

        void json_view_borrows_serialized(jh::test::cucumber::StepContext& context) const {
            context.expect(serialized_ == source_ &&
                           restored_view_.data() == serialized_.data() &&
                           restored_view_ == serialized_,
                           "JSON view-like value lost its serialized backing text");
        }

        void numeric_text_preserved(jh::test::cucumber::StepContext& context) const {
            context.expect(parsed_ == 42 && source_ == "42",
                           "stream extraction changed the source text or parsed value");
        }
    };

    using ValueConversionRoundtripDefinition = jh::test::cucumber::StepDefinition<
        ValueConversionRoundtripSteps,
        jh::test::cucumber::Given<
            "a live text value", &ValueConversionRoundtripSteps::given_live_text>,
        jh::test::cucumber::Given<
            "a JSON text value", &ValueConversionRoundtripSteps::given_json_text>,
        jh::test::cucumber::Given<
            "numeric text 42", &ValueConversionRoundtripSteps::given_numeric_text>,
        jh::test::cucumber::When<
            "the text is deserialized as a string view", &ValueConversionRoundtripSteps::deserialize_view>,
        jh::test::cucumber::When<
            "the JSON view-like value is serialized", &ValueConversionRoundtripSteps::serialize_json_view>,
        jh::test::cucumber::When<
            "it is reconstructed from the serialized string", &ValueConversionRoundtripSteps::reconstruct_json_view>,
        jh::test::cucumber::When<
            "the numeric text is deserialized through a stream", &ValueConversionRoundtripSteps::deserialize_numeric_text>,
        jh::test::cucumber::Then<
            "the view refers to the live source text", &ValueConversionRoundtripSteps::view_borrows_source>,
        jh::test::cucumber::Then<
            "the reconstructed view refers to the serialized string", &ValueConversionRoundtripSteps::json_view_borrows_serialized>,
        jh::test::cucumber::Then<
            "the parsed value is 42 and the source text is unchanged", &ValueConversionRoundtripSteps::numeric_text_preserved>
    >;
}
