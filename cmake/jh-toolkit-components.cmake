if (POLICY CMP0177)
    cmake_policy(SET CMP0177 NEW)
endif ()

set(JH_TOOLKIT_COMPONENTS "all" CACHE STRING
    "Semicolon-separated components: base, jh-no-throw, jh-test-enabled, jh-ipcs, all")
set_property(CACHE JH_TOOLKIT_COMPONENTS PROPERTY STRINGS
    base jh-no-throw jh-test-enabled jh-ipcs all)
option(JH_TOOLKIT_CLEAN_INSTALL
    "Remove JH-Toolkit files previously recorded in this install prefix before installing"
    OFF)

set(_jh_valid_components base jh-no-throw jh-test-enabled jh-ipcs all)
set(_jh_requested_components "${JH_TOOLKIT_COMPONENTS}")
string(REPLACE "," ";" _jh_requested_components "${_jh_requested_components}")
list(TRANSFORM _jh_requested_components TOLOWER)
list(REMOVE_DUPLICATES _jh_requested_components)

if (NOT _jh_requested_components)
    message(FATAL_ERROR "JH_TOOLKIT_COMPONENTS must select at least one component")
endif ()
foreach (_jh_component IN LISTS _jh_requested_components)
    if (NOT _jh_component IN_LIST _jh_valid_components)
        message(FATAL_ERROR
            "Unknown JH-Toolkit component '${_jh_component}'. "
            "Supported components: ${_jh_valid_components}")
    endif ()
endforeach ()

set(_jh_has_all FALSE)
set(_jh_has_base FALSE)
set(_jh_has_no_throw FALSE)
set(_jh_has_test_enabled FALSE)
set(_jh_has_ipcs FALSE)
if ("all" IN_LIST _jh_requested_components)
    set(_jh_has_all TRUE)
    set(_jh_has_base TRUE)
    set(_jh_has_no_throw TRUE)
    set(_jh_has_test_enabled TRUE)
    set(_jh_has_ipcs TRUE)
else ()
    foreach (_jh_component IN LISTS _jh_requested_components)
        if (_jh_component STREQUAL "base")
            set(_jh_has_base TRUE)
        elseif (_jh_component STREQUAL "jh-no-throw")
            set(_jh_has_no_throw TRUE)
        elseif (_jh_component STREQUAL "jh-test-enabled")
            set(_jh_has_test_enabled TRUE)
        elseif (_jh_component STREQUAL "jh-ipcs")
            set(_jh_has_ipcs TRUE)
        endif ()
    endforeach ()
endif ()

set(JH_TOOLKIT_ENABLE_IPCS ${_jh_has_ipcs})

function(_jh_collect_header_closure output)
    set(_jh_pending ${ARGN})
    set(_jh_collected "")

    while (_jh_pending)
        list(POP_FRONT _jh_pending _jh_current)
        if (IS_ABSOLUTE "${_jh_current}")
            set(_jh_current_path "${_jh_current}")
        else ()
            set(_jh_current_path "${PROJECT_SOURCE_DIR}/include/${_jh_current}")
        endif ()
        cmake_path(NORMAL_PATH _jh_current_path)

        if (NOT EXISTS "${_jh_current_path}")
            message(FATAL_ERROR "Required JH-Toolkit header is missing: ${_jh_current_path}")
        endif ()
        if (_jh_current_path IN_LIST _jh_collected)
            continue()
        endif ()
        list(APPEND _jh_collected "${_jh_current_path}")

        file(STRINGS "${_jh_current_path}" _jh_include_lines
            REGEX "^[ ]*#[ ]*include[ ]*[<\"]")
        get_filename_component(_jh_current_dir "${_jh_current_path}" DIRECTORY)
        foreach (_jh_include_line IN LISTS _jh_include_lines)
            string(REGEX MATCH "^[ ]*#[ ]*include[ ]*[<\"]([^>\"]+)[>\"]"
                _jh_include_match "${_jh_include_line}")
            if (NOT _jh_include_match)
                continue()
            endif ()

            set(_jh_include_path "${CMAKE_MATCH_1}")
            if (_jh_include_path MATCHES "^jh/")
                set(_jh_dependency "${PROJECT_SOURCE_DIR}/include/${_jh_include_path}")
            else ()
                set(_jh_dependency "${_jh_current_dir}/${_jh_include_path}")
            endif ()
            if (EXISTS "${_jh_dependency}")
                list(APPEND _jh_pending "${_jh_dependency}")
            endif ()
        endforeach ()
    endwhile ()

    set(${output} "${_jh_collected}" PARENT_SCOPE)
