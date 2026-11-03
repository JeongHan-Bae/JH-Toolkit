#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"

#include "jh/bloom_filter"
#include "jh/pod"

#include <array>
#include <cstddef>
#include <cstdint>
#include <istream>
#include <latch>
#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

namespace test {
    struct first_hash {
        static inline int calls = 0;

        auto operator()(const std::string &value) const {
            ++calls;
            const auto first = static_cast<std::size_t>(value.empty() ? 0 :
                static_cast<unsigned char>(value.front()));
            return std::tuple{value.size(), first + 1};
        }
    };

    struct second_hash {
        static inline int calls = 0;

        auto operator()(const std::string &value) const {
            ++calls;
            const auto first = static_cast<std::size_t>(value.empty() ? 0 :
                static_cast<unsigned char>(value.front()));
            return jh::pod::make_tuple(first, value.size() + 200);
        }
    };

    using inline_filter = jh::bloom_filter<std::string, 65, jh::typed::monostate,
            first_hash{}, second_hash{}>;
    using allocated_filter = jh::bloom_filter<std::string, 65, std::allocator<int>,
            first_hash{}, second_hash{}>;
    using persistence = jh::bloom_persistence<"hello", inline_filter>;
    static_assert(persistence::serialized_size == 80);

    std::uint64_t word_at(const std::string &data, std::size_t offset) {
        std::uint64_t result = 0;
        for (std::size_t i = 0; i < 8; ++i) {
            result |= static_cast<std::uint64_t>(
                static_cast<unsigned char>(data[offset + i])) << (i * 8);
        }
        return result;
    }

    template<typename Filter>
    std::string save(const Filter &filter) {
        std::ostringstream out(std::ios::binary);
        jh::serialize_bloom<"hello">(out, filter);
        return out.str();
    }

    void test_layout_and_round_trip() {
        inline_filter source;
        source.insert("x");
        source.insert(std::string(64, 'z'));
        first_hash::calls = second_hash::calls = 0;
        const std::string bytes = save(source);
        jh::test::tiny_test::expect(first_hash::calls == 1 && second_hash::calls == 1,
                                   "each sentinel hasher runs once during serialization");
        jh::test::tiny_test::expect(bytes.size() == persistence::serialized_size,
                                   "serialized size is fixed");
        jh::test::tiny_test::expect(word_at(bytes, 0) == 5 &&
                                   bytes.substr(8, 8) == std::string("hello\0\0\0", 8),
                                   "signature length and zero padding are fixed");
        jh::test::tiny_test::expect(word_at(bytes, 16) == 4 &&
                                   word_at(bytes, 24) == 5 && word_at(bytes, 32) == 105 &&
                                   word_at(bytes, 40) == 104 && word_at(bytes, 48) == 205 &&
                                   word_at(bytes, 56) == 65 && word_at(bytes, 72) == 1,
                                   "raw sentinel hashes and capacity use little-endian words");

        std::ostringstream failed_output(std::ios::binary);
        failed_output.setstate(std::ios::badbit);
        jh::test::tiny_test::expect_throw<std::runtime_error>([&] {
            jh::serialize_bloom<"hello">(failed_output, source);
        }, "failed binary output reports a write error");

        allocated_filter allocated;
        allocated.insert("x");
        allocated.insert(std::string(64, 'z'));
        jh::test::tiny_test::expect(save(allocated) == bytes,
                                   "allocator mode has identical binary layout");

        inline_filter restored;
        std::istringstream in(bytes, std::ios::binary);
        first_hash::calls = second_hash::calls = 0;
        jh::deserialize_bloom<"hello">(in, restored);
        jh::test::tiny_test::expect(first_hash::calls == 1 && second_hash::calls == 1,
                                   "each sentinel hasher runs once during deserialization");
        jh::test::tiny_test::expect(restored.possibly_contains("x") && save(restored) == bytes,
                                   "round trip restores the logical words");

        std::ostringstream sequence(std::ios::binary);
        jh::serialize_bloom<"hello">(sequence, source);
        jh::serialize_bloom<"hello">(sequence, allocated);
        sequence << "tail";
        std::istringstream chained(sequence.str(), std::ios::binary);
        inline_filter first;
        allocated_filter second;
        jh::deserialize_bloom<"hello">(chained, first);
        jh::deserialize_bloom<"hello">(chained, second);
        std::string tail;
        chained >> tail;
        jh::test::tiny_test::expect(first.possibly_contains("x") &&
                                   second.possibly_contains("x") && tail == "tail",
                                   "two records leave subsequent stream data intact");
    }

