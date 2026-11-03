#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"

#include "jh/serio"

#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

namespace test {
    struct dual_protocol {
        int source = 0;
        dual_protocol() = default;
        explicit dual_protocol(const std::string &) : source(1) {}
        explicit operator std::string() const { return "string"; }
    };

    std::ostream &operator<<(std::ostream &out, const dual_protocol &) {
        return out << "stream";
    }

    std::istream &operator>>(std::istream &in, dual_protocol &value) {
        value.source = 2;
        return in;
    }

    struct immovable_dual_protocol {
        int source = 0;
        immovable_dual_protocol() = default;
        explicit immovable_dual_protocol(const std::string &) : source(1) {}
        immovable_dual_protocol(const immovable_dual_protocol &) = delete;
        immovable_dual_protocol(immovable_dual_protocol &&) = delete;
        explicit operator std::string() const { return "string"; }
    };

    std::ostream &operator<<(std::ostream &out, const immovable_dual_protocol &) {
        return out << "stream";
    }

    std::istream &operator>>(std::istream &in, immovable_dual_protocol &value) {
        value.source = 2;
        return in;
    }

    struct immovable_stream_value {
        int value = 0;
        immovable_stream_value() = default;
        immovable_stream_value(const immovable_stream_value &) = delete;
        immovable_stream_value(immovable_stream_value &&) = delete;
    };

    std::ostream &operator<<(std::ostream &out, const immovable_stream_value &value) {
        return out << value.value;
    }

    std::istream &operator>>(std::istream &in, immovable_stream_value &value) {
        return in >> value.value;
    }

    struct json_view_like {
        std::string_view text;

        explicit json_view_like(const std::string &source) : text(source) {}
        json_view_like(const json_view_like &) = delete;
        json_view_like(json_view_like &&) = delete;
        explicit operator std::string() const { return std::string{text}; }
    };

    void test_string_protocol() {
        const std::string source = "borrowed key";
        const std::string_view borrowed = jh::serio::deserialize<std::string_view>(source);
        jh::test::tiny_test::expect(borrowed.data() == source.data() &&
                                   borrowed == source &&
                                   jh::serio::serialize(borrowed) == source,
                                   "String protocol borrows a live const string reference");

        jh::test::tiny_test::expect(jh::serio::serialize(dual_protocol{}) == "string" &&
                                   jh::serio::deserialize<dual_protocol>(source).source == 1,
                                   "complete String protocol wins over Stream protocol");

        const auto immovable = jh::serio::deserialize<immovable_dual_protocol>(source);
        jh::test::tiny_test::expect(immovable.source == 1 &&
                                   jh::serio::serialize(immovable) == "string",
                                   "String construction supports an immovable dual-protocol type");
    }

    void test_stream_protocol() {
        jh::test::tiny_test::expect(jh::serio::serialize(42) == "42" &&
                                   jh::serio::deserialize<int>("42 \t") == 42,
                                   "Stream protocol returns a string and consumes its input stream");
        jh::test::tiny_test::expect_throw<std::runtime_error>([&] {
            (void)jh::serio::deserialize<int>("42x");
        }, "Stream protocol rejects trailing content");

        immovable_stream_value extracted;
        const std::string source = "19";
        jh::serio::deserialize(source, extracted);
        jh::test::tiny_test::expect(extracted.value == 19 && source == "19",
                                   "Stream extraction supports an immovable value without changing source text");
    }

    void test_view_containing_roundtrip() {
        static_assert(!std::is_move_constructible_v<json_view_like>);
        static_assert(jh::concepts::serializable<json_view_like>);

        const std::string source = R"({"key":"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"})";
        const json_view_like original{source};
        const std::string encoded = jh::serio::serialize(original);
        const auto restored = jh::serio::deserialize<json_view_like>(encoded);
        jh::test::tiny_test::expect(encoded == source &&
                                   restored.text.data() == encoded.data() &&
                                   restored.text == source,
                                   "immovable view-containing value borrows the live serialized string");
    }

    void run_value_conversion_tests() {
        test_string_protocol();
        test_stream_protocol();
        test_view_containing_roundtrip();
    }
}

template<>
struct jh::test::tiny_test::test<"value conversion protocols">
        : jh::test::tiny_test::test_definition<"value conversion protocols",
                                               &::test::run_value_conversion_tests> {};
template<>
struct jh::test::tiny_test::session<"test module test_value_conversion">
        : jh::test::tiny_test::session_definition<
            "test module test_value_conversion",
            jh::test::tiny_test::test<"value conversion protocols">> {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_value_conversion"> registration{};
}
