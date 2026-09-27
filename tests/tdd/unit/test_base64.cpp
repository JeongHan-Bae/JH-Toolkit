#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

#include <random>
#include <stdexcept>
#include "jh/serio"
#include "jh/jindallae"


namespace test {
void tiny_test_case_1_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::uint8_t> dist(0, 255);
    std::uniform_int_distribution<std::size_t> len_dist(1, 256);

    constexpr int total_tests = 256;

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            const std::size_t len = len_dist(gen);
            std::vector<std::uint8_t> input(len);
            for (auto &byte: input) {
                byte = dist(gen);
            }

            const std::string encoded = jh::serio::base64::encode(input.data(), input.size());
            const auto decoded = jh::serio::base64::decode(encoded);

            jh::test::tiny_test::expect(static_cast<bool>((decoded == input)), "decoded == input");
        }
    }

}
void tiny_test_case_1_section_1() { tiny_test_case_1_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Base64 Encode/Decode Roundtrip / section 1">
    : jh::test::tiny_test::test_definition<"Base64 Encode/Decode Roundtrip / section 1", &::test::tiny_test_case_1_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_base64 case 1">
    : jh::test::tiny_test::session_definition<
          "test module test_base64 case 1", jh::test::tiny_test::test<"Base64 Encode/Decode Roundtrip / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_base64 case 1"> registration_1{};
}



namespace test {
void tiny_test_case_2_body(const int selected_section) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::uint8_t> byte_dist(0, 255);
    std::uniform_int_distribution<std::size_t> len_dist(1, 256);
    std::bernoulli_distribution pad_dist(0.5);

    constexpr int total_tests = 256;

    for (int i = 0; i < total_tests; ++i) {
        if (selected_section == 1) {
            const std::size_t len = len_dist(gen);
            std::vector<std::uint8_t> input(len);
            for (auto &byte: input)
                byte = byte_dist(gen);

            const bool pad = pad_dist(gen);
            const std::string encoded = jh::serio::base64url::encode(input.data(), input.size(), pad);
            const auto decoded = jh::serio::base64url::decode(encoded);

            jh::test::tiny_test::expect(static_cast<bool>((decoded == input)), "decoded == input");
        }
    }

}
void tiny_test_case_2_section_1() { tiny_test_case_2_body(1); }
}
template<>
struct jh::test::tiny_test::test<"Base64URL Encode/Decode Roundtrip (with/without padding) / section 1">
    : jh::test::tiny_test::test_definition<"Base64URL Encode/Decode Roundtrip (with/without padding) / section 1", &::test::tiny_test_case_2_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_base64 case 2">
    : jh::test::tiny_test::session_definition<
          "test module test_base64 case 2", jh::test::tiny_test::test<"Base64URL Encode/Decode Roundtrip (with/without padding) / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_base64 case 2"> registration_2{};
}



