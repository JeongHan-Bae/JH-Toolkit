#include "duplicate_test_shared.hpp"

template<>
struct jh::test::tiny_test::session<"second session">
    : jh::test::tiny_test::session_definition<
          "second session",
          jh::test::tiny_test::test<"shared test">
      > {};

namespace test {
    [[maybe_unused]] const tiny_test::session<"second session"> registered_session{};
}
