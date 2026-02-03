# Explicitly declare C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Common examples
set(EXAMPLE_SOURCES
        example_generator.cpp
        example_immutable_str.cpp
        example_pod.cpp
        example_process_lock.cpp
        example_occ_box.cpp
        example_process_shm.cpp
        hello.cpp
)

foreach(src ${EXAMPLE_SOURCES})
    get_filename_component(fname ${src} NAME)
    get_filename_component(target ${fname} NAME_WE)
    add_executable(${target} ${src})
    target_include_directories(${target} PRIVATE ${PROJECT_SOURCE_DIR}/include)
endforeach()

# Add process_lock subdirectory (defines writer/reader)
add_subdirectory(process_lock)

# Ensure example_process_lock is built after writer and reader
add_dependencies(example_process_lock writer reader counter)
add_dependencies(example_process_shm counter sleeper awaker pod_writer)