namespace test {
void tiny_test_case_3_body(const int selected_section) {
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
        if (selected_section == 1) {
            jh::test::tiny_test::expect(static_cast<bool>((jh::serio::base64::encode(v.bytes.data(), v.bytes.size()) == v.base64)), "jh::serio::base64::encode(v.bytes.data(), v.bytes.size()) == v.base64");
        }if (selected_section == 2) {
            const auto decoded = jh::serio::base64::decode(v.base64);
            jh::test::tiny_test::expect(static_cast<bool>((decoded == v.bytes)), "decoded == v.bytes");
        }if (selected_section == 3) {
            jh::test::tiny_test::expect(static_cast<bool>((jh::serio::base64url::encode(v.bytes.data(), v.bytes.size(), false) == v.base64url_nopad)), "jh::serio::base64url::encode(v.bytes.data(), v.bytes.size(), false) == v.base64url_nopad");
        }if (selected_section == 4) {
            jh::test::tiny_test::expect(static_cast<bool>((jh::serio::base64url::encode(v.bytes.data(), v.bytes.size(), true) == v.base64url)), "jh::serio::base64url::encode(v.bytes.data(), v.bytes.size(), true) == v.base64url");
        }if (selected_section == 5) {
            const auto decoded = jh::serio::base64url::decode(v.base64url_nopad);
            jh::test::tiny_test::expect(static_cast<bool>((decoded == v.bytes)), "decoded == v.bytes");
        }if (selected_section == 6) {
            const auto decoded = jh::serio::base64url::decode(v.base64url);
            jh::test::tiny_test::expect(static_cast<bool>((decoded == v.bytes)), "decoded == v.bytes");
        }
    }

}
void tiny_test_case_3_section_1() { tiny_test_case_3_body(1); }
void tiny_test_case_3_section_2() { tiny_test_case_3_body(2); }
void tiny_test_case_3_section_3() { tiny_test_case_3_body(3); }
void tiny_test_case_3_section_4() { tiny_test_case_3_body(4); }
void tiny_test_case_3_section_5() { tiny_test_case_3_body(5); }
void tiny_test_case_3_section_6() { tiny_test_case_3_body(6); }
}
template<>
struct jh::test::tiny_test::test<"Base64 / Base64URL Common Vectors / Base64 encode matches expected">
    : jh::test::tiny_test::test_definition<"Base64 / Base64URL Common Vectors / Base64 encode matches expected", &::test::tiny_test_case_3_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_base64 case 3">
    : jh::test::tiny_test::session_definition<
          "test module test_base64 case 3", jh::test::tiny_test::test<"Base64 / Base64URL Common Vectors / Base64 encode matches expected">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_base64 case 3"> registration_3{};
}
template<>
struct jh::test::tiny_test::test<"Base64 / Base64URL Common Vectors / Base64 decode roundtrip">
    : jh::test::tiny_test::test_definition<"Base64 / Base64URL Common Vectors / Base64 decode roundtrip", &::test::tiny_test_case_3_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_base64 case 4">
    : jh::test::tiny_test::session_definition<
          "test module test_base64 case 4", jh::test::tiny_test::test<"Base64 / Base64URL Common Vectors / Base64 decode roundtrip">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_base64 case 4"> registration_4{};
}
template<>
struct jh::test::tiny_test::test<"Base64 / Base64URL Common Vectors / Base64URL encode matches expected (no pad)">
    : jh::test::tiny_test::test_definition<"Base64 / Base64URL Common Vectors / Base64URL encode matches expected (no pad)", &::test::tiny_test_case_3_section_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_base64 case 5">
    : jh::test::tiny_test::session_definition<
          "test module test_base64 case 5", jh::test::tiny_test::test<"Base64 / Base64URL Common Vectors / Base64URL encode matches expected (no pad)">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_base64 case 5"> registration_5{};
}
template<>
struct jh::test::tiny_test::test<"Base64 / Base64URL Common Vectors / Base64URL encode matches expected (with pad)">
    : jh::test::tiny_test::test_definition<"Base64 / Base64URL Common Vectors / Base64URL encode matches expected (with pad)", &::test::tiny_test_case_3_section_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_base64 case 6">
    : jh::test::tiny_test::session_definition<
          "test module test_base64 case 6", jh::test::tiny_test::test<"Base64 / Base64URL Common Vectors / Base64URL encode matches expected (with pad)">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_base64 case 6"> registration_6{};
}
template<>
struct jh::test::tiny_test::test<"Base64 / Base64URL Common Vectors / Base64URL decode roundtrip (no pad)">
    : jh::test::tiny_test::test_definition<"Base64 / Base64URL Common Vectors / Base64URL decode roundtrip (no pad)", &::test::tiny_test_case_3_section_5> {};
template<>
struct jh::test::tiny_test::session<"test module test_base64 case 7">
    : jh::test::tiny_test::session_definition<
          "test module test_base64 case 7", jh::test::tiny_test::test<"Base64 / Base64URL Common Vectors / Base64URL decode roundtrip (no pad)">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_base64 case 7"> registration_7{};
}
template<>
struct jh::test::tiny_test::test<"Base64 / Base64URL Common Vectors / Base64URL decode roundtrip (with pad)">
    : jh::test::tiny_test::test_definition<"Base64 / Base64URL Common Vectors / Base64URL decode roundtrip (with pad)", &::test::tiny_test_case_3_section_6> {};
