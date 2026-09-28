function(project_static_pass name source)
    set(target "tiny_test_static_pass_${name}")
    add_library(${target} OBJECT "${source}")
    target_link_libraries(${target} PRIVATE jh::test::jh-tiny-test)
    target_compile_features(${target} PRIVATE cxx_std_20)
endfunction()

function(_tiny_test_static_fail_cmake_quote output value)
    set(equals "")
    while(TRUE)
        string(FIND "${value}" "]${equals}]" end_position)
        if(end_position EQUAL -1)
            break()
        endif()
        string(APPEND equals "=")
    endwhile()

    set(${output} "[${equals}[${value}]${equals}]" PARENT_SCOPE)
endfunction()

function(project_static_fail name source expected_diagnostic)
    set(case_directory "${CMAKE_CURRENT_BINARY_DIR}/fail_cases")
    file(MAKE_DIRECTORY "${case_directory}")
    set(case_file "${case_directory}/${name}.cmake")

    _tiny_test_static_fail_cmake_quote(quoted_name "${name}")
    _tiny_test_static_fail_cmake_quote(
        quoted_source "${CMAKE_CURRENT_SOURCE_DIR}/${source}"
    )
    _tiny_test_static_fail_cmake_quote(quoted_diagnostic "${expected_diagnostic}")
    string(CONCAT case_contents
        "set(STATIC_TEST_NAME ${quoted_name})\n"
        "set(STATIC_TEST_SOURCE ${quoted_source})\n"
        "set(STATIC_EXPECTED_DIAGNOSTIC ${quoted_diagnostic})\n"
    )
    file(WRITE "${case_file}" "${case_contents}")
    set_property(DIRECTORY APPEND PROPERTY JH_TINY_TEST_STATIC_FAIL_CASE_FILES
        "${case_file}"
    )
endfunction()

function(project_static_fail_finalize duplicate_test_target duplicate_session_target)
    get_property(case_files DIRECTORY PROPERTY JH_TINY_TEST_STATIC_FAIL_CASE_FILES)
    if(NOT case_files)
        return()
    endif()

    set(manifest "${CMAKE_CURRENT_BINARY_DIR}/static_fail_cases.cmake")
    set(manifest_contents "set(STATIC_FAIL_CASE_FILES)\n")
    foreach(case_file IN LISTS case_files)
        _tiny_test_static_fail_cmake_quote(quoted_case_file "${case_file}")
        string(APPEND manifest_contents
            "list(APPEND STATIC_FAIL_CASE_FILES ${quoted_case_file})\n"
        )
    endforeach()
    file(WRITE "${manifest}" "${manifest_contents}")

    add_test(NAME tiny_test_static_failures
        COMMAND "${CMAKE_COMMAND}"
            "-DSTATIC_TOOLKIT_SOURCE_DIR=${JH_TINY_TEST_TOOLKIT_SOURCE_ROOT}"
            "-DSTATIC_TINY_TEST_SOURCE_DIR=${PROJECT_SOURCE_DIR}"
            "-DSTATIC_RUNNER_SOURCE_DIR=${CMAKE_CURRENT_FUNCTION_LIST_DIR}/fail_runner"
            "-DSTATIC_RUNNER_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}/runner"
            "-DSTATIC_FAIL_CASES_FILE=${manifest}"
            "-DSTATIC_DUPLICATE_TEST_EXECUTABLE=$<TARGET_FILE:${duplicate_test_target}>"
            "-DSTATIC_DUPLICATE_SESSION_EXECUTABLE=$<TARGET_FILE:${duplicate_session_target}>"
            "-DSTATIC_GENERATOR=${CMAKE_GENERATOR}"
            "-DSTATIC_GENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM}"
            "-DSTATIC_GENERATOR_TOOLSET=${CMAKE_GENERATOR_TOOLSET}"
            "-DSTATIC_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
            "-DSTATIC_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
            "-DSTATIC_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/expect_compile_failures.cmake"
    )
endfunction()
