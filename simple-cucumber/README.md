<h1 align="center">
  <span>
    <img src="../assets/img/simple-cucumber.skill.svg"
         alt="JH-Simple-Cucumber Skill Icon"
         width="72" valign="middle">
  </span>
  <span style="font-size: x-large;">&nbsp;</span>
  <span style="font-size: x-large;">JH-Simple-Cucumber</span>
</h1>

<font size="4"><code>jh-simple-cucumber</code></font> is a small C++20 test library for typed Cucumber step definitions and Gherkin feature files. Its C++ API lives in `jh::test::cucumber`; the Gherkin C++ parser is fetched from the official Cucumber Gherkin project.

## Installation

`jh-simple-cucumber` depends on the JH-Toolkit headers and `jh::meta::expected`. Its installed CMake target uses the shared `jh::test::` package namespace. From a JH-Toolkit checkout, configure and install it as a standalone CMake package:

```sh
cmake -S simple-cucumber -B build/simple-cucumber \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$HOME/.local/jh-simple-cucumber"
cmake --build build/simple-cucumber
cmake --install build/simple-cucumber
```

The install prefix can be any directory you can write to. When using the installed package from another CMake project, install JH-Toolkit first and make both package prefixes discoverable through `CMAKE_PREFIX_PATH`:

```cmake
find_package(jh-toolkit CONFIG REQUIRED)
find_package(jh-simple-cucumber CONFIG REQUIRED)
target_link_libraries(my_bdd_runner PRIVATE jh::test::jh-simple-cucumber)
```

The `jh::test::jh-simple-cucumber-gherkin` adapter publicly links `jh::jh-toolkit`, since its headers use toolkit types. The package config also locates the Gherkin parser and Cucumber Messages dependencies. Parser dependencies are fetched by default; set `JH_SIMPLE_CUCUMBER_FETCH_PARSER_DEPS=OFF` to use an installed Cucumber Messages package instead.

### Fetch

The subproject can be fetched directly from the JH-Toolkit repository with CMake `FetchContent`:

```cmake
include(FetchContent)

FetchContent_Declare(
    jh_simple_cucumber
    GIT_REPOSITORY https://github.com/JeongHan-Bae/JH-Toolkit.git
    GIT_TAG <release-or-commit>
    SOURCE_SUBDIR simple-cucumber
)
FetchContent_MakeAvailable(jh_simple_cucumber)

target_link_libraries(my_bdd_runner PRIVATE jh::test::jh-simple-cucumber)
```

The project locates the JH-Toolkit root when fetched from this repository. Its Gherkin parser dependency is fetched during CMake configuration. Set `JH_SIMPLE_CUCUMBER_INSTALL=ON` only when the subproject's install rules are needed.

### Tests

The library's own behavior suite is in `simple-cucumber/tdd` and runs through CTest. Build and run it from the repository root:

```sh
cmake -S simple-cucumber -B build/simple-cucumber \
  -DJH_SIMPLE_CUCUMBER_BUILD_TDD=ON \
  -DJH_SIMPLE_CUCUMBER_BUILD_STATIC_TESTS=ON
cmake --build build/simple-cucumber
ctest --test-dir build/simple-cucumber --output-on-failure
```

The build target `jh_simple_cucumber_tdd` produces the runtime test executable. The `3/3` shown by CTest counts
three test entries: `simple_cucumber_tdd`, `simple_cucumber_static_passes`, and
`simple_cucumber_static_failures`. The first runs the test executable; the
other two compile their static cases during CTest execution.

Passing scenarios and their step definitions live in `tdd/pass/`; expected failures are in `tdd/fail/`. The suite covers scalar conversion, type-aware candidate selection, exponent-form doubles, constructor-injected services, `And`, Scenario Outlines, per-scenario state, DataTables, undefined steps, conversion failures, and ambiguous definitions. `JH_SIMPLE_CUCUMBER_BUILD_TDD` defaults on for standalone builds and when JH-Toolkit tests are enabled.

