# Explicitly declare C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Common examples
set(EXAMPLE_SOURCES
        example_async.cpp
        example_flat_multimap.cpp
        example_generator.cpp
        example_immutable_str.cpp
        example_meta.cpp
        example_occ_box.cpp
        example_ordered_map.cpp
        example_pod.cpp
        example_pool.cpp
        example_ranges.cpp
        example_runtime_arr.cpp
        example_serio.cpp
        example_t_str.cpp
        hello.cpp
)
if (JH_TOOLKIT_ENABLE_IPCS)
    list(APPEND EXAMPLE_SOURCES example_ipc.cpp)
endif ()

foreach(src ${EXAMPLE_SOURCES})
    get_filename_component(fname ${src} NAME)
    get_filename_component(target ${fname} NAME_WE)
    add_executable(${target} ${src})
    target_link_libraries(${target}
            PRIVATE
            jh-toolkit
    )
endforeach()

if (JH_TOOLKIT_ENABLE_IPCS)
    add_subdirectory(process_lock)
    add_dependencies(example_ipc writer reader counter sleeper awaker pod_writer)
endif ()
