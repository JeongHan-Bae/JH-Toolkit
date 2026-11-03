#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"

#include "jh/bloom_filter"
#include "jh/pod"

#include <atomic>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <limits>
#include <stdexcept>
#include <span>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

namespace test {

    struct duplicate_hashes {
        auto operator()(const int &value) const {
            const auto hash = static_cast<std::size_t>(value);
            return std::tuple{hash, hash + 128, hash + 256, hash + 64};
        }
    };

    struct pod_hashes {
        auto operator()(const int &value) const {
            const auto hash = static_cast<std::size_t>(value);
            return jh::pod::make_tuple(hash + 3, hash + 67);
        }
    };

    struct single_hash {
        auto operator()(const int &value) const {
            return std::tuple{static_cast<std::size_t>(value)};
        }
    };

    struct empty_hash_group {
        auto operator()(const int &) const {
            return std::tuple{};
        }
    };

    struct throwing_hash {
        static inline bool should_throw = false;

        auto operator()(const int &value) const {
            if (should_throw) throw std::runtime_error("hash failed");
            return std::tuple{static_cast<std::size_t>(value + 9)};
        }
    };

    struct throwing_number {
        static inline bool should_throw = false;
        std::size_t value;

        operator std::size_t() const {
            if (should_throw) throw std::runtime_error("conversion failed");
            return value;
        }
    };

    struct converting_hash {
        auto operator()(const int &value) const {
            return std::tuple{throwing_number{static_cast<std::size_t>(value + 1)}};
        }
    };

    struct allocation_counts {
        static inline int allocations = 0;
        static inline int deallocations = 0;
        static inline int constructions = 0;
        static inline int destructions = 0;
        static inline int throw_on_construction = 0;
        static inline std::size_t max_words = std::numeric_limits<std::size_t>::max();
    };

    template<typename T>
    struct counting_allocator {
        using value_type = T;

        counting_allocator() = default;
        template<typename U>
        counting_allocator(const counting_allocator<U> &) noexcept {}

        T *allocate(std::size_t count) {
            ++allocation_counts::allocations;
            return std::allocator<T>{}.allocate(count);
        }

        void deallocate(T *pointer, std::size_t count) noexcept {
            ++allocation_counts::deallocations;
            std::allocator<T>{}.deallocate(pointer, count);
        }

        [[nodiscard]] std::size_t max_size() const noexcept {
            return allocation_counts::max_words;
        }

        template<typename U, typename... Args>
        void construct(U *pointer, Args &&...args) {
            ++allocation_counts::constructions;
            if (allocation_counts::constructions ==
                allocation_counts::throw_on_construction) {
                throw std::runtime_error("allocation construction failed");
            }
            std::construct_at(pointer, std::forward<Args>(args)...);
        }

        template<typename U>
        void destroy(U *pointer) noexcept {
            ++allocation_counts::destructions;
            std::destroy_at(pointer);
        }
    };

    template<typename T, typename U>
    bool operator==(const counting_allocator<T> &, const counting_allocator<U> &) noexcept {
        return true;
    }

    struct counted_group { std::size_t first; std::size_t second; };

    inline int first_reads = 0;
    inline int second_reads = 0;
    inline int first_hasher_calls = 0;
    inline int second_hasher_calls = 0;

    template<std::size_t I, typename Group>
    requires std::is_same_v<std::remove_cvref_t<Group>, counted_group>
    std::size_t get(Group &&group) noexcept {
        if constexpr (I == 0) {
            ++first_reads;
            return group.first;
        } else {
            ++second_reads;
            return group.second;
        }
    }
}

namespace std {
    template<>
    struct tuple_size<test::counted_group> : integral_constant<size_t, 2> {};

    template<size_t I>
    struct tuple_element<I, test::counted_group> { using type = size_t; };
}

namespace test {

    struct counted_hasher {
        counted_group operator()(const int &value) const noexcept {
            ++first_hasher_calls;
            const auto first = static_cast<std::size_t>(value) % 128;
            return {first, (first + 1) % 128};
        }
    };

