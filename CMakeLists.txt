cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(network-clog-test)

set(CMAKE_CXX_STANDARD 17)

# cpm
include(./get_cpm.cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

CPMAddPackage(
    NAME boost-cmake
    GITHUB_REPOSITORY iboB/boost-cmake
    GIT_TAG 020d3769fff0eecc5ba3220ecfa85d7ef1b1f136
)
add_library(boost-beast INTERFACE)
target_link_libraries(boost-beast INTERFACE Boost::boost)

set(FISHNETS_SSL OFF)
CPMAddPackage("gh:iboB/fishnets#d5cbab3b83c372adf3af1d6322e7369ca80e3f30")

add_executable(network-clog-test main.cpp)
target_link_libraries(network-clog-test fishnets)

