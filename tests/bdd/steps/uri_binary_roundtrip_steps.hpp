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
    class UriBinaryRoundtripSteps final {
        std::vector<std::uint8_t> original_;
        std::vector<std::uint8_t> recovered_;
        std::string encoded_;

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
        void given_payload(std::string_view hex) { original_ = parse_hex(hex); }

        void percent_encode_payload()
        {
            const std::string bytes{original_.begin(), original_.end()};
            encoded_ = jh::serio::uri::encode(bytes);
        }

        void decode_uri_text()
        {
            const auto bytes = jh::serio::uri::decode(encoded_);
            recovered_.assign(bytes.begin(), bytes.end());
        }

        void encoded_text_matches(std::string_view expected, jh::test::cucumber::StepContext& context) const
        {
            context.expect(encoded_ == expected, "percent-encoded payload differs from the expected URI text");
        }

        void payload_matches(jh::test::cucumber::StepContext& context) const
        {
            context.expect(recovered_ == original_, "URI decoding changed the original payload bytes");
        }
    };

    using UriBinaryRoundtripDefinition = jh::test::cucumber::StepDefinition<
        UriBinaryRoundtripSteps,
        jh::test::cucumber::Given<
            "a byte payload represented by hexadecimal <string>", &UriBinaryRoundtripSteps::given_payload>,
        jh::test::cucumber::When<
            "the payload is percent-encoded", &UriBinaryRoundtripSteps::percent_encode_payload>,
        jh::test::cucumber::When<
            "the encoded URI text is decoded", &UriBinaryRoundtripSteps::decode_uri_text>,
        jh::test::cucumber::Then<
            "the encoded URI text is <string>", &UriBinaryRoundtripSteps::encoded_text_matches>,
        jh::test::cucumber::Then<
            "the recovered byte payload matches the original", &UriBinaryRoundtripSteps::payload_matches>
    >;
}
