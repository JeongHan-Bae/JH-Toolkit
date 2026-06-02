#include <random>
#include <stdexcept>
#include <catch2/catch_all.hpp>

#include "jh/serio"

TEST_CASE("URI Encode/Decode Roundtrip", "[uri]") {

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<uint8_t> byte_dist(0, 255);
    std::uniform_int_distribution<size_t> len_dist(1, 256);

    constexpr int total_tests = 256;

    for (int i = 0; i < total_tests; ++i) {

        SECTION("Randomized URI Test " + std::to_string(i + 1)) {

            const std::size_t len = len_dist(gen);

            std::vector<uint8_t> input(len);

            for (auto &b : input)
                b = byte_dist(gen);

            const auto bv = jh::pod::bytes_view::from(input);

            std::string raw(bv.fetch<const char>(), bv.size());

            const std::string encoded = jh::serio::uri::encode(raw);
            const std::string decoded = jh::serio::uri::decode(encoded);

            REQUIRE(decoded == raw);
        }
    }
}

TEST_CASE("URI Encode/Decode Known Pairs", "[uri]") {

    struct Pair {
        std::string raw;
        std::string encoded;
    };

    const std::vector<Pair> pairs = {

            {"", ""},

            {"abc", "abc"},

            {"Hello World", "Hello%20World"},

            {"Hello+World", "Hello%2BWorld"},

            {"a/b?c=d&e=f", "a%2Fb%3Fc%3Dd%26e%3Df"},

            {"100%", "100%25"},

            {"#fragment", "%23fragment"},

            {"A B C", "A%20B%20C"},
    };

    for (const auto &p : pairs) {

        SECTION("Encode matches expected: " + p.raw) {
            REQUIRE(jh::serio::uri::encode(p.raw) == p.encoded);
        }

        SECTION("Decode roundtrip: " + p.raw) {
            REQUIRE(jh::serio::uri::decode(p.encoded) == p.raw);
        }
    }
}

TEST_CASE("URI Invalid Percent Encoding Detection", "[uri][error]") {

    using namespace jh::serio::uri;

    SECTION("Incomplete percent encoding") {
        REQUIRE_THROWS_AS(decode("%"), std::runtime_error);
        REQUIRE_THROWS_AS(decode("%A"), std::runtime_error);
        REQUIRE_THROWS_AS(decode("abc%"), std::runtime_error);
    }

    SECTION("Invalid hex digits") {
        REQUIRE_THROWS_AS(decode("%GG"), std::runtime_error);
        REQUIRE_THROWS_AS(decode("%1G"), std::runtime_error);
        REQUIRE_THROWS_AS(decode("%G1"), std::runtime_error);
    }

    SECTION("Invalid URI characters") {
        std::string bad = "abc";
        bad.push_back('\x01');
        bad += "def";

        REQUIRE_THROWS_AS(decode(bad), std::runtime_error);
    }
}

TEST_CASE("URI URL-safe Encode legality checks", "[uri][url]") {

    using namespace jh::serio::uri;

    SECTION("Valid UTF-8 passes") {

        std::string input = "Hello World";

        const auto encoded = encode_safe(input);
        const auto decoded = decode_safe(encoded);

        REQUIRE(decoded == input);
    }

    SECTION("Control characters rejected") {

        std::string bad = "abc";
        bad.push_back('\x01');

        REQUIRE_THROWS_AS(encode_safe(bad), std::runtime_error);
    }

    SECTION("Illegal UTF-8 rejected") {

        std::string bad;
        bad.push_back(char(0xC3));
        bad.push_back(char(0x28)); // invalid UTF-8 sequence

        REQUIRE_THROWS_AS(encode_safe(bad), std::runtime_error);
    }
}

TEST_CASE("URI safe decode rejects malformed or unsafe output", "[uri][url][error]") {
    using namespace jh::serio::uri;

    SECTION("Decoded control characters are rejected") {
        REQUIRE_THROWS_AS(decode_safe("%01"), std::runtime_error);
    }
}

TEST_CASE("URI decode handles lowercase hex", "[uri]") {

    using namespace jh::serio::uri;

    REQUIRE(decode("Hello%20World") == "Hello World");
    REQUIRE(decode("Hello%20world") == "Hello world");

    REQUIRE(decode("%2f") == "/");
    REQUIRE(decode("%2F") == "/");
}

TEST_CASE("URI decode with embedded nulls", "[uri][edge]") {

    using namespace jh::serio::uri;

    std::string raw = std::string("A\0B",3);

    const auto encoded = encode(raw);
    const auto decoded = decode(encoded);

    REQUIRE(decoded.size() == raw.size());
    REQUIRE(decoded == raw);
}
