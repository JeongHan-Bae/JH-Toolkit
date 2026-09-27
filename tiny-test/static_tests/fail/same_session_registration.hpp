#pragma once

#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

extern int tiny_test_ambiguous_runs;

namespace test {
    inline void shared_body() {
        tiny_test::expect(true);
        ++tiny_test_ambiguous_runs;
    }
}

template<>
struct jh::test::tiny_test::test<"shared test">
    : jh::test::tiny_test::test_definition<"shared test", &::test::shared_body> {};

template<>
struct jh::test::tiny_test::session<"shared session">
    : jh::test::tiny_test::session_definition<
          "shared session",
          jh::test::tiny_test::test<"shared test">
      > {};

namespace test {
    [[maybe_unused]] const tiny_test::session<"shared session"> registered_session{};
}
