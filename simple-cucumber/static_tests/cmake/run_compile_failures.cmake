include("${STATIC_FAIL_CASES_FILE}")

set(case_count 0)
set(failed_count 0)
set(failure_report "")

foreach(case_file IN LISTS STATIC_FAIL_CASE_FILES)
    include("${case_file}")
    math(EXPR case_count "${case_count} + 1")

    set(build_command "${CMAKE_COMMAND}" --build "${STATIC_BUILD_DIR}"
        --target "${STATIC_TEST_TARGET}"
    )
    if(STATIC_BUILD_TYPE)
        list(APPEND build_command --config "${STATIC_BUILD_TYPE}")
    endif()

    execute_process(
        COMMAND ${build_command}
        RESULT_VARIABLE build_result
        OUTPUT_VARIABLE build_stdout
        ERROR_VARIABLE build_stderr
    )
    string(CONCAT build_output "${build_stdout}" "${build_stderr}")

    set(case_failed FALSE)
    if("${build_result}" STREQUAL "0")
        set(case_failed TRUE)
        set(case_reason "The source compiled successfully, but compilation was expected to fail.")
    else()
        string(FIND "${build_output}" "${STATIC_EXPECTED_DIAGNOSTIC}" diagnostic_position)
        if(diagnostic_position EQUAL -1)
            set(case_failed TRUE)
            set(case_reason
                "Compilation failed without the required diagnostic: ${STATIC_EXPECTED_DIAGNOSTIC}"
            )
        endif()
    endif()

    if(case_failed)
        math(EXPR failed_count "${failed_count} + 1")
        string(APPEND failure_report
            "\n[${STATIC_TEST_NAME}] ${case_reason}\n"
            "Source: ${STATIC_TEST_SOURCE}\n"
            "Build result: ${build_result}\n"
        )
        if(NOT "${build_stdout}" STREQUAL "")
            string(APPEND failure_report "Build output:\n${build_stdout}\n")
        endif()
        if(NOT "${build_stderr}" STREQUAL "")
            string(APPEND failure_report "Build errors:\n${build_stderr}\n")
        endif()
    endif()
endforeach()

if(failed_count GREATER 0)
    message(FATAL_ERROR
        "Cucumber compile-fail checks failed (${failed_count}/${case_count} cases):"
        "${failure_report}"
    )
endif()
