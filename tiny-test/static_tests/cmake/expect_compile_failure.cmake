set(configure_command
    "${CMAKE_COMMAND}"
    -S "${STATIC_RUNNER_SOURCE_DIR}"
    -B "${STATIC_RUNNER_BINARY_DIR}"
    "-DSTATIC_TOOLKIT_SOURCE_DIR=${STATIC_TOOLKIT_SOURCE_DIR}"
    "-DSTATIC_TINY_TEST_SOURCE_DIR=${STATIC_TINY_TEST_SOURCE_DIR}"
    "-DSTATIC_TEST_SOURCE=${STATIC_TEST_SOURCE}"
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
    OUTPUT_VARIABLE configure_output
    ERROR_VARIABLE configure_error
)
if(NOT "${configure_result}" STREQUAL "0")
    message(FATAL_ERROR
        "Could not configure the TinyTest compile-failure consumer:\n"
        "${configure_output}\n${configure_error}")
endif()

set(build_command "${CMAKE_COMMAND}" --build "${STATIC_RUNNER_BINARY_DIR}"
    --target tiny_test_static_fail_translation_unit)
if(STATIC_BUILD_TYPE)
    list(APPEND build_command --config "${STATIC_BUILD_TYPE}")
endif()
execute_process(
    COMMAND ${build_command}
    RESULT_VARIABLE build_result
    OUTPUT_VARIABLE build_output
    ERROR_VARIABLE build_error
)
if("${build_result}" STREQUAL "0")
    message(FATAL_ERROR "${STATIC_TEST_NAME} compiled successfully but must be rejected")
endif()

set(diagnostics "${build_output}\n${build_error}")
if(NOT diagnostics MATCHES "${STATIC_EXPECTED_DIAGNOSTIC}")
    message(FATAL_ERROR
        "${STATIC_TEST_NAME} failed for an unexpected reason; expected diagnostic "
        "${STATIC_EXPECTED_DIAGNOSTIC}:\n${diagnostics}")
endif()
