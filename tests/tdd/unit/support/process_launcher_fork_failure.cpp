#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

#include <cerrno>
#include <system_error>
#include <sys/types.h>

#include "jh/synchronous/ipc/process_launcher.h"

namespace test {
    void process_launcher_fork_failure_contracts() {
        int observed_error{};
        try {
            static_cast<void>(jh::sync::ipc::detail::fork_or_throw([] {
                errno = EAGAIN;
                return static_cast<pid_t>(-1);
            }));
        } catch (const std::system_error &error) {
            observed_error = error.code().value();
        }

        jh::test::tiny_test::expect(observed_error == EAGAIN, "fork failure preserves errno");
    }
}

template<>
struct jh::test::tiny_test::test<"process launcher reports fork failure">
    : jh::test::tiny_test::test_definition<
          "process launcher reports fork failure",
          &::test::process_launcher_fork_failure_contracts
      > {};

template<>
struct jh::test::tiny_test::session<"process_launcher_fork_failure">
    : jh::test::tiny_test::session_definition<
          "process_launcher_fork_failure",
          jh::test::tiny_test::test<"process launcher reports fork failure">
      > {};

namespace test {
    [[maybe_unused]] const tiny_test::session<"process_launcher_fork_failure"> process_launcher_session{};
}