Compile-time API contracts live in `static_tests/pass/` and `static_tests/fail/`. They run through separate pass and fail CTest runners; the fail runner collects all rejected-compilation checks before reporting mismatches. They are controlled by `JH_SIMPLE_CUCUMBER_BUILD_STATIC_TESTS`, which defaults on alongside the TDD checks.

## Usage

### Basic API

Define one `StepDefinition` by binding Gherkin expressions to `void` member or static functions. Expressions are compile-time `jh::meta::TStr` values, so their syntax and callable signatures can be checked at compile time. `Given`, `When`, and `Then` describe the step kind; an `And` step inherits the kind of the preceding step from the Gherkin parser.

The built-in placeholders map to these C++ argument types:

| Expression | C++ type |
| --- | --- |
| `<string>` | `std::string_view` |
| `<int>` | `std::int64_t` |
| `<uint>` | `std::uint64_t` |
| `<double>` | `double` |
| `<bool>` | `bool` |

The optional trailing parameters are `const DataTable&` and `StepContext&`, in that order. Bindings support const and non-const, static and non-static, `constexpr`, `noexcept`, and `noexcept(false)` functions. `<string>` borrows the current step text. Copy it into `std::string` for use after the feature run.

Within one `StepDefinition`, each `StepKind` and expression pair must be unique; a duplicate fails compilation. A DataTable binding takes `const DataTable&`, optionally followed by `StepContext&`. A table mismatch records a step failure in `RunResult` before callable invocation. Each `StepDefinition` is checked for the feature run that uses it.

Calls to `run<Definition>(feature)` and `run_file<Definition>(path)` default-construct each scenario's step object. Here is a basic definition for the user feature:

```cpp
class BasicUserSteps {
    bool user_exists_{};

public:
    void i_am_a_user() { user_exists_ = true; }
    void i_can_sign_in(jh::test::cucumber::StepContext& context) const {
        context.expect(user_exists_, "user should exist");
    }
};

using BasicUserDefinition = jh::test::cucumber::StepDefinition<
    BasicUserSteps,
    jh::test::cucumber::Given<"I am a user", &BasicUserSteps::i_am_a_user>,
    jh::test::cucumber::Then<"I can sign in", &BasicUserSteps::i_can_sign_in>
>;
```

The same steps can use a port so the feature can exercise different adapters. Each adapter implements the `UserService` interface; the step class depends only on that port:

```cpp
#include "database/handle.hpp"

class UserService {
public:
    virtual ~UserService() = default;
    virtual void add_user() = 0;
    [[nodiscard]] virtual bool has_user() const = 0;
};

class InMemoryUserService final : public UserService {
    bool user_exists_{};

public:
    void add_user() override { user_exists_ = true; }
    [[nodiscard]] bool has_user() const override { return user_exists_; }
};

class DatabaseUserService final : public UserService {
    database::handle& database_;

public:
    explicit DatabaseUserService(database::handle& database): database_(database) {}
    void add_user() override { database_.add_user(); }
    [[nodiscard]] bool has_user() const override { return database_.has_user(); }
};

class UserServiceSteps {
    UserService& service_;

public:
    explicit UserServiceSteps(UserService& service): service_(service) {}
    void i_am_a_user() { service_.add_user(); }
    void i_can_sign_in(jh::test::cucumber::StepContext& context) const {
        context.expect(service_.has_user(), "user should exist");
    }
};

using InMemoryDefinition = jh::test::cucumber::StepDefinition<
    UserServiceSteps,
    jh::test::cucumber::Given<"I am a user", &UserServiceSteps::i_am_a_user>,
    jh::test::cucumber::Then<"I can sign in", &UserServiceSteps::i_can_sign_in>
>;

using DatabaseDefinition = jh::test::cucumber::StepDefinition<
    UserServiceSteps,
    jh::test::cucumber::Given<"I am a user", &UserServiceSteps::i_am_a_user>,
    jh::test::cucumber::Then<"I can sign in", &UserServiceSteps::i_can_sign_in>
>;
```

After the definitions, run the same feature with the default-constructed steps or with either adapter:

