<h1 align="center">
  <span style="font-size: x-large;">JH Tiny Test</span>
</h1>

<font size="4"><code>jh-tiny-test</code></font> is a small C++20 runtime test library in the `jh::test::tiny_test` namespace. Define application tests under `test::tiny_test` and import the library API there. Test and session identities use `jh::meta::TStr`. Defining the same explicit specialization twice in one translation unit is a compile error. At runtime, names registered more than once in the same executable are ambiguous and make the global runner fail before running tests. Separate executable entry points have independent registries and may reuse names. Duplicate test names within a session and duplicate names in a suite are rejected by `static_assert`.

## Installation

Build and install the package from this checkout:

```sh
cmake -S tiny-test -B build/tiny-test \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$HOME/.local/jh-tiny-test"
cmake --install build/tiny-test
```

The package depends on JH-Toolkit. Install it first, and make both prefixes discoverable through `CMAKE_PREFIX_PATH`:

```cmake
find_package(jh-toolkit CONFIG REQUIRED)
find_package(jh-tiny-test CONFIG REQUIRED)
target_link_libraries(my_tests PRIVATE jh::test::jh-tiny-test)
```

When using FetchContent, set `SOURCE_SUBDIR tiny-test`. Both this package and `jh-simple-cucumber` export their CMake targets under `jh::test::`.

## Usage

Keep test bodies in the application namespace. Add only the name-to-body specializations at the library's injection point:

```cpp
#define TINY_TEST_MAIN
#include <stdexcept>

#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

namespace test {
    void addition() {
        tiny_test::expect(1 + 1 == 2, "addition should produce two");
    }

    void domain_error() {
        tiny_test::expect_throw<std::domain_error>([] {
            throw std::domain_error("invalid denominator");
        });
    }
}

template<>
struct jh::test::tiny_test::test<"addition">
    : jh::test::tiny_test::test_definition<"addition", &::test::addition> {};

template<>
struct jh::test::tiny_test::test<"domain error">
    : jh::test::tiny_test::test_definition<"domain error", &::test::domain_error> {};

template<>
struct jh::test::tiny_test::session<"arithmetic">
    : jh::test::tiny_test::session_definition<
          "arithmetic",
          jh::test::tiny_test::test<"addition">,
          jh::test::tiny_test::test<"domain error">
      > {};

namespace test {
    [[maybe_unused]] const tiny_test::session<"arithmetic"> registered_session{};
}
```

`expect`, `expect_not`, and `expect_throw` return no value. They record observations in the active test result, allowing later assertions to run and collect their own messages. `expect_throw` defaults to `jh::typed::monostate` and accepts any thrown exception; use `expect_throw<SomeException>(...)` to require a particular type. A missing or non-matching exception is recorded as a failure. Exceptions that escape the test body are caught at the test boundary and stored in the result. Process-level faults such as segmentation faults are outside C++ exception handling.

`tiny_main` accepts `--list`, a session name, or a test name. With no selection, it runs the full suite. `format_report` builds report text without printing it.

For static registration across translation units, create one session object in each translation unit that owns a session, using a unique session and test name across the executable. Each object registers its explicit session specialization with the global runner. Define `TINY_TEST_MAIN` before including the header in exactly one translation unit to generate `main`, which calls the global `tiny_main(argc, argv)` overload. The runner does not inspect application namespaces; registrations come from the explicit session objects.

## Self-checks

Build the behavior suite and the positive compile-time case, then run the CTest checks:

```sh
cmake -S tiny-test -B build/tiny-test -DCMAKE_BUILD_TYPE=Debug
cmake --build build/tiny-test
ctest --test-dir build/tiny-test --output-on-failure
```

The `static_tests/fail` cases verify that duplicate test/session specializations and duplicate names in a session or suite are rejected by the compiler. Separate multi-translation-unit executables verify that repeated test names or session names in one executable fail registration, while two independent executables can use the same names successfully.
