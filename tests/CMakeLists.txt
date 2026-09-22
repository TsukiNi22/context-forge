set(TARGET unit_tests)

# =========================
# Compilateur & Standard
# =========================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}) # TARGET destination

# =========================
# Requirement
# =========================
find_package(GTest REQUIRED)

# =========================
# Sources
# =========================
# Project sources (reuse the parent list without the entry point)
set(SRC_FORGE ${SRC})
list(REMOVE_ITEM SRC_FORGE src/forge/main.cpp)
list(TRANSFORM SRC_FORGE PREPEND ${CMAKE_SOURCE_DIR}/)

# Plugins class (only the class, the extern "C" interface is tested through the `plugins` target)
set(SRC_FORGE_PLUGINS
    ${CMAKE_SOURCE_DIR}/src/forge/rules/triggers/default/plugin.cpp
    ${CMAKE_SOURCE_DIR}/src/forge/rules/pre-rules/ansi/plugin.cpp
    ${CMAKE_SOURCE_DIR}/src/forge/rules/rules/ln/plugin.cpp
)

set(SRC
    # Forge
    Forge.cpp
    Ollama.cpp

    # Rules
    rules/AInstruction.cpp
    rules/Rules.cpp
)
add_executable(${TARGET} ${SRC} ${SRC_FORGE} ${SRC_FORGE_PLUGINS})

# =========================
# Dependencies
# =========================
target_include_directories(${TARGET} PRIVATE ../include/ ../include/exception/ .)
target_link_libraries(${TARGET} PRIVATE PkgConfig::LIBCONFIGXX httplib::httplib utils::utils GTest::gtest GTest::gtest_main)
add_dependencies(${TARGET} generated_external_exception_header plugins)

# Path used by the tests to find the shared object built by the `plugins` target
target_compile_definitions(${TARGET} PRIVATE TESTS_PLUGINS_DIR="${CMAKE_SOURCE_DIR}/plugins")

# =========================
# Modes
# =========================
target_compile_options(${TARGET} PRIVATE -g -ggdb3)

include(GoogleTest)
gtest_discover_tests(${TARGET})
