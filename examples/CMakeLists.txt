# Main test scene executable
add_executable(test_scene
    test_scene/interactive_main.cpp
)

find_package(Vulkan REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)

target_compile_definitions(test_scene PRIVATE GLFW_INCLUDE_VULKAN)   # ← THIS LINE FIXES IT

target_include_directories(test_scene PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/test_scene
    ${GLFW_INCLUDE_DIRS}
)

target_link_directories(test_scene PRIVATE ${GLFW_LIBRARY_DIRS})

target_link_libraries(test_scene PRIVATE
    vortex_core
    Vulkan::Vulkan
    ${GLFW_LIBRARIES}
)

set_property(TARGET test_scene PROPERTY CXX_STANDARD 20)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(test_scene PRIVATE DEBUG=1)
endif()

# Triangle rendering demo executable
add_executable(triangle_rendering_demo
    test_scene/triangle_rendering_demo.cpp
)

target_compile_definitions(triangle_rendering_demo PRIVATE GLFW_INCLUDE_VULKAN)

target_include_directories(triangle_rendering_demo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/test_scene
    ${GLFW_INCLUDE_DIRS}
)

target_link_directories(triangle_rendering_demo PRIVATE ${GLFW_LIBRARY_DIRS})

target_link_libraries(triangle_rendering_demo PRIVATE
    vortex_core
    Vulkan::Vulkan
    ${GLFW_LIBRARIES}
)

set_property(TARGET triangle_rendering_demo PROPERTY CXX_STANDARD 20)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(triangle_rendering_demo PRIVATE DEBUG=1)
endif()