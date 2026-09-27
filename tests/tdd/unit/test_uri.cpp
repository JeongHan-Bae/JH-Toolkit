#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

#include <random>
#include <stdexcept>
#include "jh/serio"


namespace test {
void tiny_test_case_1_body(const int selected_section) {

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<uint8_t> byte_dist(0, 255);
    std::uniform_int_distribution<size_t> len_dist(1, 256);

    constexpr int total_tests = 256;

    for (int i = 0; i < total_tests; ++i) {

        if (selected_section == 1) {

            const std::size_t len = len_dist(gen);

            std::vector<uint8_t> input(len);

            for (auto &b : input)
                b = byte_dist(gen);

            const auto bv = jh::pod::bytes_view::from(input);

            const auto raw_result = bv.fetch<const char>();
            jh::test::tiny_test::expect(static_cast<bool>((raw_result)), "raw_result");
            std::string raw(raw_result.value(), bv.size());

            const std::string encoded = jh::serio::uri::encode(raw);
            const std::string decoded = jh::serio::uri::decode(encoded);

            jh::test::tiny_test::expect(static_cast<bool>((decoded == raw)), "decoded == raw");
        }
    }

}
void tiny_test_case_1_section_1() { tiny_test_case_1_body(1); }
}
template<>
struct jh::test::tiny_test::test<"URI Encode/Decode Roundtrip / section 1">
    : jh::test::tiny_test::test_definition<"URI Encode/Decode Roundtrip / section 1", &::test::tiny_test_case_1_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_uri case 1">
    : jh::test::tiny_test::session_definition<
          "test module test_uri case 1", jh::test::tiny_test::test<"URI Encode/Decode Roundtrip / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_uri case 1"> registration_1{};
}



namespace test {
void tiny_test_case_2_body(const int selected_section) {

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

        if (selected_section == 1) {
            jh::test::tiny_test::expect(static_cast<bool>((jh::serio::uri::encode(p.raw) == p.encoded)), "jh::serio::uri::encode(p.raw) == p.encoded");
        }

        if (selected_section == 2) {
            jh::test::tiny_test::expect(static_cast<bool>((jh::serio::uri::decode(p.encoded) == p.raw)), "jh::serio::uri::decode(p.encoded) == p.raw");
        }
    }

}
void tiny_test_case_2_section_1() { tiny_test_case_2_body(1); }
void tiny_test_case_2_section_2() { tiny_test_case_2_body(2); }
}
template<>
struct jh::test::tiny_test::test<"URI Encode/Decode Known Pairs / section 1">
    : jh::test::tiny_test::test_definition<"URI Encode/Decode Known Pairs / section 1", &::test::tiny_test_case_2_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_uri case 2">
    : jh::test::tiny_test::session_definition<
          "test module test_uri case 2", jh::test::tiny_test::test<"URI Encode/Decode Known Pairs / section 1">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_uri case 2"> registration_2{};
}
template<>
struct jh::test::tiny_test::test<"URI Encode/Decode Known Pairs / section 2">
    : jh::test::tiny_test::test_definition<"URI Encode/Decode Known Pairs / section 2", &::test::tiny_test_case_2_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_uri case 3">
    : jh::test::tiny_test::session_definition<
          "test module test_uri case 3", jh::test::tiny_test::test<"URI Encode/Decode Known Pairs / section 2">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_uri case 3"> registration_3{};
}



