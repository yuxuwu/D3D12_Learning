cmake_minimum_required(VERSION 3.22)
project(Graphics)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_FLAGS -m64)
set(CMAKE_CXX_FLAGS -m64)

add_executable(Graphics11 WIN32 main.cpp Graphics.cpp Graphics.h)
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(Graphics11 PUBLIC -DDEBUG)
endif()

add_custom_target(copy_shaders ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:Graphics11>/assets
        COMMENT "Copying shaders" VERBATIM)
add_dependencies(Graphics11 copy_shaders)


set(directxtk_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/installed/x64-windows/share/directxtk)
set(directxmath_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/installed/x64-windows/share/directxmath)
find_package(directxtk 1.8.2 CONFIG REQUIRED)
find_package(directxmath CONFIG REQUIRED)
target_link_libraries(Graphics11 PRIVATE Microsoft::DirectXTK)

