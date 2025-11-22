# Engine subdirectory CMakeLists.txt

# ---- Python 3.13 manual discovery ----
set(Python3_ROOT_DIR "/usr")
set(Python3_INCLUDE_DIR "/usr/include/python3.13")
set(Python3_LIBRARY "/usr/lib/libpython3.13.so")

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
include_directories(${Python3_INCLUDE_DIR})


# Core library - including all implemented files
add_library(vortex_core STATIC
    core/vulkan_context.cpp
    core/window.cpp
    core/memory_manager.cpp
    renderer/buffer_allocator.cpp
    renderer/shader_system.cpp
    renderer/pipeline_system.cpp
    renderer/command_buffer.cpp
    renderer/synchronization.cpp
)

# Include directories
target_include_directories(vortex_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer
    ${CMAKE_CURRENT_SOURCE_DIR}/ecs
    ${CMAKE_CURRENT_SOURCE_DIR}/scene
    ${CMAKE_CURRENT_SOURCE_DIR}/scripting
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

# Vulkan dependencies
target_link_libraries(vortex_core PUBLIC Vulkan::Vulkan)

# Python dependencies - commented out for now
# target_link_libraries(vortex_core PUBLIC Python3::Python)

# Platform-specific dependencies
if(UNIX AND NOT APPLE)
    target_link_libraries(vortex_core PRIVATE X11)
endif()

# Compiler definitions
target_compile_definitions(vortex_core PRIVATE
    VORTEX_ENGINE_VERSION=1.0.0
    VORTEX_BUILD_ENGINE=1
)

# Set C++ standard
set_property(TARGET vortex_core PROPERTY CXX_STANDARD 20)