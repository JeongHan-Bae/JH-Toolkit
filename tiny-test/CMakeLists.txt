cmake_minimum_required(VERSION 3.21)
project(jh-tiny-test VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)

if(PROJECT_IS_TOP_LEVEL)
    include(CTest)
    set(_jh_tiny_test_tdd_default "${BUILD_TESTING}")
elseif(JH_ENABLE_TESTING)
    set(_jh_tiny_test_tdd_default ON)
else()
    set(_jh_tiny_test_tdd_default OFF)
endif()
option(JH_TINY_TEST_BUILD_TDD
    "Build the tiny-test runtime behavior tests"
    "${_jh_tiny_test_tdd_default}"
)
option(JH_TINY_TEST_BUILD_STATIC_TESTS
    "Build compile-time pass and duplicate-name failure checks"
    "${_jh_tiny_test_tdd_default}"
)
unset(_jh_tiny_test_tdd_default)

option(JH_TINY_TEST_INSTALL
    "Install the standalone jh-tiny-test package"
    ${PROJECT_IS_TOP_LEVEL}
)

set(JH_TINY_TEST_TOOLKIT_SOURCE_ROOT "")
set(_jh_tiny_test_search_dir "${CMAKE_CURRENT_SOURCE_DIR}")
while(NOT _jh_tiny_test_search_dir STREQUAL "")
    if(EXISTS "${_jh_tiny_test_search_dir}/include/jh/metax/expected.h"
            AND EXISTS "${_jh_tiny_test_search_dir}/CMakeLists.txt")
        set(JH_TINY_TEST_TOOLKIT_SOURCE_ROOT "${_jh_tiny_test_search_dir}")
        break()
    endif()

    get_filename_component(_jh_tiny_test_parent "${_jh_tiny_test_search_dir}" DIRECTORY)
    if(_jh_tiny_test_parent STREQUAL _jh_tiny_test_search_dir)
        break()
    endif()
    set(_jh_tiny_test_search_dir "${_jh_tiny_test_parent}")
endwhile()
unset(_jh_tiny_test_search_dir)
unset(_jh_tiny_test_parent)

if(TARGET jh-toolkit)
    set(JH_TINY_TEST_TOOLKIT_TARGET jh-toolkit)
elseif(TARGET jh::jh-toolkit)
    set(JH_TINY_TEST_TOOLKIT_TARGET jh::jh-toolkit)
else()
    if(NOT JH_TINY_TEST_TOOLKIT_SOURCE_ROOT)
        find_package(jh-toolkit CONFIG REQUIRED COMPONENTS jh-test-enabled)
        set(JH_TINY_TEST_TOOLKIT_TARGET jh::jh-toolkit)
    else()
        # Source-only embedding exposes the checkout root to tiny-test builds.
        add_library(jh_tiny_test_toolkit_source INTERFACE)
        target_include_directories(jh_tiny_test_toolkit_source
            INTERFACE "${JH_TINY_TEST_TOOLKIT_SOURCE_ROOT}/include")
        set(JH_TINY_TEST_TOOLKIT_TARGET jh_tiny_test_toolkit_source)
    endif()
endif()

if(TARGET ${JH_TINY_TEST_TOOLKIT_TARGET})
    get_target_property(_jh_tiny_toolkit_components
        ${JH_TINY_TEST_TOOLKIT_TARGET} JH_TOOLKIT_AVAILABLE_COMPONENTS)
    if(_jh_tiny_toolkit_components
            AND NOT "jh-test-enabled" IN_LIST _jh_tiny_toolkit_components)
        message(FATAL_ERROR
            "jh-tiny-test requires JH-Toolkit component jh-test-enabled")
    endif()
endif()

add_library(jh_test_tiny_test INTERFACE)
add_library(jh::test::jh-tiny-test ALIAS jh_test_tiny_test)
target_sources(jh_test_tiny_test INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/jh/test/tiny_test/tiny_test.hpp>")
target_compile_features(jh_test_tiny_test INTERFACE cxx_std_20)
target_include_directories(jh_test_tiny_test
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
if(JH_TINY_TEST_TOOLKIT_SOURCE_ROOT)
    # Publish the toolkit root directly so IDE indexers resolve its public headers.
    target_include_directories(jh_test_tiny_test INTERFACE
        "$<BUILD_INTERFACE:${JH_TINY_TEST_TOOLKIT_SOURCE_ROOT}/include>")
endif()
target_link_libraries(jh_test_tiny_test
    INTERFACE
        "$<BUILD_INTERFACE:${JH_TINY_TEST_TOOLKIT_TARGET}>"
        "$<INSTALL_INTERFACE:jh::jh-toolkit>"
)
set_target_properties(jh_test_tiny_test PROPERTIES EXPORT_NAME "jh-tiny-test")

if(JH_TINY_TEST_BUILD_TDD)
    add_subdirectory(tdd)
endif()

if(JH_TINY_TEST_BUILD_STATIC_TESTS)
    add_subdirectory(static_tests)
endif()

if(JH_TINY_TEST_INSTALL)
    include(CMakePackageConfigHelpers)

    configure_package_config_file(
        "${CMAKE_CURRENT_LIST_DIR}/cmake/jh-tiny-test-config.cmake.in"
        "${PROJECT_BINARY_DIR}/jh-tiny-test-config.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/jh-tiny-test"
    )
    write_basic_package_version_file(
        "${PROJECT_BINARY_DIR}/jh-tiny-test-config-version.cmake"
        VERSION "${PROJECT_VERSION}"
        COMPATIBILITY SameMajorVersion
    )

    install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include/jh/test/tiny_test/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/jh/test/tiny_test")
    install(TARGETS jh_test_tiny_test EXPORT jh-tiny-test-targets)
    install(EXPORT jh-tiny-test-targets
        FILE jh-tiny-test-targets.cmake
        NAMESPACE jh::test::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/jh-tiny-test")
    install(FILES
        "${PROJECT_BINARY_DIR}/jh-tiny-test-config.cmake"
        "${PROJECT_BINARY_DIR}/jh-tiny-test-config-version.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/jh-tiny-test")
endif()
