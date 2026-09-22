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
set(SRC
)
add_executable(${TARGET} ${SRC})

# =========================
# Dependencies
# =========================
target_include_directories(${TARGET} PRIVATE ../include/ .)
target_link_libraries(${TARGET} PRIVATE GTest::gtest GTest::gtest_main)

# =========================
# Modes
# =========================
target_compile_options(${TARGET} PRIVATE -g -ggdb3)

include(GoogleTest)
gtest_discover_tests(${TARGET})
