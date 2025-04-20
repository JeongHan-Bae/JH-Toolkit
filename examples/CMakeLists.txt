# Explicitly declare C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define example directory
add_executable(example_generator example_generator.cpp)
add_executable(example_string example_string.cpp)
add_executable(example_pod example_pod_basics.cpp)

# Ensure proper linking
target_link_libraries(example_generator PRIVATE jh-toolkit)
target_link_libraries(example_string PRIVATE jh-toolkit jh-toolkit-impl)
target_link_libraries(example_pod PRIVATE jh-toolkit)

# Ensure the include directory is correctly set
target_include_directories(example_generator PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(example_string PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(example_pod PRIVATE ${CMAKE_SOURCE_DIR}/include)