endfunction()

function(_jh_relative_header_list output)
    set(_jh_relative_headers "")
    foreach (_jh_header IN LISTS ARGN)
        file(RELATIVE_PATH _jh_relative_header
            "${PROJECT_SOURCE_DIR}/include" "${_jh_header}")
        list(APPEND _jh_relative_headers "${_jh_relative_header}")
    endforeach ()
    set(${output} "${_jh_relative_headers}" PARENT_SCOPE)
endfunction()

function(_jh_filter_toolkit_headers output)
    set(_jh_filtered_headers "")
    foreach (_jh_header IN LISTS ARGN)
        file(RELATIVE_PATH _jh_relative_header
            "${PROJECT_SOURCE_DIR}/include" "${_jh_header}")
        if (_jh_relative_header MATCHES "^jh/")
            list(APPEND _jh_filtered_headers "${_jh_header}")
        endif ()
    endforeach ()
    set(${output} "${_jh_filtered_headers}" PARENT_SCOPE)
endfunction()

function(_jh_install_header_files)
    foreach (_jh_header IN LISTS ARGN)
        if (IS_ABSOLUTE "${_jh_header}")
            set(_jh_header_path "${_jh_header}")
        else ()
            set(_jh_header_path "${PROJECT_SOURCE_DIR}/include/${_jh_header}")
        endif ()
        file(RELATIVE_PATH _jh_relative_header
            "${PROJECT_SOURCE_DIR}/include" "${_jh_header_path}")
        get_filename_component(_jh_header_directory "${_jh_relative_header}" DIRECTORY)
        install(FILES "${_jh_header_path}"
            DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${_jh_header_directory}")
    endforeach ()
endfunction()

_jh_collect_header_closure(_jh_no_throw_headers
    jh/meta jh/pod jh/typing/monostate.h)
file(GLOB_RECURSE _jh_test_package_headers
    LIST_DIRECTORIES FALSE
    CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/tiny-test/include/jh/test/tiny_test/*.hpp"
    "${PROJECT_SOURCE_DIR}/simple-cucumber/include/jh/test/cucumber/*.hpp"
    "${PROJECT_SOURCE_DIR}/simple-cucumber/gherkin/include/jh/test/cucumber/*.hpp")
_jh_collect_header_closure(_jh_test_headers
    jh/metax/expected.h jh/metax/t_str.h ${_jh_test_package_headers})
_jh_filter_toolkit_headers(_jh_test_headers ${_jh_test_headers})
_jh_collect_header_closure(_jh_ipc_headers jh/sync jh/ipc)
_jh_relative_header_list(JH_TOOLKIT_NO_THROW_HEADERS ${_jh_no_throw_headers})
_jh_relative_header_list(JH_TOOLKIT_TEST_HEADERS ${_jh_test_headers})
_jh_relative_header_list(JH_TOOLKIT_IPC_HEADERS ${_jh_ipc_headers})

file(GLOB_RECURSE _jh_source_headers
    LIST_DIRECTORIES FALSE
    RELATIVE "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/include/jh/*")
list(FILTER _jh_source_headers EXCLUDE REGEX "(^|/)\\.DS_Store$")
set(JH_TOOLKIT_ALL_HEADERS ${_jh_source_headers})
set(JH_TOOLKIT_BASE_HEADERS "")
foreach (_jh_header IN LISTS _jh_source_headers)
    if (_jh_header STREQUAL "jh/ipc"
            OR _jh_header STREQUAL "jh/synchronous/ipc.h"
            OR _jh_header MATCHES "^jh/synchronous/ipc/")
        continue()
    endif ()
    list(APPEND JH_TOOLKIT_BASE_HEADERS "${_jh_header}")
endforeach ()
list(REMOVE_DUPLICATES JH_TOOLKIT_ALL_HEADERS)
list(REMOVE_DUPLICATES JH_TOOLKIT_BASE_HEADERS)

add_library(jh-toolkit INTERFACE)
add_library(jh::jh-toolkit ALIAS jh-toolkit)
target_compile_features(jh-toolkit INTERFACE cxx_std_20)
target_include_directories(jh-toolkit
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
if (_jh_has_ipcs)
    target_compile_definitions(jh-toolkit INTERFACE
        "$<BUILD_INTERFACE:JH_TOOLKIT_ENABLE_IPCS=1>")
endif ()

set(JH_TOOLKIT_AVAILABLE_COMPONENTS base jh-no-throw jh-test-enabled)
if (_jh_has_ipcs)
    list(APPEND JH_TOOLKIT_AVAILABLE_COMPONENTS jh-ipcs all)
endif ()
list(REMOVE_DUPLICATES JH_TOOLKIT_AVAILABLE_COMPONENTS)
set_property(TARGET jh-toolkit PROPERTY
    JH_TOOLKIT_AVAILABLE_COMPONENTS "${JH_TOOLKIT_AVAILABLE_COMPONENTS}")
message(STATUS "JH-Toolkit components: ${_jh_requested_components}")

if (_jh_has_all OR (_jh_has_base AND _jh_has_ipcs))
    set(JH_TOOLKIT_CURRENT_HEADERS ${JH_TOOLKIT_ALL_HEADERS})
elseif (_jh_has_base)
    set(JH_TOOLKIT_CURRENT_HEADERS ${JH_TOOLKIT_BASE_HEADERS})
else ()
    set(_jh_selected_headers "")
    if (_jh_has_no_throw)
        list(APPEND _jh_selected_headers ${_jh_no_throw_headers})
    endif ()
    if (_jh_has_test_enabled)
        list(APPEND _jh_selected_headers ${_jh_test_headers})
    endif ()
    if (_jh_has_ipcs)
        list(APPEND _jh_selected_headers ${_jh_ipc_headers})
    endif ()
    list(REMOVE_DUPLICATES _jh_selected_headers)
    _jh_relative_header_list(JH_TOOLKIT_CURRENT_HEADERS ${_jh_selected_headers})
endif ()

configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/jh-toolkit-pre-install.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/jh-toolkit-pre-install.cmake"
    @ONLY)
install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/jh-toolkit-pre-install.cmake")

if (_jh_has_all OR (_jh_has_base AND _jh_has_ipcs))
    install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        PATTERN ".DS_Store" EXCLUDE)
elseif (_jh_has_base)
    _jh_install_header_files(${JH_TOOLKIT_BASE_HEADERS})
else ()
    if (_jh_selected_headers)
        _jh_install_header_files(${_jh_selected_headers})
    endif ()
endif ()

if (_jh_has_base)
    file(GLOB JH_TOOLKIT_SOURCES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")
    add_library(jh-toolkit-static STATIC ${JH_TOOLKIT_SOURCES})
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(jh-toolkit-static PRIVATE -fno-rtti)
    elseif (MSVC)
        target_compile_options(jh-toolkit-static PRIVATE /GR-)
    endif ()
    target_include_directories(jh-toolkit-static PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
    target_link_libraries(jh-toolkit-static PUBLIC jh-toolkit)
    target_compile_definitions(jh-toolkit-static
        PRIVATE JH_IS_STATIC_BUILD
        INTERFACE JH_HEADER_NO_IMPL)
    set_target_properties(jh-toolkit-static PROPERTIES EXPORT_COMPILE_DEFINITIONS ON)
endif ()

install(TARGETS jh-toolkit EXPORT jh-toolkitTargets)
install(EXPORT jh-toolkitTargets
    FILE jh-toolkitTargets.cmake
    NAMESPACE jh::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/jh-toolkit")

if (TARGET jh-toolkit-static)
    install(TARGETS jh-toolkit-static EXPORT jh-toolkit-staticTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
    install(EXPORT jh-toolkit-staticTargets
        FILE jh-toolkit-staticTargets.cmake
        NAMESPACE jh::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/jh-toolkit")
endif ()

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/jh-toolkit-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/jh-toolkit-config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/jh-toolkit")
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/jh-toolkit-config-version.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion)
configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/jh-toolkit-post-install.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/jh-toolkit-post-install.cmake"
    @ONLY)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/jh-toolkit-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/jh-toolkit-config-version.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/jh-toolkit")
install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/jh-toolkit-post-install.cmake")

if (JH_ENABLE_TESTING)
    enable_testing()
    if (_jh_has_base)
        add_subdirectory(tests)
    else ()
        message(STATUS "Skipping toolkit tests because the base component is not selected")
    endif ()
endif ()

if (JH_ENABLE_EXAMPLES)
    if (_jh_has_base)
        add_subdirectory(examples)
    else ()
        message(FATAL_ERROR "JH_ENABLE_EXAMPLES requires the base component")
    endif ()
endif ()
