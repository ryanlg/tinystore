cmake_minimum_required(VERSION 3.25)

project(
    tinystore
    VERSION 0.1.0
    LANGUAGES CXX
)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

# perf
# ---------------
add_executable(
    tsperf_wal_append
    perf/wal_append.cpp
)

target_link_libraries(
    tsperf_wal_append
    PRIVATE
        tinystore
)

target_compile_options(tsperf_wal_append PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -Wconversion
    -O3
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
    "${PROJECT_SOURCE_DIR}/perf/*.cpp"
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

# clang-tidy
# ----------
find_program(CLANG_TIDY_EXE
    NAMES clang-tidy
    REQUIRED
)

file(GLOB_RECURSE TIDY_SOURCES
    CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/*.hpp"
)

file(GLOB_RECURSE TIDY_HEADERS
    CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/include/*.hpp"
)

add_custom_target(tidy-sources
    COMMAND
        "${CLANG_TIDY_EXE}"
        -p "${CMAKE_BINARY_DIR}"
        ${TIDY_SOURCES}
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "Running clang-tidy on sources"
    VERBATIM
)

add_custom_target(tidy-headers
    COMMAND
        "${CLANG_TIDY_EXE}"
        -p "${CMAKE_BINARY_DIR}"
        ${TIDY_HEADERS}
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "Running clang-tidy on headers"
    VERBATIM
)

add_custom_target(tidy)
add_dependencies(tidy tidy-sources tidy-headers)