```cpp
extern database::handle global_db_handle;

// Run the same feature file with default-constructed steps.
const auto basic = jh::test::cucumber::run_file<BasicUserDefinition>(feature_path);

// Run that same feature with different adapters.
const auto in_memory = jh::test::cucumber::run_file<InMemoryDefinition>(
    feature_path,
    InMemoryUserService{}
);

{
    auto db_life_cycle = database::link<5432, "root", "123456">(global_db_handle);
    const auto database = jh::test::cucumber::run_file<DatabaseDefinition>(
        feature_path,
        DatabaseUserService{global_db_handle}
    );
}
```

All three calls use the same feature path. `run<Definition>(feature, args...)`
accepts the same constructor arguments for an already parsed feature. Each
definition has its own binding set, so the adapters can use the same Gherkin
expressions. This supports port-and-adapter designs.

### Fixture and step lifetimes

`run_file` parses the feature and runs it synchronously. `run` executes a
feature supplied by the caller. Both pass their constructor arguments as
lvalues to a new step object for each scenario. The step object lives through
that scenario's steps and is destroyed at the end of the scenario scope.
Its members hold scenario-local state. Each expanded `Scenario Outline`
example receives the same per-scenario lifetime.
The parsed feature inside `run_file` stays alive through execution, so
`<string>` captures can borrow its step text during the call. A step can copy
text it needs to retain after the call.

Constructor arguments live through the synchronous call. A temporary adapter
such as `InMemoryUserService{}` lives through the call expression; the runner
passes it to each scenario's step constructor. A step class can hold a
reference to the same run-scoped service across scenarios, so service state
can persist while step-object state starts fresh. The application owns an
injected lvalue such as `global_db_handle` and controls its lifetime.

RAII owners acquire resources before execution and release them when their
scope ends. In the database example, `database::link` brackets the call, and
`DatabaseUserService` holds a reference to the configured handle. The TDD
verifies a service constructor and destructor bracketing a run.

The runner creates a `StepContext` for each step. A `Then` binding can call
`context.expect` to record an assertion; the runner copies its messages into
the scenario result and ends that scenario's step sequence. A step callable's
exception propagates to the caller while C++ stack unwinding destroys the
scenario object and run-scoped temporaries. A rejection scenario can catch
its expected exception in a `When` binding, store the outcome in the step
object, and check it in a later `Then` binding.

`run<Definition>(feature)` and `run_file<Definition>(path)` return
`jh::meta::expected<RunResult, run_error>`. Parse errors, undefined steps,
assertion failures, and scalar conversion failures appear in `RunResult`;
`passed()` summarizes the outcome and `format_report()` formats it. Ambiguous
bindings return `jh::meta::unexpected(run_error::ambiguous_step)`. `RunResult`
owns the recorded names and failure messages; the caller retains ownership
of injected lvalue fixtures.

The scalar helpers `asString`, `asInt`, `asUint`, `asDouble`, and `asBool` are also available, along with the typed `as<T>` helper. Conversions that can fail return `jh::meta::expected`. `asString` makes an owned copy and is useful for values such as DataTable cells that must persist beyond their source view.

`format_report(const RunResult&)` returns a report string for the caller to
display or log. It includes feature and file names, a `PASS` or `FAIL` line
for each scenario, and step locations and messages for failures. The top-level
`RunResult::failures` holds feature and parse failures; each scenario holds
its own step failures. An ambiguous binding returns `run_error` through the
outer `expected`.

You can build another format from the public result fields and then print it, send it to a logger, or convert it to a structured report:

```cpp
#include <sstream>
#include <string>

std::string format_custom_report(const jh::test::cucumber::RunResult& result) {
    std::ostringstream output;
    output << (result.passed() ? "PASS " : "FAIL ") << result.feature_name << '\n';

    const auto write_failure = [&output](const jh::test::cucumber::StepFailure& failure) {
        output << "  ";
        if (failure.location.line != 0) {
            output << failure.location.line << ':' << failure.location.column << ' ';
        }
        output << failure.step_text << '\n';
        for (const auto& message : failure.messages) {
            output << "    " << message << '\n';
        }
    };

    for (const auto& failure : result.failures) write_failure(failure);
    for (const auto& scenario : result.scenarios) {
        output << (scenario.passed() ? "PASS " : "FAIL ") << scenario.name << '\n';
        for (const auto& failure : scenario.failures) write_failure(failure);
    }
    return output.str();
}
```

