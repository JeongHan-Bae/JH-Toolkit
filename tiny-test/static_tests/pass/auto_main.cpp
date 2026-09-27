#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

namespace test {
    void generated_main_body() { tiny_test::expect(true); }
}

template<>
struct jh::test::tiny_test::test<"shared entry test">
    : jh::test::tiny_test::test_definition<"shared entry test", &::test::generated_main_body> {};

template<>
struct jh::test::tiny_test::session<"shared entry session">
    : jh::test::tiny_test::session_definition<
          "shared entry session",
          jh::test::tiny_test::test<"shared entry test">
      > {};

namespace test {
    [[maybe_unused]] const tiny_test::session<"shared entry session"> registered_session{};
}
