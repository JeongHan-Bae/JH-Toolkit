#include "duplicate_test_shared.hpp"

template<>
struct jh::test::tiny_test::session<"first session">
    : jh::test::tiny_test::session_definition<
          "first session",
          jh::test::tiny_test::test<"shared test">
      > {};

namespace test {
    [[maybe_unused]] const tiny_test::session<"first session"> registered_session{};
}
