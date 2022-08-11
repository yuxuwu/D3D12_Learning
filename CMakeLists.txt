cmake_minimum_required(VERSION 3.22)
project(Graphics)

set(CMAKE_CXX_STANDARD 14)

add_executable(Graphics12 d3d12/main.cpp d3d11/main.cpp)
add_executable(Graphics11 d3d11/main.cpp)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(Graphics12 PUBLIC -DDEBUG)
    target_compile_definitions(Graphics11 PUBLIC -DDEBUG)
endif()

target_link_libraries(Graphics12 dxgi d3d12)
target_link_libraries(Graphics11 d3d11)