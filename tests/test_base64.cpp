#define CATCH_CONFIG_MAIN

#include <random>
#include <catch2/catch_all.hpp>
#include "jh/serio"
#include "jh/jindallae"

TEST_CASE("Base64 Encode/Decode Roundtrip", "[base64]") {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::uint8_t> dist(0, 255);
    std::uniform_int_distribution<std::size_t> len_dist(1, 256);

    constexpr int total_tests = 256;

    for (int i = 0; i < total_tests; ++i) {
        SECTION("Randomized Test " + std::to_string(i + 1)) {
            const std::size_t len = len_dist(gen);
            std::vector<std::uint8_t> input(len);
            for (auto &byte: input) {
                byte = dist(gen);
            }

            const std::string encoded = jh::serio::base64::encode(input.data(), input.size());
            const auto decoded = jh::serio::base64::decode(encoded);

            REQUIRE(decoded == input);
        }
    }
}

TEST_CASE("Base64URL Encode/Decode Roundtrip (with/without padding)", "[base64url]") {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::uint8_t> byte_dist(0, 255);
    std::uniform_int_distribution<std::size_t> len_dist(1, 256);
    std::bernoulli_distribution pad_dist(0.5);

    constexpr int total_tests = 256;

    for (int i = 0; i < total_tests; ++i) {
        SECTION("Randomized URL Test " + std::to_string(i + 1)) {
            const std::size_t len = len_dist(gen);
            std::vector<std::uint8_t> input(len);
            for (auto &byte: input)
                byte = byte_dist(gen);

            const bool pad = pad_dist(gen);
            const std::string encoded = jh::serio::base64url::encode(input.data(), input.size(), pad);
            const auto decoded = jh::serio::base64url::decode(encoded);

            REQUIRE(decoded == input);
        }
    }
}

TEST_CASE("Base64 / Base64URL Common Vectors", "[base64][base64url]") {
    struct TestVector {
        std::vector<std::uint8_t> bytes;
        std::string base64;
        std::string base64url;
        std::string base64url_nopad;
    };

    // obtained by python base64
    const std::vector<TestVector> vectors = {
            {{}, "",                         "",                         ""},
            {{0,   1,   2,   3,   4,   5,   6,   7, 8, 9, 10, 11, 12, 13, 14, 15},
                 "AAECAwQFBgcICQoLDA0ODw==", "AAECAwQFBgcICQoLDA0ODw==", "AAECAwQFBgcICQoLDA0ODw"},
            {{65,  66,  67,  68,  69,  70},
                 "QUJDREVG",                 "QUJDREVG",                 "QUJDREVG"},
            {{85,  170, 85,  170, 85,  170, 85,  170},
                 "VapVqlWqVao=",             "VapVqlWqVao=",             "VapVqlWqVao"},
            {{0,   0,   0,   0,   0,   0,   0,   0},
                 "AAAAAAAAAAA=",             "AAAAAAAAAAA=",             "AAAAAAAAAAA"},
            {{255, 255, 255, 255, 255, 255, 255, 255},
                 "//////////8=",             "__________8=",             "__________8"},
            {{0,   255, 0,   255, 0,   255, 0,   255},
                 "AP8A/wD/AP8=",             "AP8A_wD_AP8=",             "AP8A_wD_AP8"},
            {{72,  69,  76,  76,  79},
                 "SEVMTE8=",                 "SEVMTE8=",                 "SEVMTE8"},
            {{1,   2,   3,   4,   5,   6,   7,   8},
                 "AQIDBAUGBwg=",             "AQIDBAUGBwg=",             "AQIDBAUGBwg"},
    };

    for (const auto &v: vectors) {
        SECTION("Base64 encode matches expected") {
            REQUIRE(jh::serio::base64::encode(v.bytes.data(), v.bytes.size()) == v.base64);
        }SECTION("Base64 decode roundtrip") {
            const auto decoded = jh::serio::base64::decode(v.base64);
            REQUIRE(decoded == v.bytes);
        }SECTION("Base64URL encode matches expected (no pad)") {
            REQUIRE(jh::serio::base64url::encode(v.bytes.data(), v.bytes.size(), false) == v.base64url_nopad);
        }SECTION("Base64URL encode matches expected (with pad)") {
            REQUIRE(jh::serio::base64url::encode(v.bytes.data(), v.bytes.size(), true) == v.base64url);
        }SECTION("Base64URL decode roundtrip (no pad)") {
            const auto decoded = jh::serio::base64url::decode(v.base64url_nopad);
            REQUIRE(decoded == v.bytes);
        }SECTION("Base64URL decode roundtrip (with pad)") {
            const auto decoded = jh::serio::base64url::decode(v.base64url);
            REQUIRE(decoded == v.bytes);
        }
    }
}

