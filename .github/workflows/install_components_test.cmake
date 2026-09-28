if (NOT DEFINED JH_SOURCE_DIR OR NOT DEFINED JH_TEST_ROOT)
    message(FATAL_ERROR "JH_SOURCE_DIR and JH_TEST_ROOT must be provided")
endif ()

get_filename_component(JH_SOURCE_DIR "${JH_SOURCE_DIR}" REALPATH)
get_filename_component(JH_TEST_ROOT "${JH_TEST_ROOT}" ABSOLUTE)
file(REMOVE_RECURSE "${JH_TEST_ROOT}")
file(MAKE_DIRECTORY "${JH_TEST_ROOT}")

set(_jh_prefix "${JH_TEST_ROOT}/prefix")
set(_jh_generator "Ninja")
set(_jh_libdir "lib")

function(_jh_run label)
    execute_process(
        COMMAND ${ARGN}
        RESULT_VARIABLE _jh_result
        OUTPUT_VARIABLE _jh_stdout
        ERROR_VARIABLE _jh_stderr)
    if (NOT "${_jh_result}" STREQUAL "0")
        message(FATAL_ERROR
            "${label} failed (${_jh_result})\nstdout:\n${_jh_stdout}\nstderr:\n${_jh_stderr}")
    endif ()
endfunction()

function(_jh_install profile build_name clean)
    set(_jh_build_dir "${JH_TEST_ROOT}/${build_name}")
    _jh_run("${build_name} configure"
        "${CMAKE_COMMAND}" -S "${JH_SOURCE_DIR}" -B "${_jh_build_dir}"
        -G "${_jh_generator}"
        -DCMAKE_BUILD_TYPE=MinSizeRel
        -DCMAKE_INSTALL_INCLUDEDIR=include
        -DCMAKE_INSTALL_LIBDIR=lib
        -DCMAKE_INSTALL_PREFIX=${_jh_prefix}
        -DJH_TOOLKIT_COMPONENTS=${profile}
        -DJH_TOOLKIT_CLEAN_INSTALL=${clean}
        -DJH_ENABLE_TESTING=OFF
        -DJH_ENABLE_EXAMPLES=OFF)
    _jh_run("${build_name} build" "${CMAKE_COMMAND}" --build "${_jh_build_dir}" --config Release)
    _jh_run("${build_name} install" "${CMAKE_COMMAND}" --install "${_jh_build_dir}" --config Release)
    _jh_check_manifest()
endfunction()

function(_jh_check_manifest)
    set(_jh_manifest
        "${_jh_prefix}/${_jh_libdir}/cmake/jh-toolkit/jh-toolkit-installed-files.cmake")
    if (NOT EXISTS "${_jh_manifest}")
        message(FATAL_ERROR "Install did not write its actual-file manifest: ${_jh_manifest}")
    endif ()
    unset(JH_TOOLKIT_INSTALLED_FILES)
    include("${_jh_manifest}")
    foreach (_jh_file IN LISTS JH_TOOLKIT_INSTALLED_FILES)
        if (NOT EXISTS "${_jh_prefix}/${_jh_file}")
            message(FATAL_ERROR "Manifest entry is missing on disk: ${_jh_file}")
        endif ()
        if (NOT _jh_file MATCHES "^include/jh/"
                AND NOT _jh_file MATCHES "^${_jh_libdir}/cmake/jh-toolkit/"
                AND NOT _jh_file MATCHES "^${_jh_libdir}/libjh-toolkit-static")
            message(FATAL_ERROR "Manifest contains a path outside the toolkit install: ${_jh_file}")
        endif ()
    endforeach ()
endfunction()

