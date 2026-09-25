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
    class HuffmanSteps final {
        using Codec = jh::serio::huffman<"bdd_huffman", jh::serio::huff_algo::huff256_canonical>;

        std::vector<std::uint8_t> original_;
        std::vector<std::uint8_t> recovered_;
        std::string compressed_;
        std::string foreign_compressed_;
        bool incompatible_stream_rejected_{};

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

        void given_empty_bytes() { original_.clear(); }

        void compress_data()
        {
            std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
            const std::string input{original_.begin(), original_.end()};
            Codec::compress(stream, input);
            compressed_ = stream.str();
        }

        void decompress_data()
        {
            std::stringstream stream(compressed_, std::ios::in | std::ios::binary);
            stream.seekg(0);
            const auto output = Codec::decompress(stream);
            recovered_.assign(output.begin(), output.end());
        }

        void bytes_match(jh::test::cucumber::StepContext& context) const
        {
            context.expect(recovered_ == original_, "Huffman decoding did not recover the input bytes");
        }

        void given_foreign_stream()
        {
            using OtherCodec = jh::serio::huffman<"bdd_other_huffman", jh::serio::huff_algo::huff256_canonical>;
            std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
            OtherCodec::compress(stream, "customer record");
            foreign_compressed_ = stream.str();
        }

        void decompress_foreign_stream()
        {
            std::stringstream stream(foreign_compressed_, std::ios::in | std::ios::binary);
            try {
                static_cast<void>(Codec::decompress(stream));
            } catch (const std::runtime_error&) {
                incompatible_stream_rejected_ = true;
            }
        }

        void current_codec_rejects_foreign_stream(jh::test::cucumber::StepContext& context) const
        {
            context.expect(incompatible_stream_rejected_, "a stream with a different signature should be rejected");
        }
    };

    using HuffmanDefinition = jh::test::cucumber::StepDefinition<
        HuffmanSteps,
        jh::test::cucumber::Given<
            "a byte sequence represented by hexadecimal <string>", &HuffmanSteps::given_bytes>,
        jh::test::cucumber::Given<
            "an empty byte sequence", &HuffmanSteps::given_empty_bytes>,
        jh::test::cucumber::Given<
            "a stream compressed for another application", &HuffmanSteps::given_foreign_stream>,
        jh::test::cucumber::When<
            "it is compressed with canonical Huffman coding", &HuffmanSteps::compress_data>,
        jh::test::cucumber::When<
            "the compressed Huffman stream is decompressed", &HuffmanSteps::decompress_data>,
        jh::test::cucumber::When<
            "the current application tries to decompress that stream", &HuffmanSteps::decompress_foreign_stream>,
        jh::test::cucumber::Then<
            "the recovered byte sequence matches the original", &HuffmanSteps::bytes_match>,
        jh::test::cucumber::Then<
            "the incompatible stream is rejected", &HuffmanSteps::current_codec_rejects_foreign_stream>
    >;
}
