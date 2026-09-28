function(_simple_cucumber_static_cmake_quote output value)
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

function(project_static_pass name source)
    set(target "simple_cucumber_static_pass_${name}")
    add_library(${target} OBJECT "${source}")
    target_link_libraries(${target} PRIVATE jh::test::jh-simple-cucumber)
    target_compile_features(${target} PRIVATE cxx_std_20)
    set_target_properties(${target} PROPERTIES EXCLUDE_FROM_ALL TRUE)

    set_property(DIRECTORY APPEND PROPERTY JH_SIMPLE_CUCUMBER_STATIC_PASS_TARGETS "${target}")
endfunction()

function(project_static_pass_finalize)
    get_property(pass_targets DIRECTORY PROPERTY JH_SIMPLE_CUCUMBER_STATIC_PASS_TARGETS)
    if(NOT pass_targets)
        return()
    endif()

    set(manifest "${CMAKE_CURRENT_BINARY_DIR}/static_pass_targets.cmake")
    set(manifest_contents "set(STATIC_PASS_TARGETS)\n")
    foreach(pass_target IN LISTS pass_targets)
        _simple_cucumber_static_cmake_quote(quoted_target "${pass_target}")
        string(APPEND manifest_contents
            "list(APPEND STATIC_PASS_TARGETS ${quoted_target})\n"
        )
    endforeach()
    file(WRITE "${manifest}" "${manifest_contents}")

    add_test(NAME simple_cucumber_static_passes
        COMMAND "${CMAKE_COMMAND}"
            "-DSTATIC_BUILD_DIR=${CMAKE_BINARY_DIR}"
            "-DSTATIC_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
            "-DSTATIC_PASS_TARGETS_FILE=${manifest}"
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/run_compile_passes.cmake"
    )
endfunction()

function(project_static_fail name source expected_diagnostic)
    set(target "simple_cucumber_static_fail_${name}")
    add_library(${target} OBJECT "${source}")
    target_link_libraries(${target} PRIVATE jh::test::jh-simple-cucumber)
    target_compile_features(${target} PRIVATE cxx_std_20)
    set_target_properties(${target} PROPERTIES EXCLUDE_FROM_ALL TRUE)

    set(case_directory "${CMAKE_CURRENT_BINARY_DIR}/fail_cases")
    file(MAKE_DIRECTORY "${case_directory}")
    set(case_file "${case_directory}/${name}.cmake")
    _simple_cucumber_static_cmake_quote(quoted_name "${name}")
    _simple_cucumber_static_cmake_quote(quoted_target "${target}")
    _simple_cucumber_static_cmake_quote(
        quoted_source "${CMAKE_CURRENT_SOURCE_DIR}/${source}"
    )
    _simple_cucumber_static_cmake_quote(
        quoted_diagnostic "${expected_diagnostic}"
    )
    string(CONCAT case_contents
        "set(STATIC_TEST_NAME ${quoted_name})\n"
        "set(STATIC_TEST_TARGET ${quoted_target})\n"
        "set(STATIC_TEST_SOURCE ${quoted_source})\n"
        "set(STATIC_EXPECTED_DIAGNOSTIC ${quoted_diagnostic})\n"
    )
    file(WRITE "${case_file}" "${case_contents}")
    set_property(DIRECTORY APPEND PROPERTY JH_SIMPLE_CUCUMBER_STATIC_FAIL_CASES
        "${case_file}"
    )
endfunction()

function(project_static_fail_finalize)
    get_property(case_files DIRECTORY PROPERTY JH_SIMPLE_CUCUMBER_STATIC_FAIL_CASES)
    if(NOT case_files)
        return()
    endif()

    set(manifest "${CMAKE_CURRENT_BINARY_DIR}/static_fail_cases.cmake")
    set(manifest_contents "set(STATIC_FAIL_CASE_FILES)\n")
    foreach(case_file IN LISTS case_files)
        _simple_cucumber_static_cmake_quote(quoted_case_file "${case_file}")
        string(APPEND manifest_contents
            "list(APPEND STATIC_FAIL_CASE_FILES ${quoted_case_file})\n"
        )
    endforeach()
    file(WRITE "${manifest}" "${manifest_contents}")

    add_test(NAME simple_cucumber_static_failures
        COMMAND "${CMAKE_COMMAND}"
            "-DSTATIC_BUILD_DIR=${CMAKE_BINARY_DIR}"
            "-DSTATIC_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
            "-DSTATIC_FAIL_CASES_FILE=${manifest}"
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/run_compile_failures.cmake"
    )
endfunction()