The caller decides where the returned string goes:

```cpp
#include <iostream>

if (result) {
    std::cout << format_custom_report(*result);
}
```

### Example

The following step definitions double an input and check the result. The feature path macro is set by the test target's CMake configuration, as shown in the project structure section.

```cpp
#include <filesystem>
#include <iostream>

#include "jh/test/cucumber/cucumber.hpp"

namespace test::cucumber {
    using namespace jh::test::cucumber;
}

namespace test {
    class CalculatorSteps {
        double input_{};
        double result_{};

    public:
        void input(const double value) { input_ = value; }
        void double_input() { result_ = input_ * 2; }
        void expect_result(const double expected, cucumber::StepContext& context) const {
            context.expect(result_ == expected, "calculated result differs");
        }
    };

    using CalculatorDefinition = cucumber::StepDefinition<
        CalculatorSteps,
        cucumber::Given<"an input <double>", &CalculatorSteps::input>,
        cucumber::When<"the input is doubled", &CalculatorSteps::double_input>,
        cucumber::Then<"the result is <double>", &CalculatorSteps::expect_result>
    >;
}

int main() {
    const auto result = jh::test::cucumber::run_file<test::CalculatorDefinition>(
        std::filesystem::path{BDD_FEATURE_DIR} / "calculator.feature"
    );
    if (!result) {
        std::cerr << "Ambiguous step definitions.\n";
        return 1;
    }
    if (!result->passed()) {
        std::cerr << jh::test::cucumber::format_report(*result);
        return 1;
    }
}
```

Feature file `tests/bdd/features/calculator.feature`:

```gherkin
Feature: Calculator

  Scenario: Double an input
    Given an input 21
    When the input is doubled
    Then the result is 42
```

### Project structure

Keep Gherkin files and C++ step bindings under the test area, grouped by the behavior or subsystem they exercise. For example:

```text
tests/
└── bdd/
    ├── CMakeLists.txt
    ├── features/
    │   ├── calculator.feature
    │   └── storage.feature
    ├── steps/
    │   ├── calculator_steps.hpp
    │   └── storage_steps.hpp
    └── runners/
        ├── calculator_main.cpp
        └── storage_main.cpp
```

Each runner can define its own `StepDefinition` by composing bindings from its subsystem's step classes. Link each BDD executable to `jh::test::jh-simple-cucumber`, register it with CTest, and use source-tree feature paths for consistent execution from any working directory. For example, in `tests/bdd/CMakeLists.txt`:

```cmake
target_link_libraries(calculator_bdd PRIVATE jh::test::jh-simple-cucumber)
target_compile_definitions(calculator_bdd PRIVATE
    "BDD_FEATURE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/features\"")
add_test(NAME calculator_bdd COMMAND calculator_bdd)
```

## BDD and Gherkin

Behavior-driven development (BDD) describes expected behavior through concrete examples that developers, testers, and product owners can discuss together. Gherkin expresses those examples in readable feature files. A `Feature` groups behavior; each `Scenario` gives an example using `Given` for initial context, `When` for an action, and `Then` for an observable result. `And` continues the preceding step kind. `Scenario Outline` and `Examples` let the parser expand one scenario template into multiple cases.

`jh-simple-cucumber` uses the official Cucumber Gherkin C++ parser to parse feature files and expand them into steps. The library binds those parsed steps to typed C++ functions, runs each scenario with fresh step state, and collects assertion or conversion failures into a report.

## License

JH Simple Cucumber is distributed under the Apache License 2.0, the same license as JH-Toolkit. See the [repository license](../LICENSE). Third-party dependencies retain their respective licenses.