function(_jh_write_consumer name required expected_available expected_missing expect_ipcs)
    set(_jh_dir "${JH_TEST_ROOT}/consumer-${name}")
    file(MAKE_DIRECTORY "${_jh_dir}")

    file(GLOB_RECURSE _jh_installed_headers
        LIST_DIRECTORIES FALSE
        "${_jh_prefix}/include/jh/*")
    list(SORT _jh_installed_headers)
    set(_jh_include_lines "")
    foreach (_jh_header IN LISTS _jh_installed_headers)
        get_filename_component(_jh_header_name "${_jh_header}" NAME)
        if (_jh_header_name MATCHES "\\." OR _jh_header_name STREQUAL ".DS_Store")
            continue()
        endif ()
        file(RELATIVE_PATH _jh_relative_header "${_jh_prefix}/include" "${_jh_header}")
        string(APPEND _jh_include_lines "#include <${_jh_relative_header}>\n")
    endforeach ()
    string(APPEND _jh_include_lines
        "#include <jh/metax/expected.h>\n"
        "#include <jh/metax/t_str.h>\n"
        "#include <jh/typing/monostate.h>\n")

    file(WRITE "${_jh_dir}/consumer.cpp"
        "${_jh_include_lines}\n"
        "#if JH_EXPECT_IPCS\n"
        "#ifndef JH_TOOLKIT_ENABLE_IPCS\n"
        "#error IPC install did not enable the IPC aggregate\n"
        "#endif\n"
        "#else\n"
        "#ifdef JH_TOOLKIT_ENABLE_IPCS\n"
        "#error IPC support leaked into an install without IPC headers\n"
        "#endif\n"
        "#endif\n"
        "int main() { return 0; }\n")

    file(WRITE "${_jh_dir}/CMakeLists.txt"
        "cmake_minimum_required(VERSION 3.21)\n"
        "project(jh_toolkit_install_consumer LANGUAGES CXX)\n"
        "find_package(jh-toolkit CONFIG REQUIRED COMPONENTS \${JH_REQUIRED_COMPONENTS})\n"
        "get_property(_available TARGET jh::jh-toolkit PROPERTY JH_TOOLKIT_AVAILABLE_COMPONENTS)\n"
        "set(_expected_available ${expected_available})\n"
        "foreach(_component IN LISTS _expected_available)\n"
        "  if(NOT _component IN_LIST _available)\n"
        "    message(FATAL_ERROR \"Installed component capability missing: \${_component}; available: \${_available}\")\n"
        "  endif()\n"
        "endforeach()\n"
        "set(_expected_missing ${expected_missing})\n"
        "foreach(_component IN LISTS _expected_missing)\n"
        "  if(_component IN_LIST _available)\n"
        "    message(FATAL_ERROR \"Removed component still reported available: \${_component}\")\n"
        "  endif()\n"
        "endforeach()\n"
        "add_executable(consumer consumer.cpp)\n"
        "target_link_libraries(consumer PRIVATE jh::jh-toolkit)\n"
        "target_compile_definitions(consumer PRIVATE JH_EXPECT_IPCS=${expect_ipcs})\n")

    set(_jh_build_dir "${_jh_dir}/build")
    _jh_run("${name} consumer configure"
        "${CMAKE_COMMAND}" -S "${_jh_dir}" -B "${_jh_build_dir}"
        -G "${_jh_generator}"
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_PREFIX_PATH=${_jh_prefix}
        "-DJH_REQUIRED_COMPONENTS=${required}")
    _jh_run("${name} consumer build"
        "${CMAKE_COMMAND}" --build "${_jh_build_dir}" --config Release)
endfunction()

function(_jh_expect_component_rejected component)
    set(_jh_dir "${JH_TEST_ROOT}/consumer-rejected-${component}")
    file(MAKE_DIRECTORY "${_jh_dir}")
    file(WRITE "${_jh_dir}/CMakeLists.txt"
        "cmake_minimum_required(VERSION 3.21)\n"
        "project(jh_toolkit_rejected_component LANGUAGES CXX)\n"
        "find_package(jh-toolkit CONFIG REQUIRED COMPONENTS ${component})\n")
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -S "${_jh_dir}" -B "${_jh_dir}/build"
            -G "${_jh_generator}" -DCMAKE_PREFIX_PATH=${_jh_prefix}
        RESULT_VARIABLE _jh_result
        OUTPUT_VARIABLE _jh_stdout
        ERROR_VARIABLE _jh_stderr)
    if ("${_jh_result}" STREQUAL "0")
        message(FATAL_ERROR "find_package unexpectedly accepted removed component '${component}'")
    endif ()
    string(FIND "${_jh_stdout}${_jh_stderr}" "jh-toolkit-config.cmake" _jh_config_found)
    string(FIND "${_jh_stdout}${_jh_stderr}" "jh-toolkit_FOUND" _jh_package_rejected)
    if (_jh_config_found EQUAL -1 OR _jh_package_rejected EQUAL -1)
        message(FATAL_ERROR
            "find_package failed for an unrelated reason while checking '${component}'\n"
            "stdout:\n${_jh_stdout}\nstderr:\n${_jh_stderr}")
    endif ()
endfunction()