namespace test {
void tiny_test_case_3_body(const int selected_section) {

    using namespace jh::serio::uri;

    if (selected_section == 1) {
        jh::test::tiny_test::expect_throw<std::runtime_error>([&]() { (void)(decode("%")); }, "throws std::runtime_error: decode(\"%\")");
        jh::test::tiny_test::expect_throw<std::runtime_error>([&]() { (void)(decode("%A")); }, "throws std::runtime_error: decode(\"%A\")");
        jh::test::tiny_test::expect_throw<std::runtime_error>([&]() { (void)(decode("abc%")); }, "throws std::runtime_error: decode(\"abc%\")");
    }

    if (selected_section == 2) {
        jh::test::tiny_test::expect_throw<std::runtime_error>([&]() { (void)(decode("%GG")); }, "throws std::runtime_error: decode(\"%GG\")");
        jh::test::tiny_test::expect_throw<std::runtime_error>([&]() { (void)(decode("%1G")); }, "throws std::runtime_error: decode(\"%1G\")");
        jh::test::tiny_test::expect_throw<std::runtime_error>([&]() { (void)(decode("%G1")); }, "throws std::runtime_error: decode(\"%G1\")");
    }

    if (selected_section == 3) {
        std::string bad = "abc";
        bad.push_back('\x01');
        bad += "def";

        jh::test::tiny_test::expect_throw<std::runtime_error>([&]() { (void)(decode(bad)); }, "throws std::runtime_error: decode(bad)");
    }

}
void tiny_test_case_3_section_1() { tiny_test_case_3_body(1); }
void tiny_test_case_3_section_2() { tiny_test_case_3_body(2); }
void tiny_test_case_3_section_3() { tiny_test_case_3_body(3); }
}
template<>
struct jh::test::tiny_test::test<"URI Invalid Percent Encoding Detection / Incomplete percent encoding">
    : jh::test::tiny_test::test_definition<"URI Invalid Percent Encoding Detection / Incomplete percent encoding", &::test::tiny_test_case_3_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_uri case 4">
    : jh::test::tiny_test::session_definition<
          "test module test_uri case 4", jh::test::tiny_test::test<"URI Invalid Percent Encoding Detection / Incomplete percent encoding">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_uri case 4"> registration_4{};
}
template<>
struct jh::test::tiny_test::test<"URI Invalid Percent Encoding Detection / Invalid hex digits">
    : jh::test::tiny_test::test_definition<"URI Invalid Percent Encoding Detection / Invalid hex digits", &::test::tiny_test_case_3_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_uri case 5">
    : jh::test::tiny_test::session_definition<
          "test module test_uri case 5", jh::test::tiny_test::test<"URI Invalid Percent Encoding Detection / Invalid hex digits">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_uri case 5"> registration_5{};
}
template<>
struct jh::test::tiny_test::test<"URI Invalid Percent Encoding Detection / Invalid URI characters">
    : jh::test::tiny_test::test_definition<"URI Invalid Percent Encoding Detection / Invalid URI characters", &::test::tiny_test_case_3_section_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_uri case 6">
    : jh::test::tiny_test::session_definition<
          "test module test_uri case 6", jh::test::tiny_test::test<"URI Invalid Percent Encoding Detection / Invalid URI characters">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_uri case 6"> registration_6{};
}



namespace test {
void tiny_test_case_4_body(const int selected_section) {

    using namespace jh::serio::uri;

    if (selected_section == 1) {

        std::string input = "Hello World";

        const auto encoded = encode_safe(input);
        const auto decoded = decode_safe(encoded);

        jh::test::tiny_test::expect(static_cast<bool>((decoded == input)), "decoded == input");
    }

    if (selected_section == 2) {

        std::string bad = "abc";
        bad.push_back('\x01');

        jh::test::tiny_test::expect_throw<std::runtime_error>([&]() { (void)(encode_safe(bad)); }, "throws std::runtime_error: encode_safe(bad)");
    }

    if (selected_section == 3) {

        std::string bad;
        bad.push_back(char(0xC3));
        bad.push_back(char(0x28)); // invalid UTF-8 sequence

        jh::test::tiny_test::expect_throw<std::runtime_error>([&]() { (void)(encode_safe(bad)); }, "throws std::runtime_error: encode_safe(bad)");
    }

}
void tiny_test_case_4_section_1() { tiny_test_case_4_body(1); }
void tiny_test_case_4_section_2() { tiny_test_case_4_body(2); }
void tiny_test_case_4_section_3() { tiny_test_case_4_body(3); }
}
template<>
struct jh::test::tiny_test::test<"URI URL-safe Encode legality checks / Valid UTF-8 passes">
    : jh::test::tiny_test::test_definition<"URI URL-safe Encode legality checks / Valid UTF-8 passes", &::test::tiny_test_case_4_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_uri case 7">
    : jh::test::tiny_test::session_definition<
          "test module test_uri case 7", jh::test::tiny_test::test<"URI URL-safe Encode legality checks / Valid UTF-8 passes">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_uri case 7"> registration_7{};
}
template<>
struct jh::test::tiny_test::test<"URI URL-safe Encode legality checks / Control characters rejected">
    : jh::test::tiny_test::test_definition<"URI URL-safe Encode legality checks / Control characters rejected", &::test::tiny_test_case_4_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_uri case 8">
    : jh::test::tiny_test::session_definition<
          "test module test_uri case 8", jh::test::tiny_test::test<"URI URL-safe Encode legality checks / Control characters rejected">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_uri case 8"> registration_8{};
}
template<>
struct jh::test::tiny_test::test<"URI URL-safe Encode legality checks / Illegal UTF-8 rejected">
    : jh::test::tiny_test::test_definition<"URI URL-safe Encode legality checks / Illegal UTF-8 rejected", &::test::tiny_test_case_4_section_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_uri case 9">
    : jh::test::tiny_test::session_definition<
          "test module test_uri case 9", jh::test::tiny_test::test<"URI URL-safe Encode legality checks / Illegal UTF-8 rejected">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_uri case 9"> registration_9{};
}



