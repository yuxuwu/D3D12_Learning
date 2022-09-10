cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 14)

add_executable(Graphics12 WIN32 main.cpp)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(Graphics12 PUBLIC -DDEBUG)
endif()

target_link_libraries(Graphics12 dxgi d3d12)