function(_jh_install_local_test_package name source_dir parser_fetch)
    set(_jh_build_dir "${JH_TEST_ROOT}/build-${name}-package")
    _jh_run("${name} local-source configure"
        "${CMAKE_COMMAND}" -S "${source_dir}" -B "${_jh_build_dir}"
        -G "${_jh_generator}"
        -DCMAKE_BUILD_TYPE=MinSizeRel
        -DCMAKE_INSTALL_PREFIX=${_jh_prefix}
        -DCMAKE_PREFIX_PATH=${_jh_prefix}
        -DCMAKE_FIND_ROOT_PATH=${_jh_prefix}
        -DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ONLY
        -DCMAKE_FIND_USE_CMAKE_SYSTEM_PATH=OFF
        -DCMAKE_NO_SYSTEM_FROM_IMPORTED=ON
        -DCMAKE_FIND_USE_PACKAGE_REGISTRY=OFF
        -DCMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY=OFF
        -DJH_TINY_TEST_BUILD_TDD=OFF
        -DJH_TINY_TEST_BUILD_STATIC_TESTS=OFF
        -DJH_SIMPLE_CUCUMBER_BUILD_TDD=OFF
        -DJH_SIMPLE_CUCUMBER_BUILD_STATIC_TESTS=OFF
        -DJH_SIMPLE_CUCUMBER_FETCH_PARSER_DEPS=${parser_fetch})
    _jh_run("${name} local-source build"
        "${CMAKE_COMMAND}" --build "${_jh_build_dir}" --config MinSizeRel)
    _jh_run("${name} local-source install"
        "${CMAKE_COMMAND}" --install "${_jh_build_dir}" --config MinSizeRel)
endfunction()

function(_jh_build_package_consumer name)
    set(_jh_dir "${JH_TEST_ROOT}/consumer-${name}")
    set(_jh_build_dir "${_jh_dir}/build")
    _jh_run("${name} consumer configure"
        "${CMAKE_COMMAND}" -S "${_jh_dir}" -B "${_jh_build_dir}"
        -G "${_jh_generator}"
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_PREFIX_PATH=${_jh_prefix}
        -DCMAKE_FIND_ROOT_PATH=${_jh_prefix}
        -DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ONLY
        -DCMAKE_FIND_USE_CMAKE_SYSTEM_PATH=OFF
        -DCMAKE_NO_SYSTEM_FROM_IMPORTED=ON
        -DCMAKE_FIND_USE_PACKAGE_REGISTRY=OFF
        -DCMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY=OFF)
    _jh_run("${name} consumer build"
        "${CMAKE_COMMAND}" --build "${_jh_build_dir}" --config Release)
    _jh_run("${name} consumer run"
        "${CMAKE_CTEST_COMMAND}" --test-dir "${_jh_build_dir}" --output-on-failure)
endfunction()

function(_jh_write_fetchcontent_consumer name source_subdir target_name source_text)
    set(_jh_dir "${JH_TEST_ROOT}/fetchcontent-${name}")
    file(MAKE_DIRECTORY "${_jh_dir}")
    file(WRITE "${_jh_dir}/CMakeLists.txt"
        "cmake_minimum_required(VERSION 3.21)\n"
        "project(jh_fetchcontent_${name} LANGUAGES CXX)\n"
        "include(FetchContent)\n"
        "set(FETCHCONTENT_QUIET OFF)\n"
        "FetchContent_Declare(${name}\n"
        "  GIT_REPOSITORY \"${_jh_fetch_repository}\"\n"
        "  GIT_TAG \"${_jh_fetch_ref}\"\n"
        "  GIT_PROGRESS TRUE\n"
        "  SOURCE_SUBDIR ${source_subdir}\n"
        ")\n"
        "set(JH_TINY_TEST_BUILD_TDD OFF CACHE BOOL \"\" FORCE)\n"
        "set(JH_TINY_TEST_BUILD_STATIC_TESTS OFF CACHE BOOL \"\" FORCE)\n"
        "set(JH_SIMPLE_CUCUMBER_BUILD_TDD OFF CACHE BOOL \"\" FORCE)\n"
        "set(JH_SIMPLE_CUCUMBER_BUILD_STATIC_TESTS OFF CACHE BOOL \"\" FORCE)\n"
        "set(JH_SIMPLE_CUCUMBER_FETCH_PARSER_DEPS ON CACHE BOOL \"\" FORCE)\n"
        "FetchContent_MakeAvailable(${name})\n"
        "include(CTest)\n"
        "add_executable(package_consumer main.cpp)\n"
        "target_link_libraries(package_consumer PRIVATE ${target_name})\n"
        "add_test(NAME ${name}_consumer_smoke COMMAND package_consumer)\n")
    file(WRITE "${_jh_dir}/main.cpp" "${source_text}")

    set(_jh_build_dir "${_jh_dir}/build")
    _jh_run("${name} FetchContent configure"
        "${CMAKE_COMMAND}" -S "${_jh_dir}" -B "${_jh_build_dir}"
        -G "${_jh_generator}"
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_FIND_ROOT_PATH=${_jh_dir}/package-search-root
        -DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ONLY
        -DCMAKE_FIND_USE_CMAKE_SYSTEM_PATH=OFF
        -DCMAKE_FIND_USE_PACKAGE_REGISTRY=OFF
        -DCMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY=OFF)
    _jh_run("${name} FetchContent build"
        "${CMAKE_COMMAND}" --build "${_jh_build_dir}" --config Release)
    _jh_run("${name} FetchContent run"
        "${CMAKE_CTEST_COMMAND}" --test-dir "${_jh_build_dir}" --output-on-failure)
