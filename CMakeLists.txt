cmake_minimum_required(VERSION 3.22)
project(Graphics)

set(CMAKE_CXX_STANDARD 14)

add_executable(Graphics main.cpp)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(Graphics PUBLIC -DDEBUG)
endif()

target_link_libraries(Graphics dxgi d3d12)