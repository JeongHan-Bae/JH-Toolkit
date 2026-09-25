<h1 align="center">
  <span style="font-size: x-large;">&nbsp;</span>
  <span>
    <img src="https://raw.githubusercontent.com/cucumber/cucumber-js/46a5a78107be27e99c6e044c69b6e8f885ce456c/docs/images/logo.svg"
         alt="Cucumber"
         width="68" valign="middle">
  </span>
  <span style="font-size: x-large;">&nbsp;JH Simple Cucumber</span>
</h1>

<font size="4"><code>jh-simple-cucumber</code></font> is a small C++20 test library for typed Cucumber step definitions and Gherkin feature files. Its C++ API lives in `jh::test::cucumber`; the Gherkin C++ parser is fetched from the official Cucumber Gherkin project.

## Installation

`jh-simple-cucumber` depends on the JH-Toolkit headers and `jh::meta::expected`. From a JH-Toolkit checkout, configure and install it as a standalone CMake package:

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
  -DJH_SIMPLE_CUCUMBER_BUILD_TDD=ON
cmake --build build/simple-cucumber --target jh_simple_cucumber_tdd
ctest --test-dir build/simple-cucumber --output-on-failure
```

Passing scenarios and their step definitions live in `tdd/pass/`; expected failures are in `tdd/fail/`. The suite covers scalar conversion, type-aware candidate selection, exponent-form doubles, constructor-injected services, `And`, Scenario Outlines, per-scenario state, DataTables, undefined steps, conversion failures, and ambiguous definitions. `JH_SIMPLE_CUCUMBER_BUILD_TDD` defaults on for standalone builds and when JH-Toolkit tests are enabled.

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

The optional trailing parameters are `const DataTable&` and `StepContext&`, in that order. Bindings support const and non-const, static and non-static, `constexpr`, `noexcept`, and `noexcept(false)` functions. `<string>` refers to the current step text without copying; copy it into `std::string` if it must outlive the feature run.

Within one `StepDefinition`, the same `StepKind` cannot bind the same expression twice; that duplicate is rejected at compile time. A DataTable binding must take `const DataTable&` (optionally followed by `StepContext&`). If a parsed step has a table but its binding does not accept one, or its binding requires a table that the step lacks, execution records a step failure in `RunResult` and does not call the bound function. A mismatch across separate `StepDefinition`s is independent; each definition is checked only against the feature run that uses it.

Without constructor arguments, `run<Definition>(feature)` and `run_file<Definition>(path)` default-construct the step object. Here is a basic definition for the user feature:

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

All three calls use `run_file` with the same feature path; each definition can run that feature independently without switching APIs. `run<Definition>(feature, args...)` accepts the same constructor arguments when the feature has already been parsed. The two adapter-specific definitions have independent binding sets, so they can use the same Gherkin expressions without conflicting. This supports port-and-adapter designs, including hexagonal architecture. The runner creates a fresh step object for each scenario and supplies the run-scoped arguments each time; the arguments are used only during the synchronous run and are not retained in `RunResult`.

The application configures its shared `database::handle` before the run and injects it by reference through `DatabaseUserService`. Resource setup and cleanup belong to the handle or its owner; no Cucumber before/after hooks are required. The TDD uses a small service with constructor/destructor log entries to verify that a run-scoped dependency can bracket feature execution.

Run a parsed feature or a feature file with `run<Definition>(feature)` or `run_file<Definition>(path)`. Both return `jh::meta::expected<RunResult, run_error>`. Parse errors, undefined steps, assertion failures, and scalar conversion failures are reported inside `RunResult`; check `passed()` and use `format_report()` to print details. If multiple definitions accept the same step, the runner returns `jh::meta::unexpected(run_error::ambiguous_step)` because it cannot select a single binding.

The scalar helpers `asString`, `asInt`, `asUint`, `asDouble`, and `asBool` are also available, along with the typed `as<T>` helper. Conversions that can fail return `jh::meta::expected`. `asString` makes an owned copy and is useful for values such as DataTable cells that must persist beyond their source view.

`format_report(const RunResult&)` returns a report string; it does not print to a stream. For a passing run it includes the feature and file names followed by a `PASS` line for each scenario. For a failing run it prints `FAIL` for failed scenarios and includes step locations, step text, and failure messages. The top-level `RunResult::failures` contains feature or parse failures, while each scenario's `failures` contains step failures. If the outer `expected` has no value because of an ambiguous definition, handle `run_error` separately because there is no `RunResult` to format.

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

Each runner can define its own `StepDefinition` by composing bindings from its subsystem's step classes. Link each BDD executable to `jh::test::jh-simple-cucumber`, register it with CTest, and pass feature paths from the source tree so tests do not depend on the process working directory. For example, in `tests/bdd/CMakeLists.txt`:

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
