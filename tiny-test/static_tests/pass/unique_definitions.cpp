#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

namespace test {
    void unique_one() { tiny_test::expect(true); }
    void unique_two() { tiny_test::expect_not(false); }
}

template<>
struct jh::test::tiny_test::test<"unique test one">
    : jh::test::tiny_test::test_definition<"unique test one", &::test::unique_one> {};

template<>
struct jh::test::tiny_test::test<"unique test two">
    : jh::test::tiny_test::test_definition<"unique test two", &::test::unique_two> {};

template<>
struct jh::test::tiny_test::session<"unique session">
    : jh::test::tiny_test::session_definition<
          "unique session",
          jh::test::tiny_test::test<"unique test one">,
          jh::test::tiny_test::test<"unique test two">
      > {};

using valid_suite = jh::test::tiny_test::suite<
    jh::test::tiny_test::session<"unique session">
>;

static_assert(valid_suite::session_count == 1);
