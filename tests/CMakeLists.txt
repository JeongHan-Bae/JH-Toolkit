include(FetchContent)

# Fetch Catch2 dynamically
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.0 # Use the latest stable version
)

# Enable benchmark mode (OFF by default)
set(ENABLE_BENCHMARK ${ENABLE_BENCHMARK} CACHE BOOL "Enable benchmark tests")

FetchContent_MakeAvailable(Catch2)

# Create test executables
add_executable(test_generator test_generator.cpp)
add_executable(test_immutable_str test_immutable_str.cpp)
add_executable(test_sequence test_sequence.cpp)
add_executable(test_pool test_pool.cpp)
add_executable(test_data_sink test_data_sink.cpp)
add_executable(test_pod_stack test_pod_stack.cpp)
add_executable(test_runtime_arr test_runtime_arr.cpp)
add_executable(test_view test_view.cpp)

# Ensure Catch2 is linked properly
target_link_libraries(test_generator PRIVATE jh-toolkit Catch2::Catch2WithMain)
target_link_libraries(test_immutable_str PRIVATE jh-toolkit-impl Catch2::Catch2WithMain)
target_link_libraries(test_sequence PRIVATE jh-toolkit Catch2::Catch2WithMain)
target_link_libraries(test_pool PRIVATE jh-toolkit Catch2::Catch2WithMain)
target_link_libraries(test_data_sink PRIVATE jh-toolkit Catch2::Catch2WithMain)
target_link_libraries(test_pod_stack PRIVATE jh-toolkit Catch2::Catch2WithMain)
target_link_libraries(test_runtime_arr PRIVATE jh-toolkit Catch2::Catch2WithMain)
target_link_libraries(test_view PRIVATE jh-toolkit Catch2::Catch2WithMain)

# Ensure test executables can find Catch2 headers
target_include_directories(test_generator PRIVATE ${CMAKE_SOURCE_DIR}/tests)
target_include_directories(test_immutable_str PRIVATE ${CMAKE_SOURCE_DIR}/tests)
target_include_directories(test_sequence PRIVATE ${CMAKE_SOURCE_DIR}/tests)
target_include_directories(test_pool PRIVATE ${CMAKE_SOURCE_DIR}/tests)
target_include_directories(test_data_sink PRIVATE ${CMAKE_SOURCE_DIR}/tests)
target_include_directories(test_pod_stack PRIVATE ${CMAKE_SOURCE_DIR}/tests)
target_include_directories(test_runtime_arr PRIVATE ${CMAKE_SOURCE_DIR}/tests)
target_include_directories(test_view PRIVATE ${CMAKE_SOURCE_DIR}/tests)


# Register tests with CTest
add_test(NAME generator_test COMMAND test_generator)
add_test(NAME immutable_str_test COMMAND test_immutable_str)
add_test(NAME sequence_test COMMAND test_sequence)
add_test(NAME sim_pool_test COMMAND test_pool)
add_test(NAME data_sink_test COMMAND test_data_sink)
add_test(NAME pod_stack_test COMMAND test_pod_stack)
add_test(NAME runtime_arr_test COMMAND test_runtime_arr)
add_test(NAME view_test COMMAND test_view)


# Benchmark only if ENABLE_BENCHMARK is ON
if(ENABLE_BENCHMARK)
    message(STATUS "Benchmarking enabled: Adding benchmark executable")

    if(JH_DISABLE_HIGH_ALLOC_BENCHMARK)
        message(WARNING "std::list benchmark disabled by user")
        add_compile_definitions(JH_DISABLE_HIGH_ALLOC_BENCHMARK=1)
    endif()

    # ⚙️ Force optimized flags for benchmark build
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -march=native -mtune=native")

    add_executable(test_benchmark benchmarks.cpp)
    target_link_libraries(test_benchmark PRIVATE jh-toolkit jh-toolkit-impl Catch2::Catch2WithMain)
    target_include_directories(test_benchmark PRIVATE ${CMAKE_SOURCE_DIR}/tests)
endif()