template<>
struct jh::test::tiny_test::session<"test module test_base64 case 8">
    : jh::test::tiny_test::session_definition<
          "test module test_base64 case 8", jh::test::tiny_test::test<"Base64 / Base64URL Common Vectors / Base64URL decode roundtrip (with pad)">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_base64 case 8"> registration_8{};
}



namespace test {
void tiny_test_case_4_body(const int selected_section) {
    using namespace jh::serio::base64;

    if (selected_section == 1) {
        jh::test::tiny_test::expect_throw<std::runtime_error>([&]() { (void)(decode("A")); }, "throws std::runtime_error: decode(\"A\")");
        jh::test::tiny_test::expect_throw<std::runtime_error>([&]() { (void)(decode("ABC")); }, "throws std::runtime_error: decode(\"ABC\")");
    }

    if (selected_section == 2) {
        jh::test::tiny_test::expect_throw<std::runtime_error>([&]() { (void)(decode("AA$B==")); }, "throws std::runtime_error: decode(\"AA$B==\")");
        jh::test::tiny_test::expect_throw<std::runtime_error>([&]() { (void)(decode("A@BC")); }, "throws std::runtime_error: decode(\"A@BC\")");
    }

    if (selected_section == 3) {
        jh::test::tiny_test::expect_throw<std::runtime_error>([&]() { (void)(decode("AAAA===")); }, "throws std::runtime_error: decode(\"AAAA===\")");
        jh::test::tiny_test::expect_throw<std::runtime_error>([&]() { (void)(decode("AAAAA=")); }, "throws std::runtime_error: decode(\"AAAAA=\")");
    }

    if (selected_section == 4) {
        std::string bad = {'A', 'B', '\0', 'C', 'D', '=', '='};
        jh::test::tiny_test::expect_throw<std::runtime_error>([&]() { (void)(decode(bad)); }, "throws std::runtime_error: decode(bad)");
    }

}
void tiny_test_case_4_section_1() { tiny_test_case_4_body(1); }
void tiny_test_case_4_section_2() { tiny_test_case_4_body(2); }
void tiny_test_case_4_section_3() { tiny_test_case_4_body(3); }
void tiny_test_case_4_section_4() { tiny_test_case_4_body(4); }
}
template<>
struct jh::test::tiny_test::test<"Base64 invalid input detection / Bad length">
    : jh::test::tiny_test::test_definition<"Base64 invalid input detection / Bad length", &::test::tiny_test_case_4_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_base64 case 9">
    : jh::test::tiny_test::session_definition<
          "test module test_base64 case 9", jh::test::tiny_test::test<"Base64 invalid input detection / Bad length">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_base64 case 9"> registration_9{};
}
template<>
struct jh::test::tiny_test::test<"Base64 invalid input detection / Illegal characters">
    : jh::test::tiny_test::test_definition<"Base64 invalid input detection / Illegal characters", &::test::tiny_test_case_4_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_base64 case 10">
    : jh::test::tiny_test::session_definition<
          "test module test_base64 case 10", jh::test::tiny_test::test<"Base64 invalid input detection / Illegal characters">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_base64 case 10"> registration_10{};
}
template<>
struct jh::test::tiny_test::test<"Base64 invalid input detection / Bad padding">
    : jh::test::tiny_test::test_definition<"Base64 invalid input detection / Bad padding", &::test::tiny_test_case_4_section_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_base64 case 11">
    : jh::test::tiny_test::session_definition<
          "test module test_base64 case 11", jh::test::tiny_test::test<"Base64 invalid input detection / Bad padding">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_base64 case 11"> registration_11{};
}
template<>
struct jh::test::tiny_test::test<"Base64 invalid input detection / Null bytes inside input">
    : jh::test::tiny_test::test_definition<"Base64 invalid input detection / Null bytes inside input", &::test::tiny_test_case_4_section_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_base64 case 12">
    : jh::test::tiny_test::session_definition<
          "test module test_base64 case 12", jh::test::tiny_test::test<"Base64 invalid input detection / Null bytes inside input">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_base64 case 12"> registration_12{};
}



