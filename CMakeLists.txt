cmake_minimum_required(VERSION 3.25)

project(
    tinystore
    VERSION 0.1.0
    LANGUAGES CXX
)

add_library(
    tinystore
    src/store.cpp
    src/backends/inmemory.cpp
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
)

# Nix's clang wrapper injects -O2 if hardening is enabled.
target_compile_options(tinystore PRIVATE
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

find_package(Catch2 3 REQUIRED)

add_executable(
    tinystore_tests
    tests/store_test.cpp
)

target_link_libraries(
    tinystore_tests
    PRIVATE
        tinystore
        Catch2::Catch2WithMain
)

target_compile_options(tinystore_tests PRIVATE
    "$<$<CONFIG:Debug>:-O0>"
)

enable_testing()

add_test(
    NAME tinystore_tests
    COMMAND tinystore_tests
)