    void test_validation_is_transactional() {
        inline_filter source;
        source.insert("x");
        const std::string record = save(source);
        inline_filter destination;
        destination.insert("different marker");
        const std::string original = save(destination);

        const auto expect_rejected_without_change = [&](const std::string &bytes) {
            std::istringstream in(bytes, std::ios::binary);
            jh::test::tiny_test::expect_throw<std::runtime_error>([&] {
                jh::deserialize_bloom<"hello">(in, destination);
            }, "invalid Bloom record is rejected");
            jh::test::tiny_test::expect(save(destination) == original,
                                       "rejected record leaves destination unchanged");
        };

        for (const std::size_t offset : {0u, 8u, 13u, 16u, 24u, 32u,
                                         40u, 48u, 56u}) {
            std::string bad = record;
            bad[offset] ^= 1;
            expect_rejected_without_change(bad);
        }

        std::string unused_bit = record;
        unused_bit[79] = static_cast<char>(0x80);
        expect_rejected_without_change(unused_bit);

        for (const std::size_t length : {0u, 7u, 8u, 12u, 16u, 23u, 32u,
                                         56u, 64u, 79u}) {
            expect_rejected_without_change(record.substr(0, length));
        }
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

    struct immovable_dual_hash {
        auto operator()(const immovable_dual_protocol &value) const {
            return std::tuple{static_cast<std::size_t>(value.source)};
        }
    };

    struct integer_hash {
        auto operator()(const int &value) const {
            return std::tuple{static_cast<std::size_t>(value)};
        }
    };

    struct wrapped_uint64 {
        std::uint64_t value = 0;
    };

    std::ostream &operator<<(std::ostream &out, const wrapped_uint64 &value) {
        return out << value.value;
    }

    std::istream &operator>>(std::istream &in, wrapped_uint64 &value) {
        return in >> value.value;
    }

    struct wrapped_hash {
        auto operator()(const wrapped_uint64 &value) const {
            return std::tuple{static_cast<std::size_t>(value.value)};
        }
    };

    void test_adl_stream_sentinel_failure() {
        static_assert(jh::concepts::serializable<wrapped_uint64>);
        using filter_type = jh::bloom_filter<wrapped_uint64, 65,
                jh::typed::monostate, wrapped_hash{}>;

        filter_type source;
        source.insert(wrapped_uint64{7});
        std::ostringstream invalid_output(std::ios::binary);
        jh::test::tiny_test::expect_throw<std::runtime_error>([&] {
            jh::serialize_bloom<"abc">(invalid_output, source);
        }, "nonnumeric ADL stream sentinel rejects serialization");
        jh::test::tiny_test::expect(invalid_output.str().empty(),
                                   "invalid sentinel fails before writing");

        std::ostringstream valid_output(std::ios::binary);
        jh::serialize_bloom<"123">(valid_output, source);
        std::string record = valid_output.str();
        record.replace(8, 3, "abc");

        filter_type target;
        target.insert(wrapped_uint64{11});
        std::istringstream invalid_input(record, std::ios::binary);
        jh::test::tiny_test::expect_throw<std::runtime_error>([&] {
            jh::deserialize_bloom<"abc">(invalid_input, target);
        }, "nonnumeric ADL stream sentinel rejects deserialization");
        jh::test::tiny_test::expect(target.possibly_contains(wrapped_uint64{11}) &&
                                   !target.possibly_contains(wrapped_uint64{7}),
                                   "invalid sentinel leaves target unchanged");
    }

    struct large_hash {
        auto operator()(const std::string &value) const {
            return std::tuple{value == "edge" ? std::size_t{8388607} : std::size_t{3}};
        }
    };

    void test_large_allocated_record() {
        using large_filter = jh::bloom_filter<std::string, 8388608,
                std::allocator<std::uint64_t>, large_hash{}>;
        using large_persistence = jh::bloom_persistence<"hello", large_filter>;
        static_assert(large_persistence::word_count == 131072);
        static_assert(large_persistence::serialized_size == 1048616);

        large_filter source;
        source.insert("edge");
        std::ostringstream out(std::ios::binary);
        jh::serialize_bloom<"hello">(out, source);
        const std::string record = out.str();
        jh::test::tiny_test::expect(
            record.size() == large_persistence::serialized_size &&
            word_at(record, record.size() - 8) == (std::uint64_t{1} << 63),
            "large record stores the final word without an inline array"
        );

        large_filter target;
        target.insert("base");
        jh::test::tiny_test::expect_throw<std::runtime_error>([&] {
            std::istringstream short_input(record.substr(0, record.size() - 1),
                                           std::ios::binary);
            jh::deserialize_bloom<"hello">(short_input, target);
        }, "large truncated record is rejected");
        jh::test::tiny_test::expect(
            target.possibly_contains("base") && !target.possibly_contains("edge"),
            "large truncated record leaves target unchanged"
        );

        std::istringstream in(record, std::ios::binary);
        jh::deserialize_bloom<"hello">(in, target);
        jh::test::tiny_test::expect(
            target.possibly_contains("edge") && !target.possibly_contains("base"),
            "large allocated record restores all words"
        );
    }

    void test_sentinel_protocols_and_embedded_zero() {
        using dual_filter = jh::bloom_filter<immovable_dual_protocol, 65,
                jh::typed::monostate, immovable_dual_hash{}>;
        dual_filter dual_source;
        immovable_dual_protocol dual_key;
        dual_source.insert(dual_key);
        std::ostringstream dual_out(std::ios::binary);
        jh::serialize_bloom<"sentinel">(dual_out, dual_source);
        jh::test::tiny_test::expect(word_at(dual_out.str(), 24) == 1,
                                   "Bloom sentinel selects String over Stream protocol");
        using integer_filter = jh::bloom_filter<int, 65, jh::typed::monostate,
                integer_hash{}>;
        integer_filter integer_source;
        integer_source.insert(7);
        std::ostringstream integer_out(std::ios::binary);
        jh::serialize_bloom<"42">(integer_out, integer_source);
        integer_filter integer_target;
        std::istringstream integer_in(integer_out.str(), std::ios::binary);
        jh::deserialize_bloom<"42">(integer_in, integer_target);
        jh::test::tiny_test::expect(integer_target.possibly_contains(7),
                                   "stream protocol constructs a valid sentinel");

        jh::test::tiny_test::expect_throw<std::runtime_error>([&] {
            std::ostringstream invalid_out(std::ios::binary);
            jh::serialize_bloom<"not an integer">(invalid_out, integer_source);
        }, "invalid integer sentinel is rejected");

        using binary_signature = jh::bloom_persistence<"a\0b", inline_filter>;
        static_assert(binary_signature::signature_bytes == 3);
        inline_filter source;
        source.insert("x");
        std::ostringstream out(std::ios::binary);
        binary_signature::serialize(out, source);
        const std::string bytes = out.str();
        jh::test::tiny_test::expect(bytes.size() == binary_signature::serialized_size &&
                                   bytes.substr(8, 8) == std::string("a\0b\0\0\0\0\0", 8),
                                   "embedded zero is a signature byte, not a terminator");
        inline_filter restored;
        std::istringstream in(bytes, std::ios::binary);
        binary_signature::deserialize(in, restored);
        jh::test::tiny_test::expect(restored.possibly_contains("x"),
                                   "embedded-zero signature round trips");
    }

    struct view_hash {
        auto operator()(const std::string_view &value) const {
            const auto first = value.empty() ? std::size_t{0} :
                static_cast<std::size_t>(static_cast<unsigned char>(value.front()));
            return std::tuple{value.size(), first};
        }
    };

    void test_borrowed_sentinel_text_lifetime() {
        using view_filter = jh::bloom_filter<std::string_view, 65,
                jh::typed::monostate, view_hash{}>;
        using view_persistence = jh::bloom_persistence<
                "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",
                view_filter>;
        static_assert(view_persistence::signature_bytes == 62);

        view_filter source;
        source.insert("edge");
        std::ostringstream out(std::ios::binary);
        view_persistence::serialize(out, source);
        const std::string record = out.str();
        const std::size_t first_hash_offset = 8 + view_persistence::signature_words * 8 + 8;
        jh::test::tiny_test::expect(
            word_at(record, first_hash_offset) == 62 &&
            word_at(record, first_hash_offset + 8) == static_cast<std::uint64_t>('a'),
            "borrowed sentinel view remains valid during serialization"
        );

        view_filter restored;
        std::istringstream in(record, std::ios::binary);
        view_persistence::deserialize(in, restored);
        jh::test::tiny_test::expect(restored.possibly_contains("edge"),
                                   "borrowed sentinel view remains valid during deserialization");
    }

    struct immovable_key {
        std::size_t value = 0;
        immovable_key() = default;
        immovable_key(const immovable_key &) = delete;
        immovable_key(immovable_key &&) = delete;
    };

    std::ostream &operator<<(std::ostream &out, const immovable_key &key) {
        return out << key.value;
    }

    std::istream &operator>>(std::istream &in, immovable_key &key) {
        return in >> key.value;
    }

    struct immovable_hash {
        auto operator()(const immovable_key &key) const {
            return std::tuple{key.value};
        }
    };

    void test_immovable_stream_sentinel() {
        static_assert(jh::concepts::serializable<immovable_key>);
        using filter_type = jh::bloom_filter<immovable_key, 65,
                jh::typed::monostate, immovable_hash{}>;

        immovable_key key;
        key.value = 7;
        filter_type source;
        source.insert(key);
        std::ostringstream out(std::ios::binary);
        jh::serialize_bloom<"17">(out, source);

        filter_type restored;
        std::istringstream in(out.str(), std::ios::binary);
        jh::deserialize_bloom<"17">(in, restored);
        jh::test::tiny_test::expect(restored.possibly_contains(key),
                                   "immovable Stream-protocol sentinel round trips");
    }

    struct split_hash {
        auto operator()(const int &value) const noexcept {
            if (value == 1) return std::tuple{std::size_t{0}, std::size_t{64}};
            if (value == 2) return std::tuple{std::size_t{0}, std::size_t{0}};
            return std::tuple{std::size_t{64}, std::size_t{64}};
        }
    };

    template<typename Allocator>
    void check_controlled_persistence_interleaving() {
        using filter_type = jh::bloom_filter<int, 65, Allocator, split_hash{}>;
        filter_type shared;
        shared.insert(2);

        filter_type replacement;
        replacement.insert(3);
        std::ostringstream out(std::ios::binary);
        jh::serialize_bloom<"42">(out, replacement);
        const std::string replacement_record = out.str();

        std::latch request_restore{1};
        std::latch restore_requested{1};
        std::thread restorer([&] {
            request_restore.wait();
            restore_requested.count_down();
            std::istringstream in(replacement_record, std::ios::binary);
            jh::deserialize_bloom<"42">(in, shared);
        });

        std::array<std::uint64_t, filter_type::word_count> custom_record{};
        bool query_saw_first = false;
        // The custom save holds its shared lock across both word callbacks.
        shared.visit_words([&](std::size_t index, std::uint64_t word) {
            custom_record[index] = word;
            if (index == 0) {
                std::thread accessor([&] {
                    query_saw_first = shared.possibly_contains(2);
                });
                accessor.join();
                request_restore.count_down();
                restore_requested.wait();
            }
        });
        restorer.join();

        jh::test::tiny_test::expect(custom_record == std::array<std::uint64_t, 2>{1, 0} &&
                                   query_saw_first &&
                                   shared.possibly_contains(3) &&
                                   !shared.possibly_contains(2),
                                   "custom save sees old words and built-in restore commits afterward");
    }

    void test_controlled_persistence_interleaving() {
        check_controlled_persistence_interleaving<jh::typed::monostate>();
        check_controlled_persistence_interleaving<std::allocator<std::uint64_t>>();
    }

    void run_bloom_persistence_tests() {
        test_layout_and_round_trip();
        test_validation_is_transactional();
        test_sentinel_protocols_and_embedded_zero();
        test_adl_stream_sentinel_failure();
        test_large_allocated_record();
        test_borrowed_sentinel_text_lifetime();
        test_immovable_stream_sentinel();
        test_controlled_persistence_interleaving();
    }
}

template<>
struct jh::test::tiny_test::test<"bloom persistence fixed layout">
    : jh::test::tiny_test::test_definition<"bloom persistence fixed layout",
                                           &::test::run_bloom_persistence_tests> {};
template<>
struct jh::test::tiny_test::session<"test module test_bloom_persistence">
    : jh::test::tiny_test::session_definition<
          "test module test_bloom_persistence",
          jh::test::tiny_test::test<"bloom persistence fixed layout">> {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_bloom_persistence"> registration{};
}
