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

find_package(Threads REQUIRED)

add_executable(test-boost-beast beast.cpp)
target_link_libraries(test-boost-beast boost-beast ${CMAKE_THREAD_LIBS_INIT})

if(NOT MSVC)
    add_executable(test-raw-sockets socks.cpp)
    target_link_libraries(test-raw-sockets ${CMAKE_THREAD_LIBS_INIT})

    add_executable(test-raw-sockets-server socks-server.cpp)
    target_link_libraries(test-raw-sockets-server ${CMAKE_THREAD_LIBS_INIT})
     add_executable(test-raw-sockets-client socks-client.cpp)
    target_link_libraries(test-raw-sockets-client ${CMAKE_THREAD_LIBS_INIT})
endif()

