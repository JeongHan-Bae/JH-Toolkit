# Explicitly declare C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Common examples
set(EXAMPLE_SOURCES
        example_generator.cpp
        example_immutable_str.cpp
        example_pod.cpp
        example_pool.cpp
        example_ipc.cpp
        example_occ_box.cpp
        example_runtime_arr.cpp
        example_ordered_map.cpp
        example_flat_multimap.cpp
        example_ranges.cpp
        example_async.cpp
        example_serio.cpp
        hello.cpp
)

foreach(src ${EXAMPLE_SOURCES})
    get_filename_component(fname ${src} NAME)
    get_filename_component(target ${fname} NAME_WE)
    add_executable(${target} ${src})
    target_link_libraries(${target}
            PRIVATE
            jh-toolkit
    )
endforeach()

# Add process_lock subdirectory (defines writer/reader)
add_subdirectory(process_lock)

# Ensure IPC worker executables are built before the IPC umbrella example
add_dependencies(example_ipc writer reader counter sleeper awaker pod_writer)
