cmake_minimum_required(VERSION 3.20)
project(VortexEngine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Copy shaders to build directory
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})

# Find required packages
find_package(Vulkan REQUIRED)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
find_package(SDL2 REQUIRED)

# Add subdirectories
add_subdirectory(engine)
add_subdirectory(tools)
add_subdirectory(examples)

# Engine core - defined in engine/CMakeLists.txt
# add_library(vortex_core STATIC ...)  # Commented out - defined in subdirectory

# Examples - will be built by examples/CMakeLists.txt
# add_executable(test_scene examples/test_scene/main.cpp)  # Commented out - defined in subdirectory
# target_link_libraries(test_scene PRIVATE vortex_core)  # Commented out - defined in subdirectory

# Set C++ standard - handled in subdirectories
# set_property(TARGET vortex_core PROPERTY CXX_STANDARD 20)
# set_property(TARGET test_scene PROPERTY CXX_STANDARD 20)

# Compiler warnings - handled in subdirectories
# if(MSVC)
#     target_compile_options(vortex_core PRIVATE /W4)
# else()
#     target_compile_options(vortex_core PRIVATE -Wall -Wextra -Wpedantic)
# endif()

# Debug information - handled in subdirectories
# if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#     target_compile_definitions(vortex_core PRIVATE DEBUG=1)
# endif()

# Platform-specific settings - handled in subdirectories
# if(UNIX AND NOT APPLE)
#     target_link_libraries(vortex_core PRIVATE X11)
# endif()