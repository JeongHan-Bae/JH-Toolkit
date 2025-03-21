# Explicitly declare C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define example directory
add_executable(example_generator example_generator.cpp)
add_executable(example_immutable_str example_immutable_str.cpp)
add_executable(example_data_sink example_data_sink.cpp)

# Ensure proper linking
target_link_libraries(example_generator PRIVATE jh-toolkit jh-toolkit-impl)
target_link_libraries(example_immutable_str PRIVATE jh-toolkit jh-toolkit-impl)
target_link_libraries(example_data_sink PRIVATE jh-toolkit jh-toolkit-impl)

# Ensure the include directory is correctly set
target_include_directories(example_generator PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(example_immutable_str PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(example_data_sink PRIVATE ${CMAKE_SOURCE_DIR}/include)