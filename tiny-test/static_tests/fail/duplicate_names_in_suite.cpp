#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

namespace test {
    void shared_suite_body() { tiny_test::expect(true); }
}

template<>
struct jh::test::tiny_test::test<"suite shared test">
    : jh::test::tiny_test::test_definition<"suite shared test", &::test::shared_suite_body> {};

template<>
struct jh::test::tiny_test::session<"first session">
    : jh::test::tiny_test::session_definition<
          "first session",
          jh::test::tiny_test::test<"suite shared test">
      > {};

template<>
struct jh::test::tiny_test::session<"second session">
    : jh::test::tiny_test::session_definition<
          "second session",
          jh::test::tiny_test::test<"suite shared test">
      > {};

using invalid_suite = jh::test::tiny_test::suite<
    jh::test::tiny_test::session<"first session">,
    jh::test::tiny_test::session<"second session">
>;

static_assert(sizeof(invalid_suite) != 0);
