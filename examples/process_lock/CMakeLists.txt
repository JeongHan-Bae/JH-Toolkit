# examples/process_lock/CMakeLists.txt

add_executable(writer example_writer.cpp)
add_executable(reader example_reader.cpp)

foreach(target writer reader example_process_lock)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/include)
endforeach()
