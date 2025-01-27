cmake_minimum_required(VERSION 3.29)
project(jh-toolkit LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)  # Ensure standardized installation directories

# Define the INTERFACE library for header-only components
add_library(jh-toolkit INTERFACE)
target_include_directories(jh-toolkit INTERFACE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Define the STATIC library for compiled components (e.g., immutable_str.cpp)
add_library(jh-toolkit-impl STATIC src/immutable_str.cpp)
target_include_directories(jh-toolkit-impl PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(jh-toolkit-impl PUBLIC jh-toolkit)

# Install target libraries
install(TARGETS jh-toolkit jh-toolkit-impl
        EXPORT jh-toolkitTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install header files
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Generate and install CMake package configuration files
install(EXPORT jh-toolkitTargets
        FILE jh-toolkitTargets.cmake
        NAMESPACE jh::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/jh-toolkit
)

# Manually written CMake configuration file (for find_package)
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/jh-toolkit-config-version.cmake"
        VERSION 1.1.1
        COMPATIBILITY SameMajorVersion
)

configure_file(
        "${CMAKE_SOURCE_DIR}/cmake/jh-toolkit-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/jh-toolkit-config.cmake"
        @ONLY
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/jh-toolkit-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/jh-toolkit-config-version.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/jh-toolkit
)

# Enable tests and examples only in Debug mode
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    enable_testing()
    add_subdirectory(tests)
    add_subdirectory(examples)
endif()