namespace test {
void tiny_test_case_5() {
    using namespace jh::serio::base64;

    std::string out;
    auto view = decode("Qm9i", out); // "Bob"

    jh::test::tiny_test::expect(static_cast<bool>((out == "Bob")), "out == \"Bob\"");
    jh::test::tiny_test::expect(static_cast<bool>((view.size() == out.size())), "view.size() == out.size()");
    jh::test::tiny_test::expect(static_cast<bool>((std::string(view.data, view.size()) == "Bob")), "std::string(view.data, view.size()) == \"Bob\"");

    view = decode("TWFu", out); // "Man"
    jh::test::tiny_test::expect(static_cast<bool>((out == "Man")), "out == \"Man\"");
    jh::test::tiny_test::expect(static_cast<bool>((std::string(view.data, view.size()) == "Man")), "std::string(view.data, view.size()) == \"Man\"");

    view = decode("QQ==", out); // "A"
    jh::test::tiny_test::expect(static_cast<bool>((out == "A")), "out == \"A\"");
    jh::test::tiny_test::expect(static_cast<bool>((std::string(view.data, view.size()) == "A")), "std::string(view.data, view.size()) == \"A\"");

}
}
template<>
struct jh::test::tiny_test::test<"Base64 decode into user-provided buffer">
    : jh::test::tiny_test::test_definition<"Base64 decode into user-provided buffer", &::test::tiny_test_case_5> {};
template<>
struct jh::test::tiny_test::session<"test module test_base64 case 13">
    : jh::test::tiny_test::session_definition<
          "test module test_base64 case 13", jh::test::tiny_test::test<"Base64 decode into user-provided buffer">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_base64 case 13"> registration_13{};
}



namespace test {
void tiny_test_case_6() {
    using namespace jh::serio::base64url;

    std::string out;
    auto view = decode("SGVsbG8", out); // "Hello"
    jh::test::tiny_test::expect(static_cast<bool>((out == "Hello")), "out == \"Hello\"");
    jh::test::tiny_test::expect(static_cast<bool>((std::string(view.data, view.size()) == "Hello")), "std::string(view.data, view.size()) == \"Hello\"");

    view = decode("QQ", out); // "A"
    jh::test::tiny_test::expect(static_cast<bool>((out == "A")), "out == \"A\"");
    jh::test::tiny_test::expect(static_cast<bool>((std::string(view.data, view.size()) == "A")), "std::string(view.data, view.size()) == \"A\"");

}
}
template<>
struct jh::test::tiny_test::test<"Base64URL decode into user-provided buffer">
    : jh::test::tiny_test::test_definition<"Base64URL decode into user-provided buffer", &::test::tiny_test_case_6> {};
template<>
struct jh::test::tiny_test::session<"test module test_base64 case 14">
    : jh::test::tiny_test::session_definition<
          "test module test_base64 case 14", jh::test::tiny_test::test<"Base64URL decode into user-provided buffer">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_base64 case 14"> registration_14{};
}



namespace test {
void tiny_test_case_7() {
    using namespace jh::serio::base64;

    std::vector<std::uint8_t> out;
    auto view = decode("Qm9i", out); // "Bob"

    jh::test::tiny_test::expect(static_cast<bool>((out == std::vector<std::uint8_t>({'B', 'o', 'b'}))), "out == std::vector<std::uint8_t>({'B', 'o', 'b'})");
    jh::test::tiny_test::expect(static_cast<bool>((view.size() == out.size())), "view.size() == out.size()");
    auto data = view.fetch<const char>();
    jh::test::tiny_test::expect(static_cast<bool>((data)), "data");
    jh::test::tiny_test::expect(static_cast<bool>((std::string(data.value(), view.size()) == "Bob")), "std::string(data.value(), view.size()) == \"Bob\"");

    view = decode("TWFu", out); // "Man"
    jh::test::tiny_test::expect(static_cast<bool>((out == std::vector<std::uint8_t>({'M', 'a', 'n'}))), "out == std::vector<std::uint8_t>({'M', 'a', 'n'})");
    data = view.fetch<const char>();
    jh::test::tiny_test::expect(static_cast<bool>((data)), "data");
    jh::test::tiny_test::expect(static_cast<bool>((std::string(data.value(), view.size()) == "Man")), "std::string(data.value(), view.size()) == \"Man\"");

    view = decode("QQ==", out); // "A"
    jh::test::tiny_test::expect(static_cast<bool>((out == std::vector<std::uint8_t>({'A'}))), "out == std::vector<std::uint8_t>({'A'})");
    data = view.fetch<const char>();
    jh::test::tiny_test::expect(static_cast<bool>((data)), "data");
    jh::test::tiny_test::expect(static_cast<bool>((std::string(data.value(), view.size()) == "A")), "std::string(data.value(), view.size()) == \"A\"");

}
}
template<>
struct jh::test::tiny_test::test<"Base64 decode into user-provided vector<uint8_t> buffer">
    : jh::test::tiny_test::test_definition<"Base64 decode into user-provided vector<uint8_t> buffer", &::test::tiny_test_case_7> {};