TEST_CASE("Base64 invalid input detection", "[base64][error]") {
    using namespace jh::serio::base64;

    SECTION("Bad length") {
        REQUIRE_THROWS_AS(decode("A"), std::runtime_error);
        REQUIRE_THROWS_AS(decode("ABC"), std::runtime_error);
    }

    SECTION("Illegal characters") {
        REQUIRE_THROWS_AS(decode("AA$B=="), std::runtime_error);
        REQUIRE_THROWS_AS(decode("A@BC"), std::runtime_error);
    }

    SECTION("Bad padding") {
        REQUIRE_THROWS_AS(decode("AAAA==="), std::runtime_error);
        REQUIRE_THROWS_AS(decode("AAAAA="), std::runtime_error);
    }

    SECTION("Null bytes inside input") {
        std::string bad = {'A', 'B', '\0', 'C', 'D', '=', '='};
        REQUIRE_THROWS_AS(decode(bad), std::runtime_error);
    }
}

TEST_CASE("Base64 decode into user-provided buffer", "[base64][safety][buffer]") {
    using namespace jh::serio::base64;

    std::string out;
    auto view = decode("Qm9i", out); // "Bob"

    REQUIRE(out == "Bob");
    REQUIRE(view.len == out.size());
    REQUIRE(std::string(view.data, view.len) == "Bob");

    view = decode("TWFu", out); // "Man"
    REQUIRE(out == "Man");
    REQUIRE(std::string(view.data, view.len) == "Man");

    view = decode("QQ==", out); // "A"
    REQUIRE(out == "A");
    REQUIRE(std::string(view.data, view.len) == "A");
}

TEST_CASE("Base64URL decode into user-provided buffer", "[base64url][safety][buffer]") {
    using namespace jh::serio::base64url;

    std::string out;
    auto view = decode("SGVsbG8", out); // "Hello"
    REQUIRE(out == "Hello");
    REQUIRE(std::string(view.data, view.len) == "Hello");

    view = decode("QQ", out); // "A"
    REQUIRE(out == "A");
    REQUIRE(std::string(view.data, view.len) == "A");
}

TEST_CASE("Base64 decode into user-provided vector<uint8_t> buffer", "[base64][safety][buffer]") {
    using namespace jh::serio::base64;

    std::vector<uint8_t> out;
    auto view = decode("Qm9i", out); // "Bob"

    REQUIRE(out == std::vector<uint8_t>({'B', 'o', 'b'}));
    REQUIRE(view.len == out.size());
    REQUIRE(std::string(reinterpret_cast<const char *>(view.data), view.len) == "Bob");

    view = decode("TWFu", out); // "Man"
    REQUIRE(out == std::vector<uint8_t>({'M', 'a', 'n'}));
    REQUIRE(std::string(reinterpret_cast<const char *>(view.data), view.len) == "Man");

    view = decode("QQ==", out); // "A"
    REQUIRE(out == std::vector<uint8_t>({'A'}));
    REQUIRE(std::string(reinterpret_cast<const char *>(view.data), view.len) == "A");
}

TEST_CASE("Base64URL decode into user-provided vector<uint8_t> buffer", "[base64url][safety][buffer]") {
    using namespace jh::serio::base64url;

    std::vector<uint8_t> out;
    auto view = decode("SGVsbG8", out); // "Hello"
    REQUIRE(out == std::vector<uint8_t>({'H', 'e', 'l', 'l', 'o'}));
    REQUIRE(view.len == out.size());
    REQUIRE(std::string(reinterpret_cast<const char *>(view.data), view.len) == "Hello");

    view = decode("QQ", out); // "A"
    REQUIRE(out == std::vector<uint8_t>({'A'}));
    REQUIRE(std::string(reinterpret_cast<const char *>(view.data), view.len) == "A");
}

