cmake_minimum_required(VERSION 3.22)
project(Graphics)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_FLAGS -m64)
set(CMAKE_CXX_FLAGS -m64)


find_package(directxtk 1.8.2 CONFIG REQUIRED)
find_package(directxmath CONFIG REQUIRED)

find_package(imgui CONFIG REQUIRED)

add_custom_target(copy_shaders ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:BoxGraphics>/assets
        COMMENT "Copying shaders" VERBATIM)

############################################################
#  BoxGraphics app
############################################################
add_executable(BoxGraphics WIN32 BoxGraphics/BoxGraphics.cpp Graphics.cpp Graphics.h)
target_link_libraries(BoxGraphics PRIVATE Microsoft::DirectXTK)
target_link_libraries(BoxGraphics PRIVATE imgui::imgui)
add_dependencies(BoxGraphics copy_shaders)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(BoxGraphics PUBLIC -DDEBUG)
endif()


############################################################
#  GridGraphics app
############################################################
add_executable(GridGraphics WIN32 GridGraphics/GridGraphics.cpp Graphics.cpp Graphics.h)
target_link_libraries(GridGraphics PRIVATE Microsoft::DirectXTK)
target_link_libraries(GridGraphics PRIVATE imgui::imgui)
add_dependencies(GridGraphics copy_shaders)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(GridGraphics PUBLIC -DDEBUG)
endif()