template<>
struct jh::test::tiny_test::session<"test module test_base64 case 15">
    : jh::test::tiny_test::session_definition<
          "test module test_base64 case 15", jh::test::tiny_test::test<"Base64 decode into user-provided vector<uint8_t> buffer">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_base64 case 15"> registration_15{};
}



namespace test {
void tiny_test_case_8() {
    const std::uint8_t *null_data = nullptr;

    jh::test::tiny_test::expect_throw<std::invalid_argument>([&]() { (void)(jh::serio::base64::encode(null_data, 1)); }, "throws std::invalid_argument: jh::serio::base64::encode(null_data, 1)");
    jh::test::tiny_test::expect_throw<std::invalid_argument>([&]() { (void)(jh::serio::base64url::encode(null_data, 1)); }, "throws std::invalid_argument: jh::serio::base64url::encode(null_data, 1)");

}
}
template<>
struct jh::test::tiny_test::test<"Base64 encode rejects null input when length is non-zero">
    : jh::test::tiny_test::test_definition<"Base64 encode rejects null input when length is non-zero", &::test::tiny_test_case_8> {};
template<>
struct jh::test::tiny_test::session<"test module test_base64 case 16">
    : jh::test::tiny_test::session_definition<
          "test module test_base64 case 16", jh::test::tiny_test::test<"Base64 encode rejects null input when length is non-zero">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_base64 case 16"> registration_16{};
}



namespace test {
void tiny_test_case_9() {
    using namespace jh::serio::base64url;

    std::vector<std::uint8_t> out;
    auto view = decode("SGVsbG8", out); // "Hello"
    jh::test::tiny_test::expect(static_cast<bool>((out == std::vector<std::uint8_t>({'H', 'e', 'l', 'l', 'o'}))), "out == std::vector<std::uint8_t>({'H', 'e', 'l', 'l', 'o'})");
    jh::test::tiny_test::expect(static_cast<bool>((view.size() == out.size())), "view.size() == out.size()");
    auto data = view.fetch<const char>();
    jh::test::tiny_test::expect(static_cast<bool>((data)), "data");
    jh::test::tiny_test::expect(static_cast<bool>((std::string(data.value(), view.size()) == "Hello")), "std::string(data.value(), view.size()) == \"Hello\"");

    view = decode("QQ", out); // "A"
    jh::test::tiny_test::expect(static_cast<bool>((out == std::vector<std::uint8_t>({'A'}))), "out == std::vector<std::uint8_t>({'A'})");
    data = view.fetch<const char>();
    jh::test::tiny_test::expect(static_cast<bool>((data)), "data");
    jh::test::tiny_test::expect(static_cast<bool>((std::string(data.value(), view.size()) == "A")), "std::string(data.value(), view.size()) == \"A\"");

}
}
template<>
struct jh::test::tiny_test::test<"Base64URL decode into user-provided vector<uint8_t> buffer">
    : jh::test::tiny_test::test_definition<"Base64URL decode into user-provided vector<uint8_t> buffer", &::test::tiny_test_case_9> {};
template<>
struct jh::test::tiny_test::session<"test module test_base64 case 17">
    : jh::test::tiny_test::session_definition<
          "test module test_base64 case 17", jh::test::tiny_test::test<"Base64URL decode into user-provided vector<uint8_t> buffer">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_base64 case 17"> registration_17{};
}

