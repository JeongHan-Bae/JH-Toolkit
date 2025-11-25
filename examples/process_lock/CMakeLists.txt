# examples/process_lock/CMakeLists.txt

set(SOURCE_LIST
        example_writer.cpp
        example_reader.cpp
        example_counter.cpp
        example_awaker.cpp
        example_sleeper.cpp
        example_pod_writer.cpp
)

foreach(src ${SOURCE_LIST})
    get_filename_component(fname ${src} NAME)
    get_filename_component(base ${fname} NAME_WE)
    string(REGEX REPLACE "^example_" "" target ${base})
    add_executable(${target} ${src})
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/include)
endforeach()
