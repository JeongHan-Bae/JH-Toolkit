function(project_static_pass name source)
    set(target "tiny_test_static_pass_${name}")
    add_library(${target} OBJECT "${source}")
    target_link_libraries(${target} PRIVATE jh::test::jh-tiny-test)
    target_compile_features(${target} PRIVATE cxx_std_20)
endfunction()

function(project_static_fail name source expected_diagnostic)
    add_test(NAME "tiny_test_static_fail_${name}"
        COMMAND "${CMAKE_COMMAND}"
            "-DSTATIC_TEST_NAME=${name}"
            "-DSTATIC_TEST_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/${source}"
            "-DSTATIC_EXPECTED_DIAGNOSTIC=${expected_diagnostic}"
            "-DSTATIC_TOOLKIT_SOURCE_DIR=${JH_TINY_TEST_TOOLKIT_SOURCE_ROOT}"
            "-DSTATIC_TINY_TEST_SOURCE_DIR=${CMAKE_CURRENT_LIST_DIR}/../.."
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
