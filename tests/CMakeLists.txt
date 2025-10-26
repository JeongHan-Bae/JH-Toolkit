include(FetchContent)

# Fetch Catch2 dynamically
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.0 # Use the latest stable version
)

set(ENABLE_BENCHMARK ${ENABLE_BENCHMARK} CACHE BOOL "Enable benchmark tests")

FetchContent_MakeAvailable(Catch2)

# ---------------------------
# Define test targets in one list
# Format: target_name source_file linked_library
# ---------------------------
set(TESTS
        test_generator     test_generator.cpp     jh-toolkit
        test_immutable_str test_immutable_str.cpp jh-toolkit-static
        test_sequence      test_sequence.cpp      jh-toolkit
        test_pool          test_pool.cpp          jh-toolkit
        test_runtime_arr   test_runtime_arr.cpp   jh-toolkit-static
        test_view          test_view.cpp          jh-toolkit
        test_pods          test_pod_sys.cpp       jh-toolkit
        test_base64        test_base64.cpp        jh-toolkit
        test_str_template  test_str_template.cpp  jh-toolkit
        test_process_lock  test_process_lock.cpp  jh-toolkit
        test_occ           test_occ.cpp           jh-toolkit
        test_proc_counter  test_proc_counter.cpp  jh-toolkit
        test_proc_cond     test_proc_cond.cpp     jh-toolkit
        test_shproc_lock   test_shproc_lock.cpp   jh-toolkit
)

# Get total number of entries
list(LENGTH TESTS num_entries)
math(EXPR num_tests "${num_entries} / 3")
math(EXPR max_index "${num_tests} - 1")

# Iterate and create test targets
foreach(i RANGE 0 ${max_index})
    math(EXPR idx "3 * ${i}")
    math(EXPR idx1 "${idx} + 1")
    math(EXPR idx2 "${idx} + 2")

    list(GET TESTS ${idx}  target)
    list(GET TESTS ${idx1} source)
    list(GET TESTS ${idx2} lib)

    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE ${lib} Catch2::Catch2WithMain)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/tests)

    # Register the test with CTest
    string(REGEX REPLACE "^test_" "" test_name ${target})
    add_test(NAME ${test_name}_test COMMAND ${target})
endforeach()

add_dependencies(test_process_lock writer reader)
add_dependencies(test_proc_counter counter)
add_dependencies(test_proc_cond sleeper awaker)
