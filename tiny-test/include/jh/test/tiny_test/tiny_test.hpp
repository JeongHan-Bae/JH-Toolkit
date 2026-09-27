/**
 * @copyright
 * Copyright 2025 JeongHan-Bae &lt;mastropseudo\@gmail.com&gt;
 * <br>
 * Licensed under the Apache License, Version 2.0 (the "License"); <br>
 * you may not use this file except in compliance with the License.<br>
 * You may obtain a copy of the License at<br>
 * <br>
 *     http://www.apache.org/licenses/LICENSE-2.0<br>
 * <br>
 * Unless required by applicable law or agreed to in writing, software<br>
 * distributed under the License is distributed on an "AS IS" BASIS,<br>
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.<br>
 * See the License for the specific language governing permissions and<br>
 * limitations under the License.<br>
 * <br>
 * Full license: <a href="https://github.com/JeongHan-Bae/JH-Toolkit?tab=Apache-2.0-1-ov-file#readme">GitHub</a>
 */
/**
 * @file tiny_test.hpp
 * @brief Compile-time named tests and sessions with expected-based results.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "jh/metax/expected.h"
#include "jh/metax/t_str.h"
#include "jh/typing/monostate.h"

namespace jh::test::tiny_test {
    /// @brief Identifies why a test result did not pass.
    enum class test_error : std::uint8_t {
        expectation_failed,
        exception_not_thrown,
        unexpected_exception,
        exception,
        unknown_exception
    };

    /// @brief Identifies a requested test or session that is not present in a suite.
    enum class runner_error : std::uint8_t {
        unknown_test,
        unknown_session
    };

    /// @brief Stores one test's expected status and assertion or exception messages.
    struct result final {
        /// @brief Compile-time test name copied for reporting.
        std::string name;
        /// @brief Successful status, or the first recorded failure category.
        jh::meta::expected<bool, test_error> status{true};
        /// @brief Assertion and exception messages in occurrence order.
        std::vector<std::string> errors;

        /**
         * @brief Reports whether the test completed without a recorded failure.
         * @return <code>true</code> when the expected status contains <code>true</code>.
         */
        [[nodiscard]] bool passed() const noexcept {
            return status.has_value() && *status;
        }
    };

    /// @brief Stores test outcomes for one named session.
    struct session_result final {
        /// @brief Compile-time session name copied for reporting.
        std::string name;
        /// @brief Test outcomes in their declared order.
        std::vector<result> tests;

        /**
         * @brief Reports whether every test in the session passed.
         * @return <code>true</code> when every stored test result passed.
         */
        [[nodiscard]] bool passed() const noexcept {
            for (const auto& test_result : tests) {
                if (!test_result.passed()) return false;
            }
            return true;
        }
    };

    /// @brief Stores outcomes for every session in a suite.
    struct suite_result final {
        /// @brief Session outcomes in their declared order.
        std::vector<session_result> sessions;

        /**
         * @brief Reports whether every session in the suite passed.
         * @return <code>true</code> when every stored session result passed.
         */
        [[nodiscard]] bool passed() const noexcept {
            for (const auto& session_value : sessions) {
                if (!session_value.passed()) return false;
            }
            return true;
        }
    };

    namespace detail {
        inline thread_local result* active_result{};

        class result_scope final {
            result* previous_;

        public:
            explicit result_scope(result& current) noexcept
                : previous_(active_result) {
                active_result = &current;
            }

            ~result_scope() {
                active_result = previous_;
            }
        };

        inline std::string make_message(const std::string_view prefix, const std::string_view message) {
            std::string output{prefix};
            if (!message.empty()) {
                output.append(": ");
                output.append(message);
            }
            return output;
        }

        inline void
        record_failure(std::string message, const test_error error = test_error::expectation_failed) {
            if (active_result != nullptr) {
                if (active_result->status.has_value()) {
                    active_result->status = jh::meta::unexpected(error);
                }
                active_result->errors.push_back(std::move(message));
            }
        }

        inline void record_exception(std::string message, const test_error error) {
            if (active_result == nullptr) return;
            if (active_result->status.has_value()) {
                active_result->status = jh::meta::unexpected(error);
            }
            active_result->errors.push_back(std::move(message));
        }

        struct registered_session final {
            std::string name;
            std::vector<std::string> test_names;
            session_result (*run_all)();
            jh::meta::expected<result, runner_error> (*run_one)(std::string_view);
        };

        struct registry_state final {
            std::vector<registered_session> sessions;
            std::vector<std::string> errors;

            void add_session(
                std::string name,
                std::vector<std::string> test_names,
                session_result (*run_all)(),
                jh::meta::expected<result, runner_error> (*run_one)(std::string_view)
            ) {
                bool ambiguous = false;
                const auto report_ambiguity = [&](const std::string_view duplicate_name,
                                                  const std::string_view current_kind,
                                                  const std::string_view existing_kind) {
                    std::string message{"ambiguous registered name '"};
                    message.append(duplicate_name);
                    message.append("': ");
                    message.append(current_kind);
                    message.append(" conflicts with an existing ");
                    message.append(existing_kind);
                    errors.push_back(std::move(message));
                    ambiguous = true;
                };

                for (const auto& current : sessions) {
                    if (current.name == name) {
                        report_ambiguity(name, "session", "session");
                    }
                    for (const auto& current_test_name : current.test_names) {
                        if (current_test_name == name) {
                            report_ambiguity(name, "session", "test");
                        }
                    }
                    for (const auto& test_name : test_names) {
                        if (test_name == current.name) {
                            report_ambiguity(test_name, "test", "session");
                        }
                        for (const auto& current_test_name : current.test_names) {
                            if (current_test_name == test_name) {
                                report_ambiguity(test_name, "test", "test");
                            }
                        }
                    }
                }
                if (ambiguous) return;
                sessions.push_back({
                    std::move(name),
                    std::move(test_names),
                    run_all,
                    run_one
                });
            }
        };

        inline registry_state& registry() {
            static registry_state instance;
            return instance;
        }

        template<std::size_t N>
        [[nodiscard]] consteval bool unique_names(const std::array<std::string_view, N>& names) {
            for (std::size_t left = 0; left < N; ++left) {
                for (std::size_t right = left + 1; right < N; ++right) {
                    if (names[left] == names[right]) return false;
                }
            }
            return true;
        }

        template<jh::meta::TStr SessionName, class... Tests>
        [[nodiscard]] consteval bool unique_names_in_session() {
            const std::array<std::string_view, sizeof...(Tests) + 1> names{
                SessionName.view(), Tests::name.view()...
            };
            return unique_names(names);
        }

        template<class Session, std::size_t N, std::size_t... Indices>
        consteval void append_test_names(
            std::array<std::string_view, N>& names,
            std::size_t& position,
            std::index_sequence<Indices...>
        ) {
            ((names[position++] = Session::template test_name_at<Indices>()), ...);
        }

        template<class... Sessions>
        [[nodiscard]] consteval bool unique_names_in_suite() {
            constexpr std::size_t test_total = (Sessions::test_count + ... + 0);
            std::array<std::string_view, sizeof...(Sessions) + test_total> names{};
            std::size_t position = 0;
            ((names[position++] = Sessions::name.view()), ...);
            (append_test_names<Sessions>(
                names,
                position,
                std::make_index_sequence<Sessions::test_count>{}
            ), ...);
            return unique_names(names);
        }
    }

    /**
     * @brief Checks a condition and records a failure message in the active test result when false.
     * @param condition Condition that must be true.
     * @param message Optional description used when the condition is false.
     * @details When called inside a test, the failure message is stored and execution continues
     *          so later expectations can also be reported.
     */
    inline void expect(const bool condition, const std::string_view message = {}) {
        if (condition) return;
        detail::record_failure(detail::make_message("expectation failed", message));
    }

    /**
     * @brief Checks that a condition is false and records a failure when it is true.
     * @param condition Condition that must be false.
     * @param message Optional description used when the condition is true.
     * @details During a test run, the failure message is stored and execution continues.
     */
    inline void expect_not(const bool condition, const std::string_view message = {}) {
        if (!condition) return;
        detail::record_failure(detail::make_message("expect_not failed", message));
    }

    /**
     * @brief Checks that a callable throws, optionally requiring a specific exception type.
     * @tparam Exception Exception type to accept, or <code>jh::typed::monostate</code> to accept any exception.
     * @tparam Callable Nullary callable whose exception behavior is checked.
     * @param callable Callable expected to throw.
     * @param message Optional description included when the expectation fails.
     * @details The default form catches any C++ exception. A typed form catches the requested type;
     *          another type is caught by the outer handler and recorded as a failure. The outcome
     *          is written to the active test result and no value is returned to the caller.
     */
    template<class Exception = jh::typed::monostate, class Callable>
    requires std::is_invocable_v<std::decay_t<Callable>&>
    inline void expect_throw(Callable&& callable, const std::string_view message = {}) {
        bool thrown = false;
        bool non_expected_throw = false;
        std::string unexpected_message;
        try {
            if constexpr (jh::typed::monostate_t<Exception>) {
                try {
                    std::invoke(std::forward<Callable>(callable));
                } catch (...) {
                    thrown = true;
                }
            } else {
                try {
                    std::invoke(std::forward<Callable>(callable));
                } catch (const Exception&) {
                    thrown = true;
                }
            }
        } catch (const std::exception& error) {
            unexpected_message = error.what();
            non_expected_throw = true;
        } catch (...) {
            unexpected_message = "unknown exception type";
            non_expected_throw = true;
        }

        if (non_expected_throw) {
            std::string failure = detail::make_message(
                "expect_throw caught a non-matching exception",
                unexpected_message
            );
            if (!message.empty()) {
                failure.append(" (");
                failure.append(message);
                failure.push_back(')');
            }
            detail::record_failure(std::move(failure), test_error::unexpected_exception);
            return;
        }
        if (!thrown) {
            detail::record_failure(
                detail::make_message("expect_throw found no exception", message),
                test_error::exception_not_thrown
            );
            return;
        }
    }

    /**
     * @brief Injection point for one explicitly named test specialization.
     * @tparam Name Compile-time test name.
     * @details Define the specialization once in <code>jh::test::tiny_test</code>. Keep the
     *          test body in an application namespace and let the specialization refer to it.
     */
    template<jh::meta::TStr Name>
    struct test;

    /**
     * @brief Implements a named test by invoking a compile-time callable and collecting its result.
     * @tparam Name Compile-time test name.
     * @tparam Callable Captureless callable stored as a non-type template parameter.
     */
    template<jh::meta::TStr Name, auto Callable>
    struct test_definition {
        /// @brief The compile-time name of this test definition.
        inline static constexpr auto name = Name;

        /**
         * @brief Runs the callable behind the test's outer exception boundary.
         * @return Test status and all assertion or exception messages.
         */
        [[nodiscard]] static result run() {
            result output;
            output.name = std::string{Name.view()};
            detail::result_scope scope{output};
            try {
                std::invoke(Callable);
            } catch (const std::exception& error) {
                detail::record_exception(error.what(), test_error::exception);
            } catch (...) {
                detail::record_exception("unknown exception", test_error::unknown_exception);
            }
            return output;
        }
    };

    /**
     * @brief Injection point for one explicitly named session specialization.
     * @tparam Name Compile-time session name.
     * @details Define the specialization once in <code>jh::test::tiny_test</code> and list its
     *          test specializations there.
     */
    template<jh::meta::TStr Name>
    struct session;

    /**
     * @brief Implements a session from a compile-time list of named tests.
     * @tparam Name Compile-time session name.
     * @tparam Tests Test specializations included in this session.
     * @details Duplicate test names, including a test name equal to the session name, are rejected
     *          at compile time. Construct one session object in this translation unit to register
     *          it with the global runner. Registered names must be unique across the executable.
     */
    template<jh::meta::TStr Name, class... Tests>
    struct session_definition {
        static_assert(
            detail::unique_names_in_session<Name, Tests...>(),
            "TinyTest session and test names must be unique"
        );

        /// @brief The compile-time name of this session.
        inline static constexpr auto name = Name;
        /// @brief Number of tests in this session.
        inline static constexpr std::size_t test_count = sizeof...(Tests);

        /** @brief Registers this translation-unit-local session for the global runner. */
        session_definition() {
            std::vector<std::string> names;
            names.reserve(sizeof...(Tests));
            (names.emplace_back(Tests::name.view()), ...);
            detail::registry().add_session(
                std::string{Name.view()},
                std::move(names),
                &session_definition::run,
                &session_definition::run_one
            );
        }

        /**
         * @brief Runs every test in declaration order.
         * @return Session name and the result of every test.
         */
        [[nodiscard]] static session_result run() {
            session_result output;
            output.name = std::string{Name.view()};
            output.tests.reserve(sizeof...(Tests));
            (output.tests.push_back(Tests::run()), ...);
            return output;
        }

        /**
         * @brief Runs one test selected by its compile-time name.
         * @param test_name Test name to locate.
         * @return The test result, or <code>runner_error::unknown_test</code>.
         */
        [[nodiscard]] static jh::meta::expected<result, runner_error>
        run_one(const std::string_view test_name) {
            jh::meta::expected<result, runner_error> output =
                jh::meta::unexpected(runner_error::unknown_test);
            bool found = false;
            const auto select = [&](const auto* test_type) {
                using selected_type = std::remove_pointer_t<decltype(test_type)>;
                if (!found && selected_type::name.view() == test_name) {
                    output = selected_type::run();
                    found = true;
                }
            };
            (select(static_cast<Tests*>(nullptr)), ...);
            return output;
        }

        /**
         * @brief Visits test names in declaration order without running the tests.
         * @tparam Visitor Callable accepting a <code>std::string_view</code> name.
         * @param visitor Function called once for each test name.
         */
        template<class Visitor>
        static void visit_test_names(Visitor&& visitor) {
            (std::invoke(visitor, Tests::name.view()), ...);
        }

        /**
         * @brief Returns a test name by its compile-time index.
         * @tparam Index Zero-based test index.
         * @return The test name at <code>Index</code>.
         */
        template<std::size_t Index>
        [[nodiscard]] static consteval std::string_view test_name_at() {
            using test_type = std::tuple_element_t<Index, std::tuple<Tests...>>;
            return test_type::name.view();
        }
    };

    /**
     * @brief Collects compile-time named sessions and rejects duplicate names.
     * @tparam Sessions Session specializations to run.
     * @details Duplicate session names and duplicate test names across the suite are compile errors.
     */
    template<class... Sessions>
    class suite final {
        static_assert(
            detail::unique_names_in_suite<Sessions...>(),
            "TinyTest suite, session, and test names must be unique"
        );

    public:
        /// @brief Number of sessions in this suite.
        inline static constexpr std::size_t session_count = sizeof...(Sessions);

        /**
         * @brief Runs every session in declaration order.
         * @return The outcome of every session.
         */
        [[nodiscard]] suite_result run() const {
            suite_result output;
            output.sessions.reserve(sizeof...(Sessions));
            (output.sessions.push_back(Sessions::run()), ...);
            return output;
        }

        /**
         * @brief Runs one session selected by its compile-time name.
         * @param session_name Session name to locate.
         * @return The session result, or <code>runner_error::unknown_session</code>.
         */
        [[nodiscard]] jh::meta::expected<session_result, runner_error>
        run_session(const std::string_view session_name) const {
            jh::meta::expected<session_result, runner_error> output =
                jh::meta::unexpected(runner_error::unknown_session);
            bool found = false;
            const auto select = [&](const auto* session_type) {
                using selected_type = std::remove_pointer_t<decltype(session_type)>;
                if (!found && selected_type::name.view() == session_name) {
                    output = selected_type::run();
                    found = true;
                }
            };
            (select(static_cast<Sessions*>(nullptr)), ...);
            return output;
        }

        /**
         * @brief Runs one test selected by its compile-time name.
         * @param test_name Test name to locate.
         * @return The test result, or <code>runner_error::unknown_test</code>.
         */
        [[nodiscard]] jh::meta::expected<result, runner_error>
        run_test(const std::string_view test_name) const {
            jh::meta::expected<result, runner_error> output =
                jh::meta::unexpected(runner_error::unknown_test);
            bool found = false;
            const auto select = [&](const auto* session_type) {
                using selected_type = std::remove_pointer_t<decltype(session_type)>;
                if (!found) {
                    auto candidate = selected_type::run_one(test_name);
                    if (candidate) {
                        output = std::move(*candidate);
                        found = true;
                    }
                }
            };
            (select(static_cast<Sessions*>(nullptr)), ...);
            return output;
        }

        /**
         * @brief Visits sessions in declaration order without running them.
         * @tparam Visitor Callable accepting a session type.
         * @param visitor Function called once for each session type.
         */
        template<class Visitor>
        static void visit_sessions(Visitor&& visitor) {
            (std::invoke(visitor, static_cast<Sessions*>(nullptr)), ...);
        }
    };

    /// @brief Compatibility spelling for <code>suite&lt;Sessions...&gt;</code>.
    template<class... Sessions>
    using Suite = suite<Sessions...>;

    /**
     * @brief Creates a suite from test and session specializations.
     * @tparam Sessions Session specializations included in the suite.
     * @return A suite value with compile-time duplicate-name validation.
     */
    template<class... Sessions>
    [[nodiscard]] suite<Sessions...> make_suite() {
        return {};
    }

    /**
     * @brief Formats one test result as readable text.
     * @param value Test result to format.
     * @return A report string suitable for a console or logger.
     */
    [[nodiscard]] inline std::string format_report(const result& value) {
        std::string report = value.passed() ? "[ OK ] " : "[FAIL] ";
        report.append(value.name);
        report.push_back('\n');
        for (const auto& message : value.errors) {
            report.append("  ");
            report.append(message);
            report.push_back('\n');
        }
        return report;
    }

    /**
     * @brief Formats a session result, including every test failure message.
     * @param value Session result to format.
     * @return A report string suitable for a console or logger.
     */
    [[nodiscard]] inline std::string format_report(const session_result& value) {
        std::string report = "Session: ";
        report.append(value.name);
        report.push_back('\n');
        for (const auto& test_result : value.tests) report.append(format_report(test_result));
        return report;
    }

    /**
     * @brief Formats a suite result, including every session and test.
     * @param value Suite result to format.
     * @return A report string suitable for a console or logger.
     */
    [[nodiscard]] inline std::string format_report(const suite_result& value) {
        std::string report;
        for (const auto& session_value : value.sessions) report.append(format_report(session_value));
        return report;
    }

    /**
     * @brief Lists or runs translation-unit-local sessions registered by static session objects.
     * @param argc Argument count passed to <code>main</code>.
     * @param argv Argument values passed to <code>main</code>.
     * @return Zero when the selected work passes, or the number of failed tests.
     * @details Pass <code>--list</code> to list registered names, a session name to run every
     *          matching session, or a test name to run every matching test. With no selection,
     *          all registered sessions run. Duplicate names registered by different translation
     *          units in the same executable are reported as failures before tests run. Separate
     *          executables have independent registries.
     */
    inline int tiny_main(const int argc, char** argv) {
        auto& state = detail::registry();
        if (!state.errors.empty()) {
            for (const auto& error : state.errors) {
                std::cerr << "[FAIL] " << error << '\n';
            }
            return 1;
        }
        auto& registered = state.sessions;
        if (argc > 1 && argv != nullptr && argv[1] != nullptr) {
            const std::string_view argument{argv[1]};
            if (argument == "--list") {
                for (const auto& current : registered) {
                    std::cout << current.name << '\n';
                    for (const auto& test_name : current.test_names) {
                        std::cout << "  " << test_name << '\n';
                    }
                }
                return 0;
            }

            int failures = 0;
            bool selected = false;
            for (const auto& current : registered) {
                if (current.name != argument) continue;
                selected = true;
                const auto output = current.run_all();
                std::cout << format_report(output);
                for (const auto& test_result : output.tests) {
                    if (!test_result.passed()) ++failures;
                }
            }
            if (selected) return failures;

            for (const auto& current : registered) {
                bool contains_test = false;
                for (const auto& test_name : current.test_names) {
                    if (test_name == argument) {
                        contains_test = true;
                        break;
                    }
                }
                if (!contains_test) continue;
                selected = true;
                const auto output = current.run_one(argument);
                if (!output) continue;
                std::cout << format_report(*output);
                if (!output->passed()) ++failures;
            }
            if (selected) return failures;

            std::cerr << "No such test or session: " << argument << '\n';
            return 1;
        }

        int failures = 0;
        for (const auto& current : registered) {
            const auto output = current.run_all();
            std::cout << format_report(output);
            for (const auto& test_result : output.tests) {
                if (!test_result.passed()) ++failures;
            }
        }
        return failures;
    }

    /**
     * @brief Lists or runs selected named items from a suite.
     * @tparam Sessions Session specializations in the suite.
     * @param tests Compile-time validated suite to run.
     * @param argc Argument count passed to <code>main</code>.
     * @param argv Argument values passed to <code>main</code>.
     * @return Zero when the selected work passes, or the number of failed tests.
     * @details Pass <code>--list</code> to list names, a session name to run that session,
     *          or a test name to run one test. With no selection it runs the full suite.
     */
    template<class... Sessions>
    inline int tiny_main(const suite<Sessions...>& tests, const int argc, char** argv) {
        if (argc > 1 && argv != nullptr && argv[1] != nullptr) {
            const std::string_view argument{argv[1]};
            if (argument == "--list") {
                suite<Sessions...>::template visit_sessions<>([](const auto* session_type) {
                    using selected_type = std::remove_pointer_t<decltype(session_type)>;
                    std::cout << selected_type::name.view() << '\n';
                    selected_type::template visit_test_names<>([](const std::string_view test_name) {
                        std::cout << "  " << test_name << '\n';
                    });
                });
                return 0;
            }

            const auto selected_session = tests.run_session(argument);
            if (selected_session) {
                std::cout << format_report(*selected_session);
                return selected_session->passed() ? 0 : 1;
            }

            const auto selected_test = tests.run_test(argument);
            if (selected_test) {
                std::cout << format_report(*selected_test);
                return selected_test->passed() ? 0 : 1;
            }

            std::cerr << "No such test or session: " << argument << '\n';
            return 1;
        }

        const auto output = tests.run();
        std::cout << format_report(output);
        int failures = 0;
        for (const auto& session_value : output.sessions) {
            for (const auto& test_result : session_value.tests) {
                if (!test_result.passed()) ++failures;
            }
        }
        return failures;
    }
}

#ifdef TINY_TEST_MAIN
/**
 * @brief Runs all statically registered TinyTest sessions from the process command line.
 * @details Define <code>TINY_TEST_MAIN</code> in one translation unit before including this header.
 */
int main(int argc, char** argv) {
    return jh::test::tiny_test::tiny_main(argc, argv);
}
#endif
