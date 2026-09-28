cmake_minimum_required(VERSION 3.21)
project(jh-toolkit LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)  # Ensure standardized installation directories

set(PROJECT_VERSION 1.4.1)

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
#    normally overrides and enforces its global compilation strategy.
#
#    This behavior is deliberate:
#    Using FetchContent/add_subdirectory implies adoption of the project's
#    architectural philosophy and compilation discipline.
#
#    However, this behavior can be adjusted:
#
#    - Setting USE_JH_COMPILE_OPT=OFF disables jh-toolkit's compile option
#      enforcement when used as a subproject.
#
#    Consumers are expected to audit their codebase under this philosophy,
#    including the enforced optimization strategy, unless explicitly opting
#    out through the above option. RTTI is disabled privately for toolkit-owned
#    compiled targets and is not propagated to consumers.
#
#    If full independence from these policies is desired, the library may
#    alternatively be consumed via find_package.
#    Compiler and Architecture Optimization
#
#    This project currently targets Clang and GCC toolchains (or compatible
#    variants). Because these compilers expose consistent feature detection
#    behavior, jh-toolkit is able to reliably detect the compilation
#    environment and automatically select an appropriate -march optimization
#    strategy.
#
#    For native builds, the project prefers `-march=native` when supported.
#    For cross-compilation, a conservative architecture baseline is selected
#    based on `CMAKE_SYSTEM_PROCESSOR`.
#
#    This automatic architecture tuning is part of the compile-option
#    enforcement described above and is intended to provide optimal
#    performance without requiring manual configuration.
# ------------------------------------------------------------------------------

# Options

option(USE_JH_COMPILE_OPT
        "Allow jh-toolkit to enforce compile optimization flags when used via add_subdirectory/FetchContent"
        ON)

set(_jh_enable_testing_default OFF)
if (PROJECT_IS_TOP_LEVEL)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR
            CMAKE_BUILD_TYPE STREQUAL "FastDebug")
        set(_jh_enable_testing_default ON)
    endif ()
endif ()
option(JH_ENABLE_TESTING
        "Build jh-toolkit tests"
        "${_jh_enable_testing_default}")
unset(_jh_enable_testing_default)

option(JH_ENABLE_EXAMPLES
        "Build jh-toolkit examples"
        OFF)

set(JH_APPLY_COMPILE_OPT TRUE)

# If used as subproject and user disables compile options
if (NOT PROJECT_IS_TOP_LEVEL AND NOT USE_JH_COMPILE_OPT)
    set(JH_APPLY_COMPILE_OPT FALSE)
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

if (JH_APPLY_COMPILE_OPT)

    set(CMAKE_CXX_FLAGS_FASTDEBUG "-O2 -g1 -Wall -Wextra -Wpedantic")
    set(CMAKE_C_FLAGS_FASTDEBUG "-O2 -g1 -Wall -Wextra -Wpedantic")

    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(
                -O0 -g
                -Wall -Wextra -Wpedantic
        )
        message(STATUS "Debug build: light optimization with warnings")

    elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
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

        endif () # CMAKE_CXX_COMPILER_ID

        # Apply optimization flags based on build type
        add_compile_options(
                ${EXTRA_OPT_FLAGS}
                -O3
                -ftree-vectorize
                -funroll-loops
                -fno-omit-frame-pointer
                -Wall -Wextra -Wpedantic
        )
        message(STATUS "Release build: enabled aggressive optimization")

    endif () # CMAKE_BUILD_TYPE

endif () # JH_APPLY_COMPILE_OPT

string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_UPPER)
message(STATUS "${CMAKE_BUILD_TYPE} CXX flags: ${CMAKE_CXX_FLAGS_${BUILD_TYPE_UPPER}}")

include(cmake/jh-toolkit-components.cmake)
