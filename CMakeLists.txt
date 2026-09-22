cmake_minimum_required(VERSION 3.20)
set(TARGET context-forge)
project(${TARGET} VERSION 1.0.0 LANGUAGES CXX)

# =========================
# Options
# =========================
option(CONTEXT_FORGE_STABLE_RELEASE "Channel as stable release" OFF)
option(BUILD_TESTS "Build unit tests" OFF)

# =========================
# Compilateur & Standard
# =========================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}) # TARGET destination
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # Moderne compilation -fPIE
add_compile_definitions(BACKWARD_COMPATIBILITY_WARNING) # To check internal usage for potential depreacted function

# =========================
# Requirement
# =========================
find_package(utils 2.13.22 CONFIG REQUIRED) # Check utils dependencies existance
find_package(Python3 REQUIRED) # Used for the special dependencie
#find_package(httplib REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCONFIGXX REQUIRED IMPORTED_TARGET libconfig++)
find_program(CCACHE_PROGRAM ccache) # Used for cache during compilation
if (CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

# =========================
# Warnings
# =========================
add_compile_options(
    -W
    -Wall
    -Wextra
    -Wpedantic
    -Wunused-parameter
    -Wshadow
    -Wuninitialized
)

# =========================
# Sources
# =========================
set(SRC
    # Entry
    src/forge/main.cpp
    src/forge/Forge-init.cpp

    # Global
    src/forge/Forge.cpp

    # Client
    src/forge/Forge-Client.cpp

    # Server
    src/forge/Forge-Server.cpp
    src/forge/Forge-Formating.cpp
    src/forge/Forge-Loading.cpp
    src/forge/Ollama.cpp

    # Rules
    src/forge/rules/Rules.cpp
)
add_executable(${TARGET} ${SRC})

# =========================
# Plugins
# =========================
set(SRC_PLUGINS
    # Nothings...
)

set(SRC_TRIGGERS
    src/forge/rules/triggers/lib.cpp
)

set(SRC_PRE_RULES
    src/forge/rules/pre-rules/lib.cpp
)

set(SRC_RULES
    src/forge/rules/rules/lib.cpp
)

set(SRC_DEFAULT
    src/forge/rules/triggers/default/plugin.cpp
    src/forge/rules/triggers/default/lib.cpp
)
add_library(trigger_default SHARED ${SRC_PLUGINS} ${SRC_TRIGGERS} ${SRC_DEFAULT})

set(SRC_ANSI
    src/forge/rules/pre-rules/ansi/plugin.cpp
    src/forge/rules/pre-rules/ansi/lib.cpp
)
add_library(pre-rule_ansi SHARED ${SRC_PLUGINS} ${SRC_PRE_RULES} ${SRC_ANSI})

set(SRC_LN
    src/forge/rules/rules/ln/plugin.cpp
    src/forge/rules/rules/ln/lib.cpp
)
add_library(rule_ln SHARED ${SRC_PLUGINS} ${SRC_RULES} ${SRC_LN})

# =========================
# Compilation parameters
# =========================

set_target_properties(
    trigger_default
    PROPERTIES
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/plugins/triggers
)

set_target_properties(
    pre-rule_ansi
    PROPERTIES
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/plugins/pre-rules
)

set_target_properties(
    rule_ln
    PROPERTIES
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/plugins/rules
)

set(PLUGINS
    trigger_default
    pre-rule_ansi
    rule_ln
)

# =========================
# Special Targets
# =========================
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

file(GLOB_RECURSE EXCEPTION_CONFIG_FILES
    CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/cmake/config/exceptions/*.json"
)
set(GENERATED_EXCEPTION_HEADER
    "${CMAKE_SOURCE_DIR}/include/exception/generated_external_exception_header.hpp"
)
add_custom_command(
    OUTPUT ${GENERATED_EXCEPTION_HEADER}
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/cmake/scripts/generate_exception_header.py
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    DEPENDS ${EXCEPTION_CONFIG_FILES} ${CMAKE_SOURCE_DIR}/cmake/scripts/generate_exception_header.py
    COMMENT "Generating exception header (config changed)"
    VERBATIM
)
add_custom_target(generated_external_exception_header
    DEPENDS ${GENERATED_EXCEPTION_HEADER}
)

add_custom_target(plugins)
add_dependencies(plugins ${PLUGINS})

# =========================
# Release (build Optimized)
# =========================
add_custom_target(release
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR} -DCMAKE_BUILD_TYPE=Optimized
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --parallel
    COMMENT "Release version: Optimized"
    VERBATIM
)
add_custom_target(package_release
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR} -DCMAKE_BUILD_TYPE=Optimized
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target package --parallel
    COMMENT "Building plackage release version"
    VERBATIM
)

# =========================
# Dependencies
# =========================
target_include_directories(${TARGET} PRIVATE include include/exception)
target_link_libraries(${TARGET} PRIVATE PkgConfig::LIBCONFIGXX utils::utils)
add_dependencies(${TARGET} generated_external_exception_header)
foreach(target ${PLUGINS})
    target_include_directories(${target} PRIVATE include include/exception)
    target_link_libraries(${target} PRIVATE PkgConfig::LIBCONFIGXX utils::utils)
    add_dependencies(${target} generated_external_exception_header)
endforeach()

# =========================
# Modes
# =========================
target_compile_options(${TARGET} PRIVATE
    $<$<CONFIG:Debug>:-g -ggdb3>
    $<$<CONFIG:Asan>:-fsanitize=address -fno-omit-frame-pointer -g -ggdb3>
    $<$<CONFIG:Optimized>:-O3 -ffast-math -march=native -funroll-loops -pipe -DNDEBUG>
)
target_link_options(${TARGET} PRIVATE
    $<$<CONFIG:Asan>:-fsanitize=address>
)
foreach(target ${PLUGINS})
    target_compile_options(${target} PRIVATE
        $<$<CONFIG:Debug>:-g -ggdb3>
        $<$<CONFIG:Asan>:-fsanitize=address -fno-omit-frame-pointer -g -ggdb3>
        $<$<CONFIG:Optimized>:-O3 -ffast-math -march=native -funroll-loops -pipe -DNDEBUG>
    )
    target_link_options(${target} PRIVATE
        $<$<CONFIG:Asan>:-fsanitize=address>
    )
endforeach()

# =========================
# Installation
# =========================
include(GNUInstallDirs) # load variable used for installation

# Binary
install(TARGETS ${TARGET}
    EXPORT ${TARGET}Targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT bin
)

# =========================
# Utils
# =========================
add_custom_target(get_unregistered_files
    COMMAND bash ${CMAKE_SOURCE_DIR}/cmake/scripts/check_files.sh unregistered ${SRC}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# =========================
# Package Building
# =========================
set(CPACK_PACKAGE_VENDOR "TsukiNi22")
set(CPACK_PACKAGE_CONTACT "xartania.contact@gmail.com")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

set(CPACK_GENERATOR "RPM;DEB")

# Setup the channel name
if(CONTEXT_FORGE_STABLE_RELEASE)
    set(CHANNEL "")
    set(MIRROR_CHANNEL "-pre")
else()
    set(CHANNEL "-pre")
    set(MIRROR_CHANNEL "")
endif()

# Setup the package name
set(CPACK_PACKAGE_NAME "context-forge${CHANNEL}")
set(MIRROR_PACKAGE_NAME "context-forge${MIRROR_CHANNEL}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++20 wrapper to format command output")

# Setup the conflict between stable & unstable packages
if(CONTEXT_FORGE_STABLE_RELEASE)
    # If a recent stable version exist the context-forge-*-pre installation will result in the stable version
    set(CPACK_RPM_PACKAGE_OBSOLETES "${MIRROR_PACKAGE_NAME} <= ${PROJECT_VERSION}")
    set(CPACK_DEBIAN_PACKAGE_REPLACES "${MIRROR_PACKAGE_NAME} (<= ${PROJECT_VERSION})")
endif()

set(CPACK_RPM_PACKAGE_CONFLICTS "${MIRROR_PACKAGE_NAME}")
set(CPACK_DEBIAN_PACKAGE_BREAKS "${MIRROR_PACKAGE_NAME}")
set(CPACK_DEBIAN_PACKAGE_CONFLICTS "${MIRROR_PACKAGE_NAME}")

# Separate headers & lib
set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL "")
set(CPACK_COMPONENT_INCLUDE_TOPLEVEL_DIRECTORY OFF) # In case~

# Edit release value (run count)
if(CONTEXT_FORGE_STABLE_RELEASE)
    # A stable version should never have to be rebuild and should always be removed or marked as depreacted
    set(CPACK_RPM_PACKAGE_RELEASE "1")
    set(CPACK_DEBIAN_PACKAGE_RELEASE "1")
elseif(DEFINED PACKAGE_RELEASE)
    set(CPACK_RPM_PACKAGE_RELEASE "${PACKAGE_RELEASE}")
    set(CPACK_DEBIAN_PACKAGE_RELEASE "${PACKAGE_RELEASE}")
else() # 0 is impossible for github.run_number
    set(CPACK_RPM_PACKAGE_RELEASE "0")
    set(CPACK_DEBIAN_PACKAGE_RELEASE "0")
endif()

# Dependencies
set(CPACK_RPM_PACKAGE_REQUIRES "")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "")

# RPM
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_LIB_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
set(CPACK_RPM_HEADERS_PACKAGE_NAME ${CPACK_PACKAGE_NAME})

# Debian
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "TsukiNi22")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/TsukiNi22/context-forge")
set(CPACK_DEBIAN_LIB_PACKAGE_NAME ${CPACK_PACKAGE_NAME})
set(CPACK_DEBIAN_HEADERS_PACKAGE_NAME ${CPACK_PACKAGE_NAME})

include(CPack)