TEST_CASE("Compile-time Base64 / Base64URL correctness", "[constexpr][base64]") {

    SECTION("Base64 decode at compile time") {
        // "SGVsbG8=" -> "Hello"
        constexpr auto out = jh::jindallae::decode_base64<"SGVsbG8=">();
        STATIC_REQUIRE(out == jh::pod::array<uint8_t, 5>{{'H', 'e', 'l', 'l', 'o'}});
    }

    SECTION("Base64URL decode at compile time (with pad)") {
        // "SGVsbG8=" -> "Hello"
        constexpr auto out = jh::jindallae::decode_base64url<"SGVsbG8=">();
        STATIC_REQUIRE(out == jh::pod::array<uint8_t, 5>{{'H', 'e', 'l', 'l', 'o'}});
    }

    SECTION("Base64URL decode at compile time (no pad)") {
        // "SGVsbG8" -> "Hello"
        constexpr auto out = jh::jindallae::decode_base64url<"SGVsbG8">();
        STATIC_REQUIRE(out == jh::pod::array<uint8_t, 5>{{'H', 'e', 'l', 'l', 'o'}});
    }

    SECTION("Base64 encode at compile time") {
        constexpr jh::pod::array<uint8_t, 3> raw{{'H', 'i', '!'}};
        constexpr auto enc = jh::jindallae::encode_base64(raw);

        // "Hi!" -> "SGkh"
        STATIC_REQUIRE(enc == jh::jindallae::t_str<5>("SGkh"));
    }

    SECTION("Base64URL encode (no pad) at compile time") {
        constexpr jh::pod::array<uint8_t, 3> raw{{'H', 'i', '!'}};
        constexpr auto enc = jh::jindallae::encode_base64url(raw, std::false_type{});

        STATIC_REQUIRE(enc == jh::jindallae::t_str<5>("SGkh"));
    }

    SECTION("Base64URL encode (with pad) at compile time") {
        constexpr jh::pod::array<uint8_t, 3> raw{{'H', 'i', '!'}};
        constexpr auto enc = jh::jindallae::encode_base64url(raw, std::true_type{});

        STATIC_REQUIRE(enc == jh::jindallae::t_str<5>("SGkh"));
    }

    SECTION("Compile-time Base64 roundtrip") {
        constexpr auto decoded = jh::jindallae::decode_base64<"QUJD">(); // "ABC"
        constexpr auto encoded = jh::jindallae::encode_base64(decoded);

        STATIC_REQUIRE(encoded == jh::jindallae::t_str<5>("QUJD"));
    }

    SECTION("Compile-time Base64URL (no-pad) roundtrip") {
        constexpr auto decoded = jh::jindallae::decode_base64url<"QQ">(); // "A"
        constexpr auto encoded = jh::jindallae::encode_base64url(decoded);

        STATIC_REQUIRE(encoded == jh::jindallae::t_str<3>("QQ"));
    }

    SECTION("Compile-time (string → bytes → base64 → bytes → string) roundtrip") {
        // original "Hello"
        constexpr jh::jindallae::t_str str{"Hello"};

        // string → bytes
        constexpr auto bytes = str.to_bytes();

        // bytes → base64 literal
        constexpr auto encoded = jh::jindallae::encode_base64(bytes);

        // base64 → bytes
        constexpr auto decoded = jh::jindallae::decode_base64<encoded.storage.data>();

        // bytes → t_str
        constexpr auto restored = jh::jindallae::t_str<decoded.size() + 1>::from_bytes(decoded);

        STATIC_REQUIRE(restored == str);
    }

    SECTION("Compile-time (base64 literal → bytes → string → bytes → base64 literal) roundtrip") {
        // "SGVsbG8=" is "Hello"
        constexpr auto bytes = jh::jindallae::decode_base64<"SGVsbG8=">();

        // bytes → compile-time t_str<6>("Hello\0")
        constexpr auto str = jh::jindallae::t_str<bytes.size() + 1>::from_bytes(bytes);

        // back to bytes
        constexpr auto again_bytes = str.to_bytes();

        // base64 encode again
        constexpr auto encoded2 = jh::jindallae::encode_base64(again_bytes);

        STATIC_REQUIRE(encoded2 == jh::jindallae::t_str("SGVsbG8="));
    }
}
