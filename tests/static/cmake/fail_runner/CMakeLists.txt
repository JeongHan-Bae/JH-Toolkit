cmake_minimum_required(VERSION 3.21)
project(jh_static_compile_failure LANGUAGES CXX)

set(JH_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(JH_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(USE_JH_COMPILE_OPT OFF CACHE BOOL "" FORCE)

add_subdirectory("${STATIC_TOOLKIT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/jh-toolkit")

add_library(static_fail_translation_unit OBJECT "${STATIC_TEST_SOURCE}")
target_link_libraries(static_fail_translation_unit PRIVATE "${STATIC_TEST_TARGET}")
