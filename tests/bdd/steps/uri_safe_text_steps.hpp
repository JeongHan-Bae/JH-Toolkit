#pragma once

#include <charconv>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "jh/serio"
#include "jh/test/cucumber/cucumber.hpp"

namespace jh_toolkit_bdd {
    class UriSafeTextSteps final {
        std::string input_;
        std::string encoded_;
        std::string recovered_;
        bool rejected_{};

        static std::string parse_hex(std::string_view hex)
        {
            if (hex.size() % 2 != 0) throw std::invalid_argument("hex input must contain pairs of digits");
            std::string result;
            result.reserve(hex.size() / 2);
            for (std::size_t i = 0; i < hex.size(); i += 2) {
                unsigned int value{};
                const auto [end, error] = std::from_chars(hex.data() + i, hex.data() + i + 2, value, 16);
                if (error != std::errc{} || end != hex.data() + i + 2 || value > 0xFF) {
                    throw std::invalid_argument("hex input contains an invalid byte");
                }
                result.push_back(static_cast<char>(value));
            }
            return result;
        }

    public:
        void given_legal_text(std::string_view text) { input_.assign(text); }
        void given_text_bytes(std::string_view hex) { input_ = parse_hex(hex); }
        void given_encoded_text(std::string_view encoded) { input_.assign(encoded); }

        void safely_encode()
        {
            encoded_ = jh::serio::uri::encode_safe(input_);
        }

        void safely_decode()
        {
            recovered_ = jh::serio::uri::decode_safe(encoded_);
        }

        void safely_encode_candidate()
        {
            rejected_ = false;
            try {
                static_cast<void>(jh::serio::uri::encode_safe(input_));
            } catch (const std::runtime_error&) {
                rejected_ = true;
            }
        }

        void safely_decode_candidate()
        {
            rejected_ = false;
            try {
                static_cast<void>(jh::serio::uri::decode_safe(input_));
            } catch (const std::runtime_error&) {
                rejected_ = true;
            }
        }

        void encoded_text_matches(std::string_view expected, jh::test::cucumber::StepContext& context) const
        {
            context.expect(encoded_ == expected, "safe URI encoding differs from the expected text");
        }

        void recovered_text_matches(jh::test::cucumber::StepContext& context) const
        {
            context.expect(recovered_ == input_, "safe URI decoding changed the original text");
        }

        void encoding_rejects_candidate(jh::test::cucumber::StepContext& context) const
        {
            context.expect(rejected_, "safe URI encoding should reject illegal text");
        }

        void decoding_rejects_result(jh::test::cucumber::StepContext& context) const
        {
            context.expect(rejected_, "safe URI decoding should reject illegal output");
        }
    };

    using UriSafeTextDefinition = jh::test::cucumber::StepDefinition<
        UriSafeTextSteps,
        jh::test::cucumber::Given<
            "legal UTF-8 text <string>", &UriSafeTextSteps::given_legal_text>,
        jh::test::cucumber::Given<
            "text bytes represented by hexadecimal <string>", &UriSafeTextSteps::given_text_bytes>,
        jh::test::cucumber::Given<
            "percent-encoded text <string>", &UriSafeTextSteps::given_encoded_text>,
        jh::test::cucumber::When<
            "it is encoded with text validation", &UriSafeTextSteps::safely_encode>,
        jh::test::cucumber::When<
            "the encoded text is decoded with text validation", &UriSafeTextSteps::safely_decode>,
        jh::test::cucumber::When<
            "the candidate is safely encoded", &UriSafeTextSteps::safely_encode_candidate>,
        jh::test::cucumber::When<
            "it is decoded as safe URI text", &UriSafeTextSteps::safely_decode_candidate>,
        jh::test::cucumber::Then<
            "the safe encoded URI text is <string>", &UriSafeTextSteps::encoded_text_matches>,
        jh::test::cucumber::Then<
            "the recovered text matches the original", &UriSafeTextSteps::recovered_text_matches>,
        jh::test::cucumber::Then<
            "safe encoding rejects the candidate", &UriSafeTextSteps::encoding_rejects_candidate>,
        jh::test::cucumber::Then<
            "safe decoding rejects the result", &UriSafeTextSteps::decoding_rejects_result>
    >;
}
