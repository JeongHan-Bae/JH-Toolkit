#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

namespace test {
    void duplicate_member_body() { tiny_test::expect(true); }
}

template<>
struct jh::test::tiny_test::test<"duplicate member">
    : jh::test::tiny_test::test_definition<"duplicate member", &::test::duplicate_member_body> {};

template<>
struct jh::test::tiny_test::session<"duplicate member session">
    : jh::test::tiny_test::session_definition<
          "duplicate member session",
          jh::test::tiny_test::test<"duplicate member">,
          jh::test::tiny_test::test<"duplicate member">
      > {};
