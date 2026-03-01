cmake_minimum_required(VERSION 3.21)
project(jh-toolkit LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)  # Ensure standardized installation directories

set(PROJECT_VERSION 1.4.0)

# ------------------------------------------------------------------------------
# Compilation Model and Integration Philosophy
#
# 1. Precompiled Components
#    A small portion of the library provides precompiled implementations.
#    When linking against `jh-toolkit-static`, those compiled units are built
#    with Release-level optimizations by design.
#
#    IMPORTANT:
#    Template code remains governed by the consumer project's compilation
#    settings. Only the explicitly compiled translation units are subject
#    to the enforced optimization policy.
#
# 2. Fully Templated Usage
#    All functionality is available in header-only form via `jh-toolkit`.
#    When consumed through `find_package`, the user retains full control
#    over compilation flags and optimization strategy.
#
# 3. FetchContent / add_subdirectory Integration
#    When integrated via FetchContent or add_subdirectory, this project
#    intentionally overrides and enforces its global compilation strategy.
#
#    This behavior is deliberate:
#    Using FetchContent/add_subdirectory implies adoption of the project's
#    architectural philosophy and compilation discipline.
#
#    Consumers are expected to audit their codebase under this philosophy,
#    including constraints such as RTTI prohibition and enforced optimization
#    strategy.
#
#    If such constraints are not desired, the library should be consumed
#    via find_package instead.
# ------------------------------------------------------------------------------

# Options

option(JH_ENABLE_TESTING
        "Build jh-toolkit tests"
        ${PROJECT_IS_TOP_LEVEL})

option(JH_ENABLE_EXAMPLES
        "Build jh-toolkit examples"
        ${PROJECT_IS_TOP_LEVEL})

# Auto-enable development features (only top-level)

if (PROJECT_IS_TOP_LEVEL)

    if (WIN32 AND
    (CMAKE_BUILD_TYPE STREQUAL "Release" OR
            CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
        set(JH_ENABLE_TESTING ON CACHE BOOL "" FORCE)
        set(JH_ENABLE_EXAMPLES ON CACHE BOOL "" FORCE)
    endif ()

    if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR
            CMAKE_BUILD_TYPE STREQUAL "FastDebug")
        set(JH_ENABLE_TESTING ON CACHE BOOL "" FORCE)
        set(JH_ENABLE_EXAMPLES ON CACHE BOOL "" FORCE)
    endif ()

endif ()

if (JH_ENABLE_TESTING AND NOT JH_ENABLE_EXAMPLES)
    message(STATUS "Enabling examples because testing requires them.")
    set(JH_ENABLE_EXAMPLES ON CACHE BOOL "" FORCE)
endif ()

# Register custom build type
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo;MinSizeRel;FastDebug" CACHE STRING "" FORCE)
set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)

if (NOT CMAKE_BUILD_TYPE MATCHES "^(Debug|Release|RelWithDebInfo|MinSizeRel|FastDebug)$")
    message(FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}")
endif ()

if (PROJECT_IS_TOP_LEVEL)

    if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR
            CMAKE_BUILD_TYPE STREQUAL "FastDebug")

        message(WARNING
                "Install is disabled in Debug/FastDebug mode. "
                "Use Release or RelWithDebInfo for installation.")

        # Do not generate install rules
        set(CMAKE_SKIP_INSTALL_RULES TRUE)

    endif ()

endif ()

set(CMAKE_CXX_FLAGS_FASTDEBUG "-O2 -g1 -Wall -Wextra -Wpedantic")
set(CMAKE_C_FLAGS_FASTDEBUG  "-O2 -g1 -Wall -Wextra -Wpedantic")

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Applying extra CPU optimizations for Release build.")
    # Detect architecture-specific -march option
    set(EXTRA_OPT_FLAGS "")
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        if (NOT CMAKE_CROSSCOMPILING)
            include(CheckCXXCompilerFlag)
            check_cxx_compiler_flag("-march=native" HAS_MARCH_NATIVE)
            if (HAS_MARCH_NATIVE)
                set(EXTRA_OPT_FLAGS "-march=native")
            endif ()
        else ()
            message(STATUS "Cross-compiling for processor: ${CMAKE_SYSTEM_PROCESSOR}")
            if (CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64)$")
                set(EXTRA_OPT_FLAGS "-march=x86-64-v3")
            elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64)$")
                set(EXTRA_OPT_FLAGS "-march=armv8-a")
            elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "^armv7")
                set(EXTRA_OPT_FLAGS "-march=armv7-a")
            else ()
                message(WARNING "Unknown target architecture: ${CMAKE_SYSTEM_PROCESSOR}, no -march set")
            endif ()
        endif ()

        message(STATUS "Applied -march flag: ${EXTRA_OPT_FLAGS}")

        # Apply optimization flags based on build type
        if (CMAKE_BUILD_TYPE STREQUAL "Release")
            add_compile_options(
                    ${EXTRA_OPT_FLAGS}
                    -O3
                    -fno-rtti
                    -ftree-vectorize
                    -funroll-loops
                    -fno-omit-frame-pointer
                    -Wall -Wextra -Wpedantic
            )
            message(STATUS "Release build: enabled aggressive optimization")
        elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
            add_compile_options(
                    -O0 -g
                    -Wall -Wextra -Wpedantic
            )
            message(STATUS "Debug build: light optimization with warnings")
        endif ()
    endif ()