endfunction()

set(_jh_primary_prefix "${_jh_prefix}")
set(_jh_prefix "${JH_TEST_ROOT}/test-only-prefix")
_jh_install("jh-test-enabled;jh-test-enabled" "install-test-only" OFF)
_jh_write_consumer("test-only" "jh-test-enabled" "jh-test-enabled"
    "base;jh-no-throw;jh-ipcs;all" 0)

set(_jh_prefix "${_jh_primary_prefix}")
_jh_install("jh-no-throw" "install-no-throw" OFF)
_jh_write_consumer("no-throw" "jh-no-throw"
    "jh-no-throw;jh-test-enabled" "base;jh-ipcs;all" 0)

_jh_install("jh-test-enabled,jh-ipcs,jh-test-enabled" "install-test-and-ipc" OFF)
_jh_write_consumer("merged-components" "jh-no-throw;jh-test-enabled;jh-ipcs"
    "jh-no-throw;jh-test-enabled;jh-ipcs" "base;all" 1)

_jh_install("base" "install-base-additive" OFF)
_jh_write_consumer("union-all" "all"
    "base;jh-no-throw;jh-test-enabled;jh-ipcs;all" "" 1)

_jh_install("base" "install-clean-base" ON)
set(_jh_remaining_ipc_headers "")
foreach (_jh_ipc_header IN ITEMS jh/ipc jh/synchronous/ipc.h)
    if (EXISTS "${_jh_prefix}/include/${_jh_ipc_header}")
        list(APPEND _jh_remaining_ipc_headers "include/${_jh_ipc_header}")
    endif ()
endforeach ()
if (_jh_remaining_ipc_headers)
    message(FATAL_ERROR
        "Explicit clean install left IPC headers in the prefix: ${_jh_remaining_ipc_headers}")
endif ()
_jh_write_consumer("clean-base" "base"
    "base;jh-no-throw;jh-test-enabled" "jh-ipcs;all" 0)
_jh_expect_component_rejected("jh-ipcs")

set(_jh_prefix "${JH_TEST_ROOT}/framework-package-prefix")
_jh_install("jh-test-enabled" "install-framework-toolkit" ON)

set(_jh_local_sources "${JH_TEST_ROOT}/local-package-sources")
file(MAKE_DIRECTORY "${_jh_local_sources}/tiny-test" "${_jh_local_sources}/simple-cucumber")
file(COPY "${JH_SOURCE_DIR}/tiny-test/" DESTINATION "${_jh_local_sources}/tiny-test")
file(COPY "${JH_SOURCE_DIR}/simple-cucumber/" DESTINATION "${_jh_local_sources}/simple-cucumber")

_jh_install_local_test_package("tiny-test" "${_jh_local_sources}/tiny-test" OFF)
set(_jh_tiny_consumer "${JH_TEST_ROOT}/consumer-tiny-test")
file(MAKE_DIRECTORY "${_jh_tiny_consumer}")
file(WRITE "${_jh_tiny_consumer}/CMakeLists.txt"
    "cmake_minimum_required(VERSION 3.21)\n"
    "project(jh_tiny_test_installed_consumer LANGUAGES CXX)\n"
    "include(CTest)\n"
    "find_package(jh-tiny-test CONFIG REQUIRED)\n"
    "add_executable(package_consumer main.cpp)\n"
    "target_link_libraries(package_consumer PRIVATE jh::test::jh-tiny-test)\n"
    "add_test(NAME tiny_test_installed_consumer COMMAND package_consumer)\n")
file(WRITE "${_jh_tiny_consumer}/main.cpp" [=[
#include <jh/test/tiny_test/tiny_test.hpp>

namespace install_smoke {
void works() { jh::test::tiny_test::expect(2 + 2 == 4, "tiny-test package works"); }
}

template<>
struct jh::test::tiny_test::test<"installed package">
    : jh::test::tiny_test::test_definition<"installed package", &install_smoke::works> {};

template<>
struct jh::test::tiny_test::session<"installed package smoke">
    : jh::test::tiny_test::session_definition<"installed package smoke",
          jh::test::tiny_test::test<"installed package">> {};

int main() {
    const auto result = jh::test::tiny_test::session<"installed package smoke">::run();
    return result.tests.size() == 1 && result.tests.front().passed() ? 0 : 1;
}
]=])
_jh_build_package_consumer("tiny-test")

