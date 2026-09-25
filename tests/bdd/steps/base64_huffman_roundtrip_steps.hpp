#pragma once

#include <charconv>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "jh/serio"
#include "jh/test/cucumber/cucumber.hpp"

namespace jh_toolkit_bdd {
    class Base64HuffmanSteps final {
        std::vector<std::uint8_t> original_;
        std::vector<std::uint8_t> recovered_;
        std::string base64_text_;
        std::string compressed_;
        std::string decompressed_text_;

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
        void given_bytes(std::string_view hex) { original_ = parse_hex(hex); }

        void encode_with_base64()
        {
            base64_text_ = jh::serio::base64::encode(original_.data(), original_.size());
        }

        void compress_base64_text()
        {
            using Codec = jh::serio::huffman<"bdd_base64_huffman", jh::serio::huff_algo::huff128_canonical>;
            std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
            Codec::compress(stream, base64_text_);
            compressed_ = stream.str();
        }

        void decompress_data()
        {
            using Codec = jh::serio::huffman<"bdd_base64_huffman", jh::serio::huff_algo::huff128_canonical>;
            std::stringstream stream(compressed_, std::ios::in | std::ios::binary);
            decompressed_text_ = Codec::decompress(stream);
        }

        void decode_base64()
        {
            recovered_ = jh::serio::base64::decode(decompressed_text_);
        }

        void bytes_match(jh::test::cucumber::StepContext& context) const
        {
            context.expect(recovered_ == original_, "combined encoding did not recover the input bytes");
        }
    };

    using Base64HuffmanDefinition = jh::test::cucumber::StepDefinition<
        Base64HuffmanSteps,
        jh::test::cucumber::Given<
            "a byte sequence represented by hexadecimal <string>", &Base64HuffmanSteps::given_bytes>,
        jh::test::cucumber::When<
            "it is encoded with Base64", &Base64HuffmanSteps::encode_with_base64>,
        jh::test::cucumber::When<
            "the Base64 text is compressed with canonical Huffman coding",
            &Base64HuffmanSteps::compress_base64_text>,
        jh::test::cucumber::When<
            "the compressed data is decompressed", &Base64HuffmanSteps::decompress_data>,
        jh::test::cucumber::When<
            "the recovered text is decoded from Base64", &Base64HuffmanSteps::decode_base64>,
        jh::test::cucumber::Then<
            "the recovered byte sequence matches the original", &Base64HuffmanSteps::bytes_match>
    >;
}
