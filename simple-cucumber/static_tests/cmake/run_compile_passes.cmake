include("${STATIC_PASS_TARGETS_FILE}")

set(pass_count 0)
set(failed_count 0)
set(failure_report "")

foreach(pass_target IN LISTS STATIC_PASS_TARGETS)
    math(EXPR pass_count "${pass_count} + 1")
    set(build_command "${CMAKE_COMMAND}" --build "${STATIC_BUILD_DIR}"
        --target "${pass_target}"
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
    if(NOT "${build_result}" STREQUAL "0")
        math(EXPR failed_count "${failed_count} + 1")
        string(APPEND failure_report
            "\n[${pass_target}] Compile-pass target failed (result: ${build_result}).\n"
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
        "Cucumber compile-pass checks failed (${failed_count}/${pass_count} targets):"
        "${failure_report}"
    )
endif()
