cmake_minimum_required(VERSION 3.21)
project(jh_tiny_test_static_failure LANGUAGES CXX)

set(JH_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(JH_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(USE_JH_COMPILE_OPT OFF CACHE BOOL "" FORCE)
set(JH_TINY_TEST_BUILD_TDD OFF CACHE BOOL "" FORCE)
set(JH_TINY_TEST_BUILD_STATIC_TESTS OFF CACHE BOOL "" FORCE)
set(JH_TINY_TEST_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory("${STATIC_TOOLKIT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/jh-toolkit")
add_subdirectory("${STATIC_TINY_TEST_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/tiny-test")

include("${STATIC_FAIL_CASES_FILE}")

foreach(case_file IN LISTS STATIC_FAIL_CASE_FILES)
    include("${case_file}")
    set(target "tiny_test_static_fail_translation_unit_${STATIC_TEST_NAME}")
    add_library("${target}" OBJECT "${STATIC_TEST_SOURCE}")
    target_link_libraries("${target}" PRIVATE jh::test::jh-tiny-test)
    target_compile_features("${target}" PRIVATE cxx_std_20)
endforeach()
