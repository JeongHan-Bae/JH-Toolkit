cmake_minimum_required(VERSION 3.21)
project(jh-simple-cucumber VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)

if(PROJECT_IS_TOP_LEVEL)
    include(CTest)
    set(_jh_simple_cucumber_tdd_default "${BUILD_TESTING}")
elseif(JH_ENABLE_TESTING)
    set(_jh_simple_cucumber_tdd_default ON)
else()
    set(_jh_simple_cucumber_tdd_default OFF)
endif()
option(JH_SIMPLE_CUCUMBER_BUILD_TDD
    "Build the simple-cucumber self-tests"
    "${_jh_simple_cucumber_tdd_default}"
)
unset(_jh_simple_cucumber_tdd_default)

option(JH_SIMPLE_CUCUMBER_INSTALL
    "Install the standalone jh-simple-cucumber package and its parser dependencies"
    ${PROJECT_IS_TOP_LEVEL}
)

if(TARGET jh-toolkit)
    set(JH_SIMPLE_CUCUMBER_TOOLKIT_TARGET jh-toolkit)
elseif(TARGET jh::jh-toolkit)
    set(JH_SIMPLE_CUCUMBER_TOOLKIT_TARGET jh::jh-toolkit)
else()
    # When simple-cucumber is added from a JH-Toolkit checkout (including via
    # FetchContent with SOURCE_SUBDIR), locate the checkout root independently
    # of the caller's current source and binary directories.
    set(JH_SIMPLE_CUCUMBER_SOURCE_ROOT "")
    set(_jh_toolkit_search_dir "${CMAKE_CURRENT_SOURCE_DIR}")
    while(NOT _jh_toolkit_search_dir STREQUAL "")
        if(EXISTS "${_jh_toolkit_search_dir}/include/jh/metax/expected.h"
                AND EXISTS "${_jh_toolkit_search_dir}/CMakeLists.txt")
            set(JH_SIMPLE_CUCUMBER_SOURCE_ROOT "${_jh_toolkit_search_dir}")
            break()
        endif()

        get_filename_component(_jh_toolkit_parent "${_jh_toolkit_search_dir}" DIRECTORY)
        if(_jh_toolkit_parent STREQUAL _jh_toolkit_search_dir)
            break()
        endif()
        set(_jh_toolkit_search_dir "${_jh_toolkit_parent}")
    endwhile()
    unset(_jh_toolkit_search_dir)
    unset(_jh_toolkit_parent)

    if(JH_SIMPLE_CUCUMBER_SOURCE_ROOT)
        # In source-only embedding, this interface target resolves toolkit
        # headers from the checkout. Installed exports below depend on the
        # public jh::jh-toolkit package target instead.
        add_library(jh_simple_cucumber_toolkit_source INTERFACE)
        target_include_directories(jh_simple_cucumber_toolkit_source
            INTERFACE "${JH_SIMPLE_CUCUMBER_SOURCE_ROOT}/include")
        set(JH_SIMPLE_CUCUMBER_TOOLKIT_TARGET jh_simple_cucumber_toolkit_source)
    else()
        find_package(jh-toolkit CONFIG REQUIRED)
        set(JH_SIMPLE_CUCUMBER_TOOLKIT_TARGET jh::jh-toolkit)
    endif()
endif()

file(GLOB JH_SIMPLE_CUCUMBER_PUBLIC_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_LIST_DIR}/include/jh/test/cucumber/*.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/gherkin/include/jh/test/cucumber/*.hpp"
)

add_subdirectory(gherkin)

add_library(jh_test_cucumber INTERFACE)
add_library(jh::test::jh-simple-cucumber ALIAS jh_test_cucumber)
foreach(_jh_simple_cucumber_header IN LISTS JH_SIMPLE_CUCUMBER_PUBLIC_HEADERS)
    target_sources(jh_test_cucumber INTERFACE
        "$<BUILD_INTERFACE:${_jh_simple_cucumber_header}>")
endforeach()

target_link_libraries(jh_test_cucumber
    INTERFACE jh::test::jh-simple-cucumber-gherkin
)
target_compile_features(jh_test_cucumber
    INTERFACE cxx_std_20
)
target_include_directories(jh_test_cucumber
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
set_target_properties(jh_test_cucumber PROPERTIES EXPORT_NAME "jh-simple-cucumber")

if(JH_SIMPLE_CUCUMBER_BUILD_TDD)
    add_subdirectory(tdd)
endif()

if(JH_SIMPLE_CUCUMBER_INSTALL)
    include(CMakePackageConfigHelpers)

    configure_package_config_file(
        "${CMAKE_CURRENT_LIST_DIR}/cmake/jh-simple-cucumber-config.cmake.in"
        "${PROJECT_BINARY_DIR}/jh-simple-cucumber-config.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/jh-simple-cucumber"
    )
    write_basic_package_version_file(
        "${PROJECT_BINARY_DIR}/jh-simple-cucumber-config-version.cmake"
        VERSION "${PROJECT_VERSION}"
        COMPATIBILITY SameMajorVersion
    )

    install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include/jh/test/cucumber/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/jh/test/cucumber")
    install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/gherkin/include/jh/test/cucumber/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/jh/test/cucumber")

    install(TARGETS jh_test_cucumber EXPORT jh-simple-cucumber-targets)
    install(TARGETS jh_test_cucumber_gherkin EXPORT jh-simple-cucumber-targets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
    install(EXPORT jh-simple-cucumber-targets
        FILE jh-simple-cucumber-targets.cmake
        NAMESPACE jh::test::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/jh-simple-cucumber")
    install(FILES
        "${PROJECT_BINARY_DIR}/jh-simple-cucumber-config.cmake"
        "${PROJECT_BINARY_DIR}/jh-simple-cucumber-config-version.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/jh-simple-cucumber")
endif()