namespace test {
void tiny_test_case_5_body(const int selected_section) {
    using namespace jh::serio::uri;

    if (selected_section == 1) {
        jh::test::tiny_test::expect_throw<std::runtime_error>([&]() { (void)(decode_safe("%01")); }, "throws std::runtime_error: decode_safe(\"%01\")");
    }

}
void tiny_test_case_5_section_1() { tiny_test_case_5_body(1); }
}
template<>
struct jh::test::tiny_test::test<"URI safe decode rejects malformed or unsafe output / Decoded control characters are rejected">
    : jh::test::tiny_test::test_definition<"URI safe decode rejects malformed or unsafe output / Decoded control characters are rejected", &::test::tiny_test_case_5_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_uri case 10">
    : jh::test::tiny_test::session_definition<
          "test module test_uri case 10", jh::test::tiny_test::test<"URI safe decode rejects malformed or unsafe output / Decoded control characters are rejected">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_uri case 10"> registration_10{};
}



namespace test {
void tiny_test_case_6() {

    using namespace jh::serio::uri;

    jh::test::tiny_test::expect(static_cast<bool>((decode("Hello%20World") == "Hello World")), "decode(\"Hello%20World\") == \"Hello World\"");
    jh::test::tiny_test::expect(static_cast<bool>((decode("Hello%20world") == "Hello world")), "decode(\"Hello%20world\") == \"Hello world\"");

    jh::test::tiny_test::expect(static_cast<bool>((decode("%2f") == "/")), "decode(\"%2f\") == \"/\"");
    jh::test::tiny_test::expect(static_cast<bool>((decode("%2F") == "/")), "decode(\"%2F\") == \"/\"");

}
}
template<>
struct jh::test::tiny_test::test<"URI decode handles lowercase hex">
    : jh::test::tiny_test::test_definition<"URI decode handles lowercase hex", &::test::tiny_test_case_6> {};
template<>
struct jh::test::tiny_test::session<"test module test_uri case 11">
    : jh::test::tiny_test::session_definition<
          "test module test_uri case 11", jh::test::tiny_test::test<"URI decode handles lowercase hex">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_uri case 11"> registration_11{};
}



namespace test {
void tiny_test_case_7() {

    using namespace jh::serio::uri;

    std::string raw = std::string("A\0B",3);

    const auto encoded = encode(raw);
    const auto decoded = decode(encoded);

    jh::test::tiny_test::expect(static_cast<bool>((decoded.size() == raw.size())), "decoded.size() == raw.size()");
    jh::test::tiny_test::expect(static_cast<bool>((decoded == raw)), "decoded == raw");

}
}
template<>
struct jh::test::tiny_test::test<"URI decode with embedded nulls">
    : jh::test::tiny_test::test_definition<"URI decode with embedded nulls", &::test::tiny_test_case_7> {};
template<>
struct jh::test::tiny_test::session<"test module test_uri case 12">
    : jh::test::tiny_test::session_definition<
          "test module test_uri case 12", jh::test::tiny_test::test<"URI decode with embedded nulls">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_uri case 12"> registration_12{};
}