    struct later_hasher {
        auto operator()(const int &value) const noexcept {
            ++second_hasher_calls;
            return std::tuple{static_cast<std::size_t>(value / 128 + 10)};
        }
    };

    inline std::atomic<int> concurrent_calls{0};
    inline std::atomic<bool> query_entered{false};
    inline std::atomic<bool> release_query{false};
    inline std::atomic<bool> writer_hashed{false};

    struct coordinated_hash {
        auto operator()(const int &value) const {
            const int call = concurrent_calls.fetch_add(1);
            if (call == 0) {
                query_entered.store(true);
                while (!release_query.load()) std::this_thread::yield();
            } else if (call == 1) {
                writer_hashed.store(true);
            }
            return std::tuple{static_cast<std::size_t>(value),
                              static_cast<std::size_t>(value + 64)};
        }
    };

    void test_mapping_and_storage() {
        using inline_filter = jh::bloom_filter<int, 128, jh::typed::monostate,
                duplicate_hashes{}, pod_hashes{}>;
        static_assert(!std::is_copy_constructible_v<inline_filter>);
        static_assert(!std::is_move_constructible_v<inline_filter>);

        inline_filter filter;
        jh::test::tiny_test::expect(!filter.possibly_contains(5), "empty filter rejects value");
        filter.insert(5);
        jh::test::tiny_test::expect(filter.possibly_contains(5), "inserted value is possible");
        jh::test::tiny_test::expect(!filter.possibly_contains(6), "different bit rejects value");
        jh::test::tiny_test::expect(filter.possibly_contains(133), "modulo collision is possible");
        filter.clear();
        jh::test::tiny_test::expect(!filter.possibly_contains(5), "clear removes all bits");

        using one_bit_filter = jh::bloom_filter<int, 1, jh::typed::monostate,
                single_hash{}>;
        one_bit_filter one_bit;
        jh::test::tiny_test::expect(!one_bit.possibly_contains(7),
                                   "one-bit filter starts empty");
        one_bit.insert(7);
        jh::test::tiny_test::expect(one_bit.possibly_contains(8),
                                   "one-bit capacity maps every hash to one bit");
        one_bit.clear();
        jh::test::tiny_test::expect(!one_bit.possibly_contains(8),
                                   "one-bit filter clears its bit");

        using empty_group_filter = jh::bloom_filter<int, 65,
                jh::typed::monostate, empty_hash_group{}, single_hash{}>;
        static_assert(empty_group_filter::hash_count == 1);
        empty_group_filter with_empty_group;
        with_empty_group.insert(7);
        jh::test::tiny_test::expect(with_empty_group.possibly_contains(7) &&
                                   !with_empty_group.possibly_contains(8),
                                   "an empty hash group contributes no bits or checks");

        allocation_counts::allocations = 0;
        allocation_counts::deallocations = 0;
        allocation_counts::constructions = 0;
        allocation_counts::destructions = 0;
        using allocated_filter = jh::bloom_filter<int, 128, counting_allocator<int>,
                duplicate_hashes{}, pod_hashes{}>;
        static_assert(!std::is_copy_constructible_v<allocated_filter>);
        static_assert(!std::is_move_constructible_v<allocated_filter>);
        {
            allocated_filter allocated{counting_allocator<int>{}};
            jh::test::tiny_test::expect(!allocated.possibly_contains(5),
                                       "allocated storage starts empty");
            allocated.insert(5);
            jh::test::tiny_test::expect(allocated.possibly_contains(5), "allocated storage works");
            jh::test::tiny_test::expect(allocation_counts::allocations == 1,
                                       "word sequence is allocated once");
            jh::test::tiny_test::expect(allocation_counts::constructions == 2,
                                       "both words are constructed");
        }
        jh::test::tiny_test::expect(allocation_counts::deallocations == 1,
                                   "word sequence is deallocated once");
        jh::test::tiny_test::expect(allocation_counts::destructions == 2,
                                   "both words are destroyed");

        allocation_counts::throw_on_construction = 4;
        jh::test::tiny_test::expect_throw<std::runtime_error>([&] {
            allocated_filter failed{counting_allocator<int>{}};
        }, "word construction failure is reported");
        allocation_counts::throw_on_construction = 0;
        jh::test::tiny_test::expect(
            allocation_counts::allocations == 2 && allocation_counts::deallocations == 2 &&
            allocation_counts::constructions == 4 && allocation_counts::destructions == 3,
            "failed word construction destroys its prefix and releases storage"
        );

        allocation_counts::max_words = 1;
        jh::test::tiny_test::expect_throw<std::length_error>([&] {
            allocated_filter refused{counting_allocator<int>{}};
        }, "allocator max_size rejects the word count");
        allocation_counts::max_words = std::numeric_limits<std::size_t>::max();
        jh::test::tiny_test::expect(allocation_counts::allocations == 2,
                                   "allocator max_size rejects before allocation");

        using large_filter = jh::bloom_filter<int, 8388608,
                std::allocator<std::uint64_t>, single_hash{}>;
        static_assert(sizeof(large_filter) < 1024);
        large_filter large;
        large.insert(8388607);
        jh::test::tiny_test::expect(large.possibly_contains(8388607) &&
                                   !large.possibly_contains(8388606),
                                   "allocated storage supports the final bit beyond 16 KiB");
        large.clear();
        jh::test::tiny_test::expect(!large.possibly_contains(8388607),
                                   "large allocated storage clears completely");
    }

    void test_short_circuit_and_exception_safety() {
        using filter_type = jh::bloom_filter<int, 128, jh::typed::monostate,
                counted_hasher{}, later_hasher{}>;
        filter_type filter;
        first_hasher_calls = second_hasher_calls = 0;
        filter.insert(0);
        jh::test::tiny_test::expect(first_hasher_calls == 1 && second_hasher_calls == 1,
                                   "insert calls each hasher once");

        first_reads = second_reads = first_hasher_calls = second_hasher_calls = 0;
        jh::test::tiny_test::expect(!filter.possibly_contains(3), "missing first bit rejects");
        jh::test::tiny_test::expect(first_reads == 1 && second_reads == 0 &&
                                   first_hasher_calls == 1 && second_hasher_calls == 0,
                                   "group stops at first missing bit");

        first_reads = second_reads = first_hasher_calls = second_hasher_calls = 0;
        jh::test::tiny_test::expect(!filter.possibly_contains(1), "missing second bit rejects");
        jh::test::tiny_test::expect(first_reads == 1 && second_reads == 1 &&
                                   first_hasher_calls == 1 && second_hasher_calls == 0,
                                   "next hasher is skipped");

        first_reads = second_reads = first_hasher_calls = second_hasher_calls = 0;
        jh::test::tiny_test::expect(!filter.possibly_contains(128), "later hasher rejects");
        jh::test::tiny_test::expect(first_reads == 1 && second_reads == 1 &&
                                   first_hasher_calls == 1 && second_hasher_calls == 1,
                                   "later hasher runs after group passes");

        using throwing_filter = jh::bloom_filter<int, 64, jh::typed::monostate,
                single_hash{}, throwing_hash{}>;
        throwing_filter throws;
        throwing_hash::should_throw = true;
        jh::test::tiny_test::expect_throw<std::runtime_error>([&] {
            throws.insert(2);
        }, "hasher failure is reported");
        throwing_hash::should_throw = false;
        jh::test::tiny_test::expect(!throws.possibly_contains(2),
                                   "hasher failure leaves storage unchanged");

        throws.insert(2);
        throwing_hash::should_throw = true;
        jh::test::tiny_test::expect_throw<std::runtime_error>([&] {
            (void)throws.possibly_contains(2);
        }, "query hasher failure is reported");
        throwing_hash::should_throw = false;
        throws.clear();
        jh::test::tiny_test::expect(!throws.possibly_contains(2),
                                   "query failure releases the shared lock");

        using converting_filter = jh::bloom_filter<int, 64, jh::typed::monostate,
                single_hash{}, converting_hash{}>;
        converting_filter converting;
        throwing_number::should_throw = true;
        jh::test::tiny_test::expect_throw<std::runtime_error>([&] {
            converting.insert(2);
        }, "hash conversion failure is reported");
        throwing_number::should_throw = false;
        jh::test::tiny_test::expect(!converting.possibly_contains(2),
                                   "conversion failure leaves storage unchanged");
    }

    void test_concurrent_snapshot() {
        using filter_type = jh::bloom_filter<int, 128, jh::typed::monostate,
                coordinated_hash{}>;
        filter_type filter;
        concurrent_calls.store(0);
        query_entered.store(false);
        release_query.store(false);
        writer_hashed.store(false);
        bool before_insert = true;

        std::thread reader([&] { before_insert = filter.possibly_contains(7); });
        while (!query_entered.load()) std::this_thread::yield();
        std::thread writer([&] { filter.insert(7); });
        while (!writer_hashed.load()) std::this_thread::yield();
        release_query.store(true);
        reader.join();
        writer.join();

        jh::test::tiny_test::expect(!before_insert && filter.possibly_contains(7),
                                   "query observes complete state before or after insert");
    }

    void test_public_word_persistence_interface() {
        using inline_filter = jh::bloom_filter<int, 65,
                jh::typed::monostate, single_hash{}>;
        using allocated_filter = jh::bloom_filter<int, 65,
                std::allocator<std::uint64_t>, single_hash{}>;

        inline_filter source;
        source.insert(64);
        std::array<std::uint64_t, inline_filter::word_count> words{};
        std::size_t visited = 0;
        source.visit_words([&](std::size_t index, std::uint64_t word) {
            words[index] = word;
            ++visited;
        });
        jh::test::tiny_test::expect(visited == 2 && words[0] == 0 && words[1] == 1,
                                   "public word visitor emits logical values in order");

        allocated_filter restored;
        restored.replace_words(words);
        jh::test::tiny_test::expect(restored.possibly_contains(64) &&
                                   !restored.possibly_contains(63),
                                   "allocated filter restores words from a custom format");

        restored.insert(63);
        restored.replace_words(words);
        jh::test::tiny_test::expect(!restored.possibly_contains(63) &&
                                   restored.possibly_contains(64),
                                   "whole-state restoration replaces an earlier insertion");

        jh::test::tiny_test::expect_throw<std::length_error>([&] {
            restored.replace_words(std::span<const std::uint64_t>{words.data(), 1});
        }, "custom restoration rejects a short word sequence");

        words[1] = 3;
        jh::test::tiny_test::expect_throw<std::invalid_argument>([&] {
            restored.replace_words(words);
        }, "custom restoration rejects unused high bits");
        jh::test::tiny_test::expect(restored.possibly_contains(64) &&
                                   !restored.possibly_contains(63),
                                   "invalid custom restoration leaves words unchanged");

        jh::test::tiny_test::expect_throw<std::runtime_error>([&] {
            restored.visit_words([](std::size_t, std::uint64_t) {
                throw std::runtime_error("visitor failed");
            });
        }, "visitor exception is reported");
        restored.clear();
        jh::test::tiny_test::expect(!restored.possibly_contains(64),
                                   "visitor exception releases the shared lock");
    }
}

namespace test {
    void run_bloom_filter_tests() {
        test_mapping_and_storage();
        test_short_circuit_and_exception_safety();
        test_concurrent_snapshot();
        test_public_word_persistence_interface();
    }
}

template<>
struct jh::test::tiny_test::test<"bloom_filter maps and checks hashes">
        : jh::test::tiny_test::test_definition<"bloom_filter maps and checks hashes",
                                               &::test::run_bloom_filter_tests> {};
template<>
struct jh::test::tiny_test::session<"test module test_bloom_filter">
        : jh::test::tiny_test::session_definition<
            "test module test_bloom_filter",
            jh::test::tiny_test::test<"bloom_filter maps and checks hashes">> {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_bloom_filter"> registration{};
}
