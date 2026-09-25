#pragma once

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <vector>

#include "jh/serio"
#include "jh/test/cucumber/cucumber.hpp"

namespace jh_toolkit_bdd {
    class Base64Steps final {
        std::vector<std::uint8_t> original_;
        std::vector<std::uint8_t> recovered_;
        std::string format_;
        std::string encoded_;
        std::vector<std::string> invalid_inputs_;
        std::vector<bool> invalid_rejected_;

        static std::vector<std::uint8_t> parse_hex(std::string_view hex)
        {
            if (hex.size() % 2 != 0) throw std::invalid_argument("hex input must contain pairs of digits");
            std::vector<std::uint8_t> result;
            result.reserve(hex.size() / 2);
            for (std::size_t i = 0; i < hex.size(); i += 2) {
                unsigned int value{};
                const auto [end, error] = std::from_chars(hex.data() + i, hex.data() + i + 2, value, 16);
                if (error != std::errc{} || end != hex.data() + i + 2 || value > 0xFF) {
                    throw std::invalid_argument("hex input contains an invalid byte");
                }
                result.push_back(static_cast<std::uint8_t>(value));
            }
            return result;
        }

    public:
        void given_bytes(std::string_view hex)
        {
            original_ = parse_hex(hex);
            recovered_.clear();
            encoded_.clear();
        }

        void given_empty_bytes()
        {
            original_.clear();
            recovered_.clear();
            encoded_.clear();
        }

        void encode(std::string_view format)
        {
            format_.assign(format);
            if (format == "Base64") {
                encoded_ = jh::serio::base64::encode(original_.data(), original_.size());
            } else if (format == "Base64URL without padding") {
                encoded_ = jh::serio::base64url::encode(original_.data(), original_.size(), false);
            } else if (format == "Base64URL with padding") {
                encoded_ = jh::serio::base64url::encode(original_.data(), original_.size(), true);
            } else {
                throw std::invalid_argument("unknown Base64 format in feature");
            }
        }

        void decode_encoded_text()
        {
            if (format_ == "Base64") {
                recovered_ = jh::serio::base64::decode(encoded_);
            } else if (format_ == "Base64URL without padding" || format_ == "Base64URL with padding") {
                recovered_ = jh::serio::base64url::decode(encoded_);
            } else {
                throw std::logic_error("Base64 format was not selected before decoding");
            }
        }

        void encoded_text_matches(std::string_view expected, jh::test::cucumber::StepContext& context) const
        {
            context.expect(encoded_ == expected, "Base64 output differs from the expected representation");
        }

        void empty_input_produces_no_text(jh::test::cucumber::StepContext& context) const
        {
            context.expect(encoded_.empty(), "encoding an empty byte sequence should produce empty text");
        }

        void given_malformed_texts(
            const jh::test::cucumber::DataTable& table,
            jh::test::cucumber::StepContext& context)
        {
            invalid_inputs_.clear();
            invalid_rejected_.clear();
            for (const auto& row : table.rows()) {
                const auto value = row.at("encoded");
                if (!context.expect(value.has_value(), "malformed Base64 table needs an encoded column")) return;
                invalid_inputs_.emplace_back(*value);
            }
        }

        void given_text_with_embedded_nul()
        {
            invalid_inputs_.clear();
            invalid_rejected_.clear();
            invalid_inputs_.emplace_back("AB\0CD==", 7);
        }

        void decode_invalid_inputs()
        {
            invalid_rejected_.clear();
            for (const auto& input : invalid_inputs_) {
                bool rejected = false;
                try {
                    static_cast<void>(jh::serio::base64::decode(input));
                } catch (const std::runtime_error&) {
                    rejected = true;
                }
                invalid_rejected_.push_back(rejected);
            }
        }

        void malformed_inputs_are_rejected(jh::test::cucumber::StepContext& context) const
        {
            const bool all_rejected = !invalid_inputs_.empty()
                && invalid_rejected_.size() == invalid_inputs_.size()
                && std::ranges::all_of(invalid_rejected_, [](const bool value) { return value; });
            context.expect(all_rejected, "every malformed Base64 input should be rejected");
        }

        void bytes_match(jh::test::cucumber::StepContext& context) const
        {
            context.expect(recovered_ == original_, "Base64 did not recover the input bytes");
        }
    };

    using Base64Definition = jh::test::cucumber::StepDefinition<
        Base64Steps,
        jh::test::cucumber::Given<
            "a byte sequence represented by hexadecimal <string>", &Base64Steps::given_bytes>,
        jh::test::cucumber::When<
            "it is encoded with <string>", &Base64Steps::encode>,
        jh::test::cucumber::When<
            "the encoded text is decoded", &Base64Steps::decode_encoded_text>,
        jh::test::cucumber::When<
            "each text is decoded", &Base64Steps::decode_invalid_inputs>,
        jh::test::cucumber::Then<
            "the encoded Base64 string is <string>", &Base64Steps::encoded_text_matches>,
        jh::test::cucumber::Then<
            "encoding an empty sequence produces no Base64 text", &Base64Steps::empty_input_produces_no_text>,
        jh::test::cucumber::Then<
            "the recovered byte sequence matches the original", &Base64Steps::bytes_match>,
        jh::test::cucumber::Then<
            "each malformed input is rejected", &Base64Steps::malformed_inputs_are_rejected>,
        jh::test::cucumber::Given<
            "an empty byte sequence", &Base64Steps::given_empty_bytes>,
        jh::test::cucumber::Given<
            "the following malformed Base64 texts", &Base64Steps::given_malformed_texts>,
        jh::test::cucumber::Given<
            "Base64 text containing an embedded NUL byte", &Base64Steps::given_text_with_embedded_nul>
    >;
}
