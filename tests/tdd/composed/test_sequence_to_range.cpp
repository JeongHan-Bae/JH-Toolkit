#define TINY_TEST_MAIN

#include <ranges>

#include "jh/conceptual/sequence.h"
#include "jh/pod"
#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

namespace test {
    void sequence_converts_to_range() {
        constexpr jh::pod::array<int, 3> sequence{1, 2, 3};
        auto range = jh::to_range(sequence);
        auto current = range.begin();

        std::ranges::for_each(range, [&](const int value) {
            tiny_test::expect(value == *current, "range iteration preserves sequence order");
            ++current;
        });

        tiny_test::expect(current == range.end(), "range iteration visits every sequence element");
    }
}

template<>
struct jh::test::tiny_test::test<"sequence converts to range">
    : jh::test::tiny_test::test_definition<"sequence converts to range", &::test::sequence_converts_to_range> {};

template<>
struct jh::test::tiny_test::session<"sequence range composition session">
    : jh::test::tiny_test::session_definition<
          "sequence range composition session",
          jh::test::tiny_test::test<"sequence converts to range">
      > {};

namespace test {
    [[maybe_unused]] const tiny_test::session<"sequence range composition session"> registration{};
}
