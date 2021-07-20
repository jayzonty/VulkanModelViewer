cmake_minimum_required(VERSION 3.10)

# Project name
project(VulkanModelViewer)

find_package(Vulkan REQUIRED)

add_subdirectory(deps/glfw3-3-4)
#add_subdirectory(deps/assimp)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

#set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
#set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Generate compile_commands.json for YouCompleteMe (YCM)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Specify include directories
include_directories(${Vulkan_INCLUDE_DIR} deps/glm/include include)

# Set SOURCES to contain all the source files
set(SOURCES
    src/Graphics/Vulkan/VulkanBuffer.cpp
    src/Graphics/Vulkan/VulkanContext.cpp
    src/Graphics/Vulkan/VulkanImage.cpp
    src/Graphics/Vulkan/VulkanImageView.cpp

    src/Input/Input.cpp

    src/Application.cpp
    src/Main.cpp
)

# Executable
add_executable(VulkanModelViewer ${SOURCES})

target_compile_options(VulkanModelViewer PUBLIC -Wall)

# Link libraries
target_link_libraries(VulkanModelViewer ${Vulkan_LIBRARY} glfw)

# Post-build copy command
#add_custom_command(TARGET VulkanModelViewer POST_BUILD
#    COMMAND glslangValidator -S vert -e main -o ${CMAKE_SOURCE_DIR}/resources/shaders/basic_vert.spv -V ${CMAKE_SOURCE_DIR}/resources/shaders/basic_vert.glsl
#    COMMAND glslangValidator -S frag -e main -o ${CMAKE_SOURCE_DIR}/resources/shaders/basic_frag.spv -V ${CMAKE_SOURCE_DIR}/resources/shaders/basic_frag.glsl
#    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources/ $<TARGET_FILE_DIR:VulkanModelViewer>/resources/
#)