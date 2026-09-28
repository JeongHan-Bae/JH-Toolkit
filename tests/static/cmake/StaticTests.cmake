function(project_static_pass name source)
    cmake_parse_arguments(STATIC "" "TARGET" "" ${ARGN})
    if(NOT STATIC_TARGET)
        set(STATIC_TARGET jh-toolkit)
    endif()

    set(target "static_pass_${name}")
    add_library(${target} OBJECT "${source}")
    target_link_libraries(${target} PRIVATE "${STATIC_TARGET}")
    set_target_properties(${target} PROPERTIES EXCLUDE_FROM_ALL TRUE)
    set_property(DIRECTORY APPEND PROPERTY JH_STATIC_PASS_TARGETS "${target}")

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        string(REPLACE "." ";" gcc_version_parts "${CMAKE_CXX_COMPILER_VERSION}")
        list(GET gcc_version_parts 0 gcc_major)
        list(GET gcc_version_parts 1 gcc_minor)
        if(gcc_major LESS 14 OR (gcc_major EQUAL 14 AND gcc_minor LESS 3))
            target_compile_options(${target} PRIVATE
                -g0
                -fno-var-tracking
                -fno-var-tracking-assignments
                -fno-inline
            )
        endif()
    endif()
endfunction()

function(project_static_pass_finalize)
    get_property(pass_targets DIRECTORY PROPERTY JH_STATIC_PASS_TARGETS)
    if(NOT pass_targets)
        return()
    endif()

    set(manifest "${CMAKE_CURRENT_BINARY_DIR}/static_pass_targets.cmake")
    set(manifest_contents "set(STATIC_PASS_TARGETS)\n")
    foreach(pass_target IN LISTS pass_targets)
        _project_static_cmake_quote(quoted_target "${pass_target}")
        string(APPEND manifest_contents
            "list(APPEND STATIC_PASS_TARGETS ${quoted_target})\n"
        )
    endforeach()
    file(WRITE "${manifest}" "${manifest_contents}")

    add_test(NAME static_compile_passes
        COMMAND "${CMAKE_COMMAND}"
            "-DSTATIC_BUILD_DIR=${CMAKE_BINARY_DIR}"
            "-DSTATIC_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
            "-DSTATIC_PASS_TARGETS_FILE=${manifest}"
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/run_compile_passes.cmake"
    )
endfunction()

function(_project_static_cmake_quote output value)
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

function(project_static_fail name source)
    cmake_parse_arguments(STATIC "" "TARGET;EXPECT_DIAGNOSTIC" "" ${ARGN})
    if(NOT STATIC_TARGET)
        set(STATIC_TARGET jh-toolkit)
    endif()

    if(NOT DEFINED STATIC_EXPECT_DIAGNOSTIC)
        set(STATIC_EXPECT_DIAGNOSTIC "")
    endif()

    set(case_directory "${CMAKE_CURRENT_BINARY_DIR}/fail_cases")
    file(MAKE_DIRECTORY "${case_directory}")
    set(case_file "${case_directory}/${name}.cmake")

    _project_static_cmake_quote(quoted_name "${name}")
    _project_static_cmake_quote(
        quoted_source "${CMAKE_CURRENT_SOURCE_DIR}/${source}"
    )
    _project_static_cmake_quote(quoted_target "${STATIC_TARGET}")
    _project_static_cmake_quote(
        quoted_diagnostic "${STATIC_EXPECT_DIAGNOSTIC}"
    )
    string(CONCAT case_contents
        "set(STATIC_TEST_NAME ${quoted_name})\n"
        "set(STATIC_TEST_SOURCE ${quoted_source})\n"
        "set(STATIC_TEST_TARGET ${quoted_target})\n"
        "set(STATIC_EXPECT_DIAGNOSTIC ${quoted_diagnostic})\n"
    )
    file(WRITE "${case_file}" "${case_contents}")
    set_property(DIRECTORY APPEND PROPERTY JH_STATIC_FAIL_CASE_FILES "${case_file}")
endfunction()

function(project_static_fail_finalize)
    get_property(case_files DIRECTORY PROPERTY JH_STATIC_FAIL_CASE_FILES)
    if(NOT case_files)
        return()
    endif()

    set(manifest "${CMAKE_CURRENT_BINARY_DIR}/static_fail_cases.cmake")
    set(manifest_contents "set(STATIC_FAIL_CASE_FILES)\n")
    foreach(case_file IN LISTS case_files)
        _project_static_cmake_quote(quoted_case_file "${case_file}")
        string(APPEND manifest_contents
            "list(APPEND STATIC_FAIL_CASE_FILES ${quoted_case_file})\n"
        )
    endforeach()
    file(WRITE "${manifest}" "${manifest_contents}")

    add_test(NAME static_compile_failures
        COMMAND "${CMAKE_COMMAND}"
            "-DSTATIC_TOOLKIT_SOURCE_DIR=${PROJECT_SOURCE_DIR}"
            "-DSTATIC_RUNNER_SOURCE_DIR=${CMAKE_CURRENT_FUNCTION_LIST_DIR}/fail_runner"
            "-DSTATIC_RUNNER_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}/runner"
            "-DSTATIC_FAIL_CASES_FILE=${manifest}"
            "-DSTATIC_GENERATOR=${CMAKE_GENERATOR}"
            "-DSTATIC_GENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM}"
            "-DSTATIC_GENERATOR_TOOLSET=${CMAKE_GENERATOR_TOOLSET}"
            "-DSTATIC_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
            "-DSTATIC_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
            "-DSTATIC_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/expect_compile_failures.cmake"
    )
endfunction()
