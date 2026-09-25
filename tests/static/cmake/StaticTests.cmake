function(project_static_pass name source)
    cmake_parse_arguments(STATIC "" "TARGET" "" ${ARGN})
    if(NOT STATIC_TARGET)
        set(STATIC_TARGET jh-toolkit)
    endif()

    set(target "static_pass_${name}")
    add_library(${target} OBJECT "${source}")
    target_link_libraries(${target} PRIVATE "${STATIC_TARGET}")

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

function(project_static_fail name source)
    cmake_parse_arguments(STATIC "" "TARGET" "" ${ARGN})
    if(NOT STATIC_TARGET)
        set(STATIC_TARGET jh-toolkit)
    endif()

    set(test_name "static_fail_${name}")
    add_test(NAME "${test_name}"
        COMMAND "${CMAKE_COMMAND}"
            "-DSTATIC_TEST_NAME=${name}"
            "-DSTATIC_TEST_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/${source}"
            "-DSTATIC_TEST_TARGET=${STATIC_TARGET}"
            "-DSTATIC_TOOLKIT_SOURCE_DIR=${PROJECT_SOURCE_DIR}"
            "-DSTATIC_RUNNER_SOURCE_DIR=${CMAKE_CURRENT_FUNCTION_LIST_DIR}/fail_runner"
            "-DSTATIC_RUNNER_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}/runner/${name}"
            "-DSTATIC_GENERATOR=${CMAKE_GENERATOR}"
            "-DSTATIC_GENERATOR_PLATFORM=${CMAKE_GENERATOR_PLATFORM}"
            "-DSTATIC_GENERATOR_TOOLSET=${CMAKE_GENERATOR_TOOLSET}"
            "-DSTATIC_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
            "-DSTATIC_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
            "-DSTATIC_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/expect_compile_failure.cmake"
    )
endfunction()
