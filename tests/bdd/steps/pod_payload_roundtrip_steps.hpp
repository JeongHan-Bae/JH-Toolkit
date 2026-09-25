#pragma once

#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>

#include "jh/pod"
#include "jh/serio"
#include "jh/test/cucumber/cucumber.hpp"

namespace jh_toolkit_bdd {
    class PodPayloadSteps final {
        jh::pod::array<std::uint32_t, 2> original_{};
        jh::pod::array<std::uint32_t, 2> recovered_{};
        std::string bytes_;
        std::string compressed_;
        std::string decompressed_;

    public:
        void given_values(const std::uint64_t first, const std::uint64_t second)
        {
            original_ = {{static_cast<std::uint32_t>(first), static_cast<std::uint32_t>(second)}};
        }

        void represent_as_bytes()
        {
            const auto source = jh::pod::bytes_view::from(original_.data, original_.size());
            const auto source_bytes = source.fetch<char>();
            if (!source_bytes) throw std::runtime_error("POD pair could not be viewed as bytes");
            bytes_.assign(source_bytes.value(), source.len);
        }

        void compress_payload()
        {
            using Codec = jh::serio::huffman<"bdd_pod_payload", jh::serio::huff_algo::huff256_canonical>;
            std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
            Codec::compress(stream, bytes_);
            compressed_ = stream.str();
        }

        void decompress_payload()
        {
            using Codec = jh::serio::huffman<"bdd_pod_payload", jh::serio::huff_algo::huff256_canonical>;
            std::stringstream stream(compressed_, std::ios::in | std::ios::binary);
            decompressed_ = Codec::decompress(stream);
        }

        void interpret_recovered_bytes()
        {
            const auto restored = jh::pod::bytes_view::from(decompressed_.data(), decompressed_.size())
                .fetch<jh::pod::array<std::uint32_t, 2>>();
            if (!restored) throw std::runtime_error("decompressed bytes do not contain a POD pair");
            recovered_ = *restored.value();
        }

        void values_match(jh::test::cucumber::StepContext& context) const
        {
            context.expect(recovered_[0] == original_[0] && recovered_[1] == original_[1],
                           "compression did not restore both POD values");
        }
    };

    using PodPayloadDefinition = jh::test::cucumber::StepDefinition<
        PodPayloadSteps,
        jh::test::cucumber::Given<
            "a POD pair with values <uint> and <uint>", &PodPayloadSteps::given_values>,
        jh::test::cucumber::When<
            "the POD pair is represented as bytes", &PodPayloadSteps::represent_as_bytes>,
        jh::test::cucumber::When<
            "the byte payload is compressed with canonical Huffman coding", &PodPayloadSteps::compress_payload>,
        jh::test::cucumber::When<
            "the compressed payload is decompressed", &PodPayloadSteps::decompress_payload>,
        jh::test::cucumber::When<
            "the recovered bytes are interpreted as the original POD pair", &PodPayloadSteps::interpret_recovered_bytes>,
        jh::test::cucumber::Then<
            "both values in the recovered POD pair match the original", &PodPayloadSteps::values_match>
    >;
}
