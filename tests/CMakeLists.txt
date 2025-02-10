include(FetchContent)

# Fetch Catch2 dynamically
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.0 # Use the latest stable version
)

FetchContent_MakeAvailable(Catch2)

# Create test executables
add_executable(test_generator test_generator.cpp)
add_executable(test_immutable_str test_immutable_str.cpp)
add_executable(test_sequence test_sequence.cpp)
add_executable(test_pool test_pool.cpp)

# Ensure Catch2 is linked properly
target_link_libraries(test_generator PRIVATE jh-toolkit Catch2::Catch2WithMain)
target_link_libraries(test_immutable_str PRIVATE jh-toolkit jh-toolkit-impl Catch2::Catch2WithMain)
target_link_libraries(test_sequence PRIVATE jh-toolkit Catch2::Catch2WithMain)
target_link_libraries(test_pool PRIVATE jh-toolkit Catch2::Catch2WithMain)

# Ensure test executables can find Catch2 headers
target_include_directories(test_generator PRIVATE ${CMAKE_SOURCE_DIR}/tests)
target_include_directories(test_immutable_str PRIVATE ${CMAKE_SOURCE_DIR}/tests)
target_include_directories(test_sequence PRIVATE ${CMAKE_SOURCE_DIR}/tests)
target_include_directories(test_pool PRIVATE ${CMAKE_SOURCE_DIR}/tests)


# Register tests with CTest
add_test(NAME generator_test COMMAND test_generator)
add_test(NAME immutable_str_test COMMAND test_immutable_str)
add_test(NAME sequence_test COMMAND test_sequence)
add_test(NAME sim_pool_test COMMAND test_pool)
