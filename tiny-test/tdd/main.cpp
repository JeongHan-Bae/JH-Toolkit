#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

namespace test {
    void passing_expectations() {
        tiny_test::expect(true, "true expectation");
        tiny_test::expect_not(false, "false expectation");
    }

    void multiple_expectation_failures() {
        tiny_test::expect(false, "first failure");
        tiny_test::expect_not(true, "second failure");
    }

    void untyped_exception() {
        tiny_test::expect_throw([] { throw 7; });
    }

    void typed_exception() {
        tiny_test::expect_throw<std::domain_error>([] {
            throw std::domain_error("expected domain error");
        });
    }

    void missing_exception() {
        tiny_test::expect_throw([] {}, "must throw");
    }

    void unexpected_typed_exception() {
        tiny_test::expect_throw<std::domain_error>([] {
            throw std::logic_error("wrong type detail");
        });
    }

    void outer_standard_exception() {
        throw std::runtime_error("outer standard detail");
    }

    void outer_non_standard_exception() {
        throw 9;
    }
}

template<>
struct jh::test::tiny_test::test<"passing expectations">
    : jh::test::tiny_test::test_definition<"passing expectations", &::test::passing_expectations> {};

template<>
struct jh::test::tiny_test::test<"multiple expectation failures">
    : jh::test::tiny_test::test_definition<"multiple expectation failures", &::test::multiple_expectation_failures> {};

template<>
struct jh::test::tiny_test::test<"untyped exception">
    : jh::test::tiny_test::test_definition<"untyped exception", &::test::untyped_exception> {};

template<>
struct jh::test::tiny_test::test<"typed exception">
    : jh::test::tiny_test::test_definition<"typed exception", &::test::typed_exception> {};

template<>
struct jh::test::tiny_test::test<"missing exception">
    : jh::test::tiny_test::test_definition<"missing exception", &::test::missing_exception> {};

template<>
struct jh::test::tiny_test::test<"unexpected typed exception">
    : jh::test::tiny_test::test_definition<"unexpected typed exception", &::test::unexpected_typed_exception> {};

template<>
struct jh::test::tiny_test::test<"outer standard exception">
    : jh::test::tiny_test::test_definition<"outer standard exception", &::test::outer_standard_exception> {};

template<>
struct jh::test::tiny_test::test<"outer non-standard exception">
    : jh::test::tiny_test::test_definition<"outer non-standard exception", &::test::outer_non_standard_exception> {};

template<>
struct jh::test::tiny_test::session<"runtime behavior">
    : jh::test::tiny_test::session_definition<
          "runtime behavior",
          jh::test::tiny_test::test<"passing expectations">,
          jh::test::tiny_test::test<"multiple expectation failures">,
          jh::test::tiny_test::test<"untyped exception">,
          jh::test::tiny_test::test<"typed exception">,
          jh::test::tiny_test::test<"missing exception">,
          jh::test::tiny_test::test<"unexpected typed exception">,
          jh::test::tiny_test::test<"outer standard exception">,
          jh::test::tiny_test::test<"outer non-standard exception">
      > {};

namespace test {
    [[maybe_unused]] const tiny_test::session<"runtime behavior"> runtime_session_registration{};
}

namespace {
    class scoped_streambuf_redirect final {
        std::ostream* stream_{};
        std::streambuf* previous_{};

    public:
        scoped_streambuf_redirect(std::ostream& stream, std::streambuf* replacement) noexcept
            : stream_(&stream), previous_(stream.rdbuf(replacement)) {}

        scoped_streambuf_redirect(const scoped_streambuf_redirect&) = delete;
        scoped_streambuf_redirect& operator=(const scoped_streambuf_redirect&) = delete;

        scoped_streambuf_redirect(scoped_streambuf_redirect&& other) noexcept
            : stream_(std::exchange(other.stream_, nullptr)), previous_(other.previous_) {}

        scoped_streambuf_redirect& operator=(scoped_streambuf_redirect&&) = delete;

        ~scoped_streambuf_redirect() noexcept {
            if (stream_ != nullptr) stream_->rdbuf(previous_);
        }
    };

    struct check_result {
        bool passed{};
        std::string description;
    };

    void verify(
        std::vector<check_result>& checks,
        const bool condition,
        const std::string_view description
    ) {
        checks.push_back({condition, std::string{description}});
    }
}

