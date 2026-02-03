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
        test_generator.cpp      jh-toolkit
        test_immutable_str.cpp  jh-toolkit-static
        test_sequence.cpp       jh-toolkit
        test_pool.cpp           jh-toolkit
        test_runtime_arr.cpp    jh-toolkit-static
        test_ranges.cpp         jh-toolkit
        test_pod_sys.cpp        jh-toolkit
        test_base64.cpp         jh-toolkit
        test_t_str.cpp          jh-toolkit
        test_process_lock.cpp   jh-toolkit
        test_occ.cpp            jh-toolkit
        test_proc_mapping.cpp   jh-toolkit
        test_proc_cond.cpp      jh-toolkit
        test_shproc_lock.cpp    jh-toolkit
        test_fiber.cpp          jh-toolkit
        test_lookup_map.cpp     jh-toolkit
        test_huffman.cpp        jh-toolkit
        test_ordered_map.cpp    jh-toolkit
        test_slot.cpp           jh-toolkit
        test_adt.cpp            jh-toolkit
        test_flat_multimap.cpp  jh-toolkit
)

# Get total number of entries
list(LENGTH TESTS num_entries)
math(EXPR num_tests "${num_entries} / 2")
math(EXPR max_index "${num_tests} - 1")

set(USE_GCC_WORKAROUND OFF)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    string(REPLACE "." ";" GCC_VER_LIST ${CMAKE_CXX_COMPILER_VERSION})
    list(GET GCC_VER_LIST 0 GCC_MAJOR)
    list(GET GCC_VER_LIST 1 GCC_MINOR)

    if (GCC_MAJOR LESS 14 OR (GCC_MAJOR EQUAL 14 AND GCC_MINOR LESS 3))
        set(USE_GCC_WORKAROUND ON)
        message(STATUS
                "GCC ${CMAKE_CXX_COMPILER_VERSION} detected (< 14.3), "
                "enabling lightweight workaround flags for test targets.")
    endif()
endif()

# Iterate and create test targets
foreach(i RANGE 0 ${max_index})
    math(EXPR idx "2 * ${i}")
    math(EXPR idx1 "${idx} + 1")

    list(GET TESTS ${idx}  source)
    list(GET TESTS ${idx1} lib)
    get_filename_component(fname ${source} NAME)
    get_filename_component(target ${fname} NAME_WE)

    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE ${lib} Catch2::Catch2)
    target_sources(${target} PRIVATE ${PROJECT_SOURCE_DIR}/tests/main.cpp)
    target_include_directories(${target} PRIVATE ${PROJECT_SOURCE_DIR}/tests)

    if (USE_GCC_WORKAROUND)
        target_compile_options(${target} PRIVATE
                -g0
                -fno-var-tracking
                -fno-var-tracking-assignments
                -fno-inline
        )
    endif()

    # Register the test with CTest
    string(REGEX REPLACE "^test_" "" test_name ${target})
    add_test(NAME ${test_name}_test COMMAND ${target})
endforeach()

add_dependencies(test_process_lock writer reader)
add_dependencies(test_proc_mapping counter pod_writer)
add_dependencies(test_proc_cond sleeper awaker)
