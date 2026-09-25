set(configure_command
    "${CMAKE_COMMAND}"
    -S "${STATIC_RUNNER_SOURCE_DIR}"
    -B "${STATIC_RUNNER_BINARY_DIR}"
    "-DSTATIC_TOOLKIT_SOURCE_DIR=${STATIC_TOOLKIT_SOURCE_DIR}"
    "-DSTATIC_TEST_SOURCE=${STATIC_TEST_SOURCE}"
    "-DSTATIC_TEST_TARGET=${STATIC_TEST_TARGET}"
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
    OUTPUT_QUIET
    ERROR_QUIET
)
if(NOT "${configure_result}" STREQUAL "0")
    message(FATAL_ERROR "Could not configure the compile-failure consumer")
endif()

set(build_command "${CMAKE_COMMAND}" --build "${STATIC_RUNNER_BINARY_DIR}"
    --target static_fail_translation_unit)
if(STATIC_BUILD_TYPE)
    list(APPEND build_command --config "${STATIC_BUILD_TYPE}")
endif()

execute_process(
    COMMAND ${build_command}
    RESULT_VARIABLE build_result
    OUTPUT_QUIET
    ERROR_QUIET
)
if("${build_result}" STREQUAL "0")
    message(FATAL_ERROR "${STATIC_TEST_NAME} compiled successfully but must be rejected")
endif()