endif ()

string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_UPPER)
message(STATUS "${CMAKE_BUILD_TYPE} CXX flags: ${CMAKE_CXX_FLAGS_${BUILD_TYPE_UPPER}}")

set(TAR "ALL" CACHE STRING "Comma-separated list of targets to build (e.g. POD,...,ALL)")
string(TOUPPER "${TAR}" TAR_UPPER)
string(REPLACE "," ";" TAR_LIST "${TAR_UPPER}")
function(tar_enabled result_name target_name)
    if ("${target_name}" IN_LIST TAR_LIST)
        set(${result_name} TRUE PARENT_SCOPE)
    else ()
        set(${result_name} FALSE PARENT_SCOPE)
    endif ()
endfunction()

# Targets tracking
tar_enabled(ENABLE_POD "POD")
tar_enabled(ENABLE_ALL "ALL")

if (CMAKE_BUILD_TYPE STREQUAL "Release" AND NOT ENABLE_POD AND NOT ENABLE_ALL)
    message(FATAL_ERROR "Nothing selected for build. Use -DTAR=POD,ALL,...")
endif ()

# Check if we are building just the POD
if (ENABLE_POD)
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        message(STATUS ">>> Building POD module")
    endif ()
    add_library(jh-toolkit-pod INTERFACE)

    target_include_directories(jh-toolkit-pod INTERFACE
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    # Install forwarding header 'pod' (no extension, so must use FILES)
    install(FILES
            ${PROJECT_SOURCE_DIR}/include/jh/pod
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jh
    )

    # Install header directory 'pods/'
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/jh/pods
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jh
    )

    # Install depending headers in directory 'typing/'
    install(FILES
            ${PROJECT_SOURCE_DIR}/include/jh/typing/monostate.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jh/typing
    )

    # Install depending headers in directory 'macros/'
    install(FILES
            ${PROJECT_SOURCE_DIR}/include/jh/macros/platform.h
            ${PROJECT_SOURCE_DIR}/include/jh/macros/type_name.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jh/macros
    )

    # Install depending headers in directory 'metax/'
    install(FILES
            ${PROJECT_SOURCE_DIR}/include/jh/metax/hash.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jh/metax
    )

    # Install depending headers in directory 'serialize_io/'
    install(FILES
            ${PROJECT_SOURCE_DIR}/include/jh/serialize_io/base64.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jh/serialize_io
    )

    # Install depending headers in directory 'detail/'
    install(FILES
            ${PROJECT_SOURCE_DIR}/include/jh/detail/base64_common.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jh/detail
    )

    # Install the INTERFACE target
    install(TARGETS jh-toolkit-pod EXPORT jh-toolkitTargets)

    # Install CMake config files
    install(EXPORT jh-toolkitTargets
            FILE jh-toolkitTargets.cmake
            NAMESPACE jh::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/jh-toolkit
    )
endif ()
if (ENABLE_ALL)
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        message(STATUS ">>> Building main implementation (ALL)")
    endif ()
    # Define the INTERFACE library for header-only components
    add_library(jh-toolkit INTERFACE
            include/jh/)
    target_include_directories(jh-toolkit INTERFACE
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    # Define the STATIC library for compiled components (e.g., immutable_str.cpp)
    file(GLOB JH_TOOLKIT_SOURCES CONFIGURE_DEPENDS
            "${PROJECT_SOURCE_DIR}/src/*.cpp"
    )

    # Build the static library (no header-only restriction during build)
    add_library(jh-toolkit-static STATIC ${JH_TOOLKIT_SOURCES})
    target_include_directories(jh-toolkit-static PUBLIC
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
    target_link_libraries(jh-toolkit-static PUBLIC jh-toolkit)

    # When other projects link to jh-toolkit-static,
    # they automatically get JH_HEADER_NO_IMPL defined.
    target_compile_definitions(jh-toolkit-static
            PRIVATE
            JH_IS_STATIC_BUILD
            INTERFACE
            JH_HEADER_NO_IMPL
    )

    set_target_properties(jh-toolkit-static PROPERTIES
            EXPORT_COMPILE_DEFINITIONS ON
    )

    # Install target libraries
    install(TARGETS jh-toolkit jh-toolkit-static
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

endif ()

# Manually written CMake configuration file (for find_package)
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/jh-toolkit-config-version.cmake"
        VERSION PROJECT_VERSION
        COMPATIBILITY SameMajorVersion
)

configure_file(
        "${PROJECT_SOURCE_DIR}/cmake/jh-toolkit-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/jh-toolkit-config.cmake"
        @ONLY
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/jh-toolkit-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/jh-toolkit-config-version.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/jh-toolkit
)

if (JH_ENABLE_TESTING)
    enable_testing()
endif ()

if (JH_ENABLE_EXAMPLES)
    add_subdirectory(examples)
endif ()

if (JH_ENABLE_TESTING)
    add_subdirectory(tests)
endif ()
