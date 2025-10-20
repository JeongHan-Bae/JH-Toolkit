# Explicitly declare C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Common examples
add_executable(example_generator example_generator.cpp)
add_executable(example_string example_string.cpp)
add_executable(example_pod example_pod_basics.cpp)
add_executable(example_process_lock example_process_lock.cpp)
add_executable(example_occ_box example_occ_box.cpp)

# Common linking
foreach(target example_generator example_string example_pod example_occ_box)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${target} PRIVATE jh-toolkit-impl Catch2::Catch2WithMain)
endforeach()

# Add process_lock subdirectory (defines writer/reader)
add_subdirectory(process_lock)

# Ensure example_process_lock is built after writer and reader
add_dependencies(example_process_lock writer reader)
