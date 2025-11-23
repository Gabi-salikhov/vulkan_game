# Main test scene executable
add_executable(test_scene
    test_scene/interactive_main.cpp
)

find_package(Vulkan REQUIRED)
find_package(SDL2 REQUIRED)

target_include_directories(test_scene PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/test_scene
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(test_scene PRIVATE
    vortex_core
    Vulkan::Vulkan
    SDL2::SDL2
)

set_property(TARGET test_scene PROPERTY CXX_STANDARD 20)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(test_scene PRIVATE DEBUG=1)
endif()

# Triangle rendering demo executable
add_executable(triangle_rendering_demo
    test_scene/triangle_rendering_demo.cpp
)

target_include_directories(triangle_rendering_demo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/test_scene
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(triangle_rendering_demo PRIVATE
    vortex_core
    Vulkan::Vulkan
    SDL2::SDL2
)

set_property(TARGET triangle_rendering_demo PROPERTY CXX_STANDARD 20)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(triangle_rendering_demo PRIVATE DEBUG=1)
endif()