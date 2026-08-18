cmake_minimum_required(VERSION 3.25)

project(
    tinystore
    VERSION 0.1.0
    LANGUAGES CXX
)

# tinystore
# ---------
add_library(
    tinystore
    src/backends/inmemory.cpp
    src/byte_codec.cpp
    src/record.cpp
    src/store.cpp
)

# Nix's clang wrapper injects -O2 if hardening is enabled.
target_compile_options(tinystore PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -Wconversion
    "$<$<CONFIG:Debug>:-O0>"
)

target_compile_features(
    tinystore
    PUBLIC cxx_std_23
)

target_include_directories(
    tinystore
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# tinystore_tests
# ---------------
find_package(Catch2 3 REQUIRED)

add_executable(
    tinystore_tests
    tests/byte_codec_test.cpp
    tests/record_test.cpp
    tests/store_test.cpp
)

target_link_libraries(
    tinystore_tests
    PRIVATE
        tinystore
        Catch2::Catch2WithMain
)

target_compile_options(tinystore_tests PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -Wconversion
    "$<$<CONFIG:Debug>:-O0>"
)

enable_testing()

add_test(
    NAME tinystore_tests
    COMMAND tinystore_tests
)

# clang-format
# ------------
find_program(CLANG_FORMAT
    NAMES clang-format
    REQUIRED
)

file(GLOB_RECURSE FORMAT_SOURCES
    CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/*.hpp"
    "${PROJECT_SOURCE_DIR}/include/*.hpp"
    "${PROJECT_SOURCE_DIR}/tests/*.cpp"
    "${PROJECT_SOURCE_DIR}/tests/*.hpp"
)

add_custom_target(format
    COMMAND ${CLANG_FORMAT} -i ${FORMAT_SOURCES}
    COMMENT "Formatting C++ sources"
    VERBATIM
)

add_custom_target(format-check
    COMMAND ${CLANG_FORMAT}
        --dry-run
        --Werror
        ${FORMAT_SOURCES}
    COMMENT "Checking C++ formatting"
    VERBATIM
)
