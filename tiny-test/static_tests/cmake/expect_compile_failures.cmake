include("${STATIC_FAIL_CASES_FILE}")

set(configure_command
    "${CMAKE_COMMAND}"
    -S "${STATIC_RUNNER_SOURCE_DIR}"
    -B "${STATIC_RUNNER_BINARY_DIR}"
    "-DSTATIC_TOOLKIT_SOURCE_DIR=${STATIC_TOOLKIT_SOURCE_DIR}"
    "-DSTATIC_TINY_TEST_SOURCE_DIR=${STATIC_TINY_TEST_SOURCE_DIR}"
    "-DSTATIC_FAIL_CASES_FILE=${STATIC_FAIL_CASES_FILE}"
)
if(STATIC_CXX_COMPILER)
    list(APPEND configure_command "-DCMAKE_CXX_COMPILER=${STATIC_CXX_COMPILER}")
endif()
if(STATIC_BUILD_TYPE)
    list(APPEND configure_command "-DCMAKE_BUILD_TYPE=${STATIC_BUILD_TYPE}")
else()
    list(APPEND configure_command "-DCMAKE_BUILD_TYPE=Debug")
endif()
if(STATIC_TOOLCHAIN_FILE)
    list(APPEND configure_command "-DCMAKE_TOOLCHAIN_FILE=${STATIC_TOOLCHAIN_FILE}")
endif()
if(STATIC_GENERATOR)
    list(APPEND configure_command -G "${STATIC_GENERATOR}")
endif()
if(STATIC_GENERATOR_PLATFORM)
    list(APPEND configure_command -A "${STATIC_GENERATOR_PLATFORM}")
endif()
if(STATIC_GENERATOR_TOOLSET)
    list(APPEND configure_command -T "${STATIC_GENERATOR_TOOLSET}")
endif()

execute_process(
    COMMAND ${configure_command}
    RESULT_VARIABLE configure_result
    OUTPUT_VARIABLE configure_stdout
    ERROR_VARIABLE configure_stderr
)
if(NOT "${configure_result}" STREQUAL "0")
    string(CONCAT configure_output "${configure_stdout}" "${configure_stderr}")
    message(FATAL_ERROR
        "Could not configure the TinyTest compile-failure runner "
        "(result: ${configure_result})\n${configure_output}"
    )
endif()

set(check_count 0)
set(failed_count 0)
set(failure_report "")

foreach(case_file IN LISTS STATIC_FAIL_CASE_FILES)
    include("${case_file}")
    math(EXPR check_count "${check_count} + 1")

    set(build_command "${CMAKE_COMMAND}" --build "${STATIC_RUNNER_BINARY_DIR}"
        --target "tiny_test_static_fail_translation_unit_${STATIC_TEST_NAME}"
    )
    if(STATIC_BUILD_TYPE)
        list(APPEND build_command --config "${STATIC_BUILD_TYPE}")
    else()
        list(APPEND build_command --config Debug)
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
        )
        if(NOT "${build_result}" STREQUAL "0")
            string(APPEND failure_report "Build result: ${build_result}\n")
        endif()
        if(NOT "${build_stdout}" STREQUAL "")
            string(APPEND failure_report "Build output:\n${build_stdout}\n")
        endif()
        if(NOT "${build_stderr}" STREQUAL "")
            string(APPEND failure_report "Build errors:\n${build_stderr}\n")
        endif()
    endif()
endforeach()

function(_run_tiny_test_runtime_case name executable argument)
    set(runtime_check_count "${check_count}")
    set(runtime_failed_count "${failed_count}")
    set(runtime_failure_report "${failure_report}")

    execute_process(
        COMMAND "${executable}" "${argument}"
        RESULT_VARIABLE runtime_result
        OUTPUT_VARIABLE runtime_stdout
        ERROR_VARIABLE runtime_stderr
    )
    math(EXPR runtime_check_count "${runtime_check_count} + 1")

    if(NOT "${runtime_result}" STREQUAL "0")
        math(EXPR runtime_failed_count "${runtime_failed_count} + 1")
        string(APPEND runtime_failure_report
            "\n[${name}] Runtime registration check failed (result: ${runtime_result}).\n"
            "Command: ${executable} ${argument}\n"
        )
        if(NOT "${runtime_stdout}" STREQUAL "")
            string(APPEND runtime_failure_report "Program output:\n${runtime_stdout}\n")
        endif()
        if(NOT "${runtime_stderr}" STREQUAL "")
            string(APPEND runtime_failure_report "Program errors:\n${runtime_stderr}\n")
        endif()
    endif()

    set(check_count "${runtime_check_count}" PARENT_SCOPE)
    set(failed_count "${runtime_failed_count}" PARENT_SCOPE)
    set(failure_report "${runtime_failure_report}" PARENT_SCOPE)
endfunction()

_run_tiny_test_runtime_case(
    duplicate_test_across_translation_units
    "${STATIC_DUPLICATE_TEST_EXECUTABLE}"
    test
)
_run_tiny_test_runtime_case(
    duplicate_session_across_translation_units
    "${STATIC_DUPLICATE_SESSION_EXECUTABLE}"
    session
)

if(failed_count GREATER 0)
    message(FATAL_ERROR
        "TinyTest static fail checks failed (${failed_count}/${check_count} checks):"
        "${failure_report}"
    )
endif()
