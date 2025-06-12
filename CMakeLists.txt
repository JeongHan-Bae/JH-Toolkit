cmake_minimum_required(VERSION 3.20)
project(jh-toolkit LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)  # Ensure standardized installation directories

set(PROJECT_VERSION 1.3.2)

# Register custom build type
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo;MinSizeRel;FastDebug" CACHE STRING "" FORCE)
set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)

if (NOT CMAKE_BUILD_TYPE MATCHES "^(Debug|Release|RelWithDebInfo|MinSizeRel|FastDebug)$")
    message(FATAL_ERROR "Unknown build type: ${CMAKE_BUILD_TYPE}")
endif ()

# Optional: Define optimization flags globally for FastDebug
if (CMAKE_BUILD_TYPE STREQUAL "FastDebug")
    message(STATUS ">>> FastDebug mode enabled")

    # defined for logging
    set(CMAKE_CXX_FLAGS_FASTDEBUG "-O2 -g" CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS_FASTDEBUG "-O2 -g" CACHE STRING "" FORCE)

    # Actually apply the flags (target-based or global)
    add_compile_options(-O2 -g)
endif ()

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
                    -ftree-vectorize
                    -funroll-loops
                    -fno-omit-frame-pointer
                    -Wall -Wextra -Wpedantic
            )
            message(STATUS "Release build: enabled aggressive optimization")
        elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
            add_compile_options(
                    -O2
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
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    # Install forwarding header 'pod' (no extension, so must use FILES)
    # Install aggregator header 'pod.h'
    install(FILES
            ${CMAKE_SOURCE_DIR}/include/jh/pod
            ${CMAKE_SOURCE_DIR}/include/jh/pod.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jh
    )

    # Install header directory 'pods/'
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/jh/pods
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jh
    )

    # Install depending headers in directory 'utils/'
    install(FILES
            ${CMAKE_SOURCE_DIR}/include/jh/utils/hash_fn.h
            ${CMAKE_SOURCE_DIR}/include/jh/utils/platform.h
            ${CMAKE_SOURCE_DIR}/include/jh/utils/typed.h
            ${CMAKE_SOURCE_DIR}/include/jh/utils/base64.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jh/utils
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
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    # Define the STATIC library for compiled components (e.g., immutable_str.cpp)
    file(GLOB JH_TOOLKIT_SOURCES CONFIGURE_DEPENDS
            "${CMAKE_SOURCE_DIR}/src/*.cpp"
    )

    add_library(jh-toolkit-impl STATIC ${JH_TOOLKIT_SOURCES})
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

endif ()

# Manually written CMake configuration file (for find_package)
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/jh-toolkit-config-version.cmake"
        VERSION PROJECT_VERSION
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
if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "FastDebug")
    enable_testing()
    add_subdirectory(tests)
    add_subdirectory(examples)

    message(WARNING "Install is disabled in Debug or FastDebug mode. Use Release or RelWithDebInfo instead.")
    add_custom_target(disable-install ALL
            COMMENT "Install disabled in Debug mode"
    )
    function(install)
        message(FATAL_ERROR "Install is explicitly disabled in Debug mode.")
    endfunction()
endif ()
