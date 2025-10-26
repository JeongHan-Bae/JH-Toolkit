# examples/process_lock/CMakeLists.txt

add_executable(writer example_writer.cpp)
add_executable(reader example_reader.cpp)
add_executable(counter example_counter_worker.cpp)
add_executable(awaker example_awaker.cpp)
add_executable(sleeper example_sleeper.cpp)

foreach(target writer reader counter awaker sleeper)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/include)
endforeach()