int main() {
    std::vector<check_result> checks;
    const auto outcome = test::tiny_test::session<"runtime behavior">::run();

    verify(checks, outcome.name == "runtime behavior", "session result retains its compile-time name");
    verify(checks, outcome.tests.size() == 8, "all named tests ran in declaration order");
    if (outcome.tests.size() != 8) return 1;
    verify(checks, outcome.tests[0].passed(), "expect and expect_not accept matching conditions");

    const auto& assertion_failures = outcome.tests[1];
    verify(checks, !assertion_failures.passed(), "failed expectations mark the test as failed");
    verify(checks, assertion_failures.errors.size() == 2, "multiple failed expectations are retained");
    verify(checks,
           assertion_failures.errors.size() >= 2 &&
               assertion_failures.errors[0].find("first failure") != std::string::npos &&
               assertion_failures.errors[1].find("second failure") != std::string::npos,
           "expectation messages retain occurrence order");

    verify(checks, outcome.tests[2].passed(), "default expect_throw accepts any exception type");
    verify(checks, outcome.tests[3].passed(), "typed expect_throw accepts the requested exception type");

    const auto& missing_exception = outcome.tests[4];
    verify(checks,
           !missing_exception.passed() &&
               missing_exception.status.has_error() &&
               missing_exception.status.error() == test::tiny_test::test_error::exception_not_thrown,
           "expect_throw fails when the callable does not throw");
    verify(checks,
           !missing_exception.errors.empty() &&
               missing_exception.errors[0].find("must throw") != std::string::npos,
           "missing exception failure keeps its message");

    const auto& unexpected_exception = outcome.tests[5];
    verify(checks,
           !unexpected_exception.passed() &&
               unexpected_exception.status.has_error() &&
               unexpected_exception.status.error() == test::tiny_test::test_error::unexpected_exception,
           "typed expect_throw fails for a different exception type");
    verify(checks,
           !unexpected_exception.errors.empty() &&
               unexpected_exception.errors[0].find("wrong type detail") != std::string::npos,
           "typed mismatch keeps the thrown exception message");

    verify(checks,
           outcome.tests[6].status.has_error() &&
               outcome.tests[6].status.error() == test::tiny_test::test_error::exception &&
               !outcome.tests[6].errors.empty() &&
               outcome.tests[6].errors[0] == "outer standard detail",
           "outer catch records standard exception text");
    verify(checks,
           outcome.tests[7].status.has_error() &&
               outcome.tests[7].status.error() == test::tiny_test::test_error::unknown_exception &&
               !outcome.tests[7].errors.empty() &&
               outcome.tests[7].errors[0] == "unknown exception",
           "outer catch records a fallback message for non-standard exceptions");

    using runtime_suite = test::tiny_test::suite<test::tiny_test::session<"runtime behavior">>;
    const auto selected = runtime_suite{}.run_test("typed exception");
    verify(checks, selected && selected->passed(), "suite can select a test by its compile-time name");
    const auto missing = runtime_suite{}.run_test("not registered");
    verify(checks,
           !missing && missing.error() == test::tiny_test::runner_error::unknown_test,
           "suite returns an expected error for an unknown test");

    std::ostringstream runner_output;
    char list_argument[] = "--list";
    char* list_argv[]{nullptr, list_argument};
    int list_exit{};
    {
        const scoped_streambuf_redirect redirect{std::cout, runner_output.rdbuf()};
        list_exit = test::tiny_test::tiny_main(2, list_argv);
    }
    verify(checks,
           list_exit == 0 && runner_output.str().find("runtime behavior") != std::string::npos,
           "tiny_main lists compile-time session and test names");

    runner_output.str({});
    char test_argument[] = "typed exception";
    char* test_argv[]{nullptr, test_argument};
    int test_exit{};
    {
        const scoped_streambuf_redirect redirect{std::cout, runner_output.rdbuf()};
        test_exit = test::tiny_test::tiny_main(2, test_argv);
    }
    verify(checks,
           test_exit == 0 && runner_output.str().find("[ OK ] typed exception") != std::string::npos,
           "tiny_main runs one selected test");

    int failures = 0;
    for (const auto& check : checks) {
        if (check.passed) continue;
        std::cerr << "[FAIL] " << check.description << '\n';
        ++failures;
    }
    if (failures == 0) {
        std::cout << "All TinyTest behavior checks passed.\n";
    }
    return failures;
}
