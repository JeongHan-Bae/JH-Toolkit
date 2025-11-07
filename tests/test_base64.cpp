#define CATCH_CONFIG_MAIN
#include <random>
#include <catch2/catch_all.hpp>
#include "jh/utils/base64.h"
#include "jh/jindallae"

TEST_CASE("Base64 Encode/Decode Roundtrip", "[base64]") {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::uint8_t> dist(0, 255);
    std::uniform_int_distribution<std::size_t> len_dist(1, 256);

    constexpr int total_tests = 128;

    for (int i = 0; i < total_tests; ++i) {
        SECTION("Randomized Test " + std::to_string(i + 1)) {
            const std::size_t len = len_dist(gen);
            std::vector<std::uint8_t> input(len);
            for (auto &byte: input) {
                byte = dist(gen);
            }

            const std::string encoded = jh::utils::base64::encode(input.data(), input.size());
            const auto decoded = jh::utils::base64::decode(encoded);

            REQUIRE(decoded == input);
        }
    }
}