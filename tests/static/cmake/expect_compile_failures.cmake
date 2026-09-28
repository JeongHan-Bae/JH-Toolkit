include("${STATIC_FAIL_CASES_FILE}")

set(configure_command
    "${CMAKE_COMMAND}"
    -S "${STATIC_RUNNER_SOURCE_DIR}"
    -B "${STATIC_RUNNER_BINARY_DIR}"
    "-DSTATIC_TOOLKIT_SOURCE_DIR=${STATIC_TOOLKIT_SOURCE_DIR}"
    "-DSTATIC_FAIL_CASES_FILE=${STATIC_FAIL_CASES_FILE}"
)
if(STATIC_CXX_COMPILER)
    list(APPEND configure_command "-DCMAKE_CXX_COMPILER=${STATIC_CXX_COMPILER}")
endif()
if(STATIC_BUILD_TYPE)
    list(APPEND configure_command "-DCMAKE_BUILD_TYPE=${STATIC_BUILD_TYPE}")
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
        "Could not configure the compile-failure runner (result: ${configure_result})\n"
        "${configure_output}"
    )
endif()

set(case_count 0)
set(failed_count 0)
set(failure_report "")

foreach(case_file IN LISTS STATIC_FAIL_CASE_FILES)
    include("${case_file}")
    math(EXPR case_count "${case_count} + 1")

    set(build_command "${CMAKE_COMMAND}" --build "${STATIC_RUNNER_BINARY_DIR}"
        --target "static_fail_translation_unit_${STATIC_TEST_NAME}"
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
    elseif(DEFINED STATIC_EXPECT_DIAGNOSTIC AND
           NOT STATIC_EXPECT_DIAGNOSTIC STREQUAL "")
        string(FIND "${build_output}" "${STATIC_EXPECT_DIAGNOSTIC}" diagnostic_position)
        if(diagnostic_position EQUAL -1)
            set(case_failed TRUE)
            set(case_reason
                "Compilation failed without the required diagnostic: ${STATIC_EXPECT_DIAGNOSTIC}"
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

if(failed_count GREATER 0)
    message(FATAL_ERROR
        "Compile-fail checks failed (${failed_count}/${case_count} cases):"
        "${failure_report}"
    )
endif()
