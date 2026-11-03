#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

#include <stdexcept>

#include "jh/synchronous/ipc/process_launcher.h"

namespace {
    using exit_one_launcher = jh::sync::ipc::process_launcher<"tdd_unit_process_launcher_exit_one">;
    using missing_launcher = jh::sync::ipc::process_launcher<"tdd_ipc_missing_exec_fixture">;
}

namespace test {
    void process_launcher_exec_failure_contracts() {
        jh::test::tiny_test::expect_throw<std::runtime_error>([&] {
            auto missing_executable = missing_launcher::start();
            (void) missing_executable.wait();
        }, "start throws when exec cannot find the requested executable");

        auto exit_one = exit_one_launcher::start();
        const auto result = exit_one.wait();
        jh::test::tiny_test::expect(
            result.has_value() && result.value() == 1,
            "a successfully executed program may exit with code one"
        );
    }

}

template<>
struct jh::test::tiny_test::test<"process launcher reports exec failure">
    : jh::test::tiny_test::test_definition<
          "process launcher reports exec failure",
          &::test::process_launcher_exec_failure_contracts
      > {};

template<>
struct jh::test::tiny_test::session<"process_launcher">
    : jh::test::tiny_test::session_definition<
          "process_launcher",
          jh::test::tiny_test::test<"process launcher reports exec failure">
      > {};

namespace test {
    [[maybe_unused]] const tiny_test::session<"process_launcher"> process_launcher_session{};
}
