include(FetchContent)

# Gherkin's C++ library depends on the official Cucumber Messages C++ library.
option(JH_SIMPLE_CUCUMBER_FETCH_PARSER_DEPS
    "Fetch Gherkin and Cucumber Messages dependencies when no package is provided"
    ON
)
set(CUCUMBER_GHERKIN_FETCH_DEPS "${JH_SIMPLE_CUCUMBER_FETCH_PARSER_DEPS}")
if(JH_SIMPLE_CUCUMBER_FETCH_PARSER_DEPS)
    # Cucumber Messages may resolve nlohmann_json through CPM's local-package
    # mode. Find it here first so an imported target is visible from the
    # sibling Gherkin directory as well.
    find_package(nlohmann_json 3.12.0 CONFIG QUIET)
endif()
if(NOT JH_SIMPLE_CUCUMBER_INSTALL)
    # This subproject is embedded in the toolkit's test build; don't add
    # third-party install rules to the parent project's install set.
    set(CMAKE_SKIP_INSTALL_RULES TRUE)
endif()

FetchContent_Declare(
    cucumber_gherkin
    GIT_REPOSITORY https://github.com/cucumber/gherkin.git
    GIT_TAG v42.0.0
    SOURCE_SUBDIR cpp
)
FetchContent_MakeAvailable(cucumber_gherkin)

# Keep warnings from the vendored upstream parser out of this project's build
# output. Do not suppress warnings on our adapter target.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(_jh_gherkin_suppress_warnings -w)
elseif(MSVC)
    set(_jh_gherkin_suppress_warnings /w)
endif()
foreach(_jh_gherkin_target IN ITEMS
        cucumber_gherkin_lib
        cucumber_gherkin_bin
        cucumber_gherkin_generate_tokens_bin)
    if(TARGET ${_jh_gherkin_target} AND _jh_gherkin_suppress_warnings)
        target_compile_options(${_jh_gherkin_target} PRIVATE ${_jh_gherkin_suppress_warnings})
    endif()
endforeach()
unset(_jh_gherkin_suppress_warnings)
unset(_jh_gherkin_target)

add_library(jh_test_cucumber_gherkin STATIC src/adapter.cpp)
target_sources(jh_test_cucumber_gherkin PRIVATE ${JH_SIMPLE_CUCUMBER_PUBLIC_HEADERS})
add_library(jh::test::jh-simple-cucumber-gherkin ALIAS jh_test_cucumber_gherkin)
target_include_directories(jh_test_cucumber_gherkin
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_include_directories(jh_test_cucumber_gherkin SYSTEM PRIVATE
    "${cucumber_gherkin_SOURCE_DIR}/cpp/include/gherkin"
)
target_compile_features(jh_test_cucumber_gherkin
    PUBLIC cxx_std_20
)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(jh_test_cucumber_gherkin PRIVATE -fno-rtti)
elseif(MSVC)
    target_compile_options(jh_test_cucumber_gherkin PRIVATE /GR-)
endif()
target_link_libraries(jh_test_cucumber_gherkin
    PUBLIC
        "$<BUILD_INTERFACE:${JH_SIMPLE_CUCUMBER_TOOLKIT_TARGET}>"
        "$<INSTALL_INTERFACE:jh::jh-toolkit>"
    PRIVATE cucumber::gherkin
)
set_target_properties(jh_test_cucumber_gherkin PROPERTIES
    EXPORT_NAME "jh-simple-cucumber-gherkin")