_jh_install_local_test_package("simple-cucumber" "${_jh_local_sources}/simple-cucumber" ON)
set(_jh_cucumber_consumer "${JH_TEST_ROOT}/consumer-simple-cucumber")
file(MAKE_DIRECTORY "${_jh_cucumber_consumer}")
file(WRITE "${_jh_cucumber_consumer}/CMakeLists.txt"
    "cmake_minimum_required(VERSION 3.21)\n"
    "project(jh_simple_cucumber_installed_consumer LANGUAGES CXX)\n"
    "include(CTest)\n"
    "find_package(jh-simple-cucumber CONFIG REQUIRED)\n"
    "add_executable(package_consumer main.cpp)\n"
    "target_link_libraries(package_consumer PRIVATE jh::test::jh-simple-cucumber)\n"
    "add_test(NAME simple_cucumber_installed_consumer COMMAND package_consumer)\n")
file(WRITE "${_jh_cucumber_consumer}/main.cpp" [=[
#include <jh/test/cucumber/cucumber.hpp>

namespace cucumber = jh::test::cucumber;

class PackageSteps {
public:
    void available(cucumber::StepContext& context) {
        context.expect(true, "simple-cucumber package works");
    }
};

using PackageDefinition = cucumber::StepDefinition<PackageSteps,
    cucumber::Given<"the installed parser runs", &PackageSteps::available>>;

int main() {
    const auto feature = cucumber::parse_feature("package.feature",
        "Feature: installed package\n"
        "  Scenario: parse and execute\n"
        "    Given the installed parser runs\n");
    if (!feature) return 1;
    const auto result = cucumber::run<PackageDefinition>(feature.value());
    return result.has_value() && result->passed() ? 0 : 1;
}
]=])
_jh_build_package_consumer("simple-cucumber")

set(_jh_fetch_repository "$ENV{JH_FETCH_REPOSITORY}")
set(_jh_fetch_ref "$ENV{JH_FETCH_REF}")
if (_jh_fetch_repository AND _jh_fetch_ref)
    set(_jh_tiny_fetch_source [=[
#include <jh/test/tiny_test/tiny_test.hpp>

namespace fetch_smoke {
void works() { jh::test::tiny_test::expect(true, "FetchContent tiny-test works"); }
}

template<>
struct jh::test::tiny_test::test<"FetchContent smoke">
    : jh::test::tiny_test::test_definition<"FetchContent smoke", &fetch_smoke::works> {};

template<>
struct jh::test::tiny_test::session<"FetchContent session">
    : jh::test::tiny_test::session_definition<"FetchContent session",
          jh::test::tiny_test::test<"FetchContent smoke">> {};

int main() {
    const auto result = jh::test::tiny_test::session<"FetchContent session">::run();
    return result.tests.size() == 1 && result.tests.front().passed() ? 0 : 1;
}
]=])
    _jh_write_fetchcontent_consumer(jh_tiny_test "tiny-test"
        "jh::test::jh-tiny-test" "${_jh_tiny_fetch_source}")

    set(_jh_cucumber_fetch_source [=[
#include <jh/test/cucumber/cucumber.hpp>

namespace cucumber = jh::test::cucumber;

class FetchSteps {
public:
    void available(cucumber::StepContext& context) {
        context.expect(true, "FetchContent simple-cucumber works");
    }
};

using FetchDefinition = cucumber::StepDefinition<FetchSteps,
    cucumber::Given<"the fetched parser runs", &FetchSteps::available>>;

int main() {
    const auto feature = cucumber::parse_feature("fetch.feature",
        "Feature: fetched package\n"
        "  Scenario: parse and execute\n"
        "    Given the fetched parser runs\n");
    if (!feature) return 1;
    const auto result = cucumber::run<FetchDefinition>(feature.value());
    return result.has_value() && result->passed() ? 0 : 1;
}
]=])
    _jh_write_fetchcontent_consumer(jh_simple_cucumber "simple-cucumber"
        "jh::test::jh-simple-cucumber" "${_jh_cucumber_fetch_source}")
else ()
    message(STATUS "Skipping current-branch FetchContent checks without JH_FETCH_REPOSITORY and JH_FETCH_REF")
endif ()

message(STATUS "Install and package consumer integration passed")
