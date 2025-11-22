# Tools subdirectory CMakeLists.txt

# Shader compiler tool - placeholder for now
# add_executable(shader_compiler
#     shader_compiler/main.cpp
#     shader_compiler/glsl_to_spirv.cpp
#     shader_compiler/compiler_utils.cpp
# )

# target_include_directories(shader_compiler PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}/..
#     ${CMAKE_CURRENT_SOURCE_DIR}/shader_compiler
# )

# target_link_libraries(shader_compiler PRIVATE
#     vortex_core
#     Vulkan::Vulkan
# )

# Texture processor tool - placeholder for now
# add_executable(texture_processor
#     texture_processor/main.cpp
#     texture_processor/image_loader.cpp
#     texture_processor/texture_optimizer.cpp
# )

# target_include_directories(texture_processor PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}/..
#     ${CMAKE_CURRENT_SOURCE_DIR}/texture_processor
# )

# target_link_libraries(texture_processor PRIVATE
#     vortex_core
# )

# Model converter tool - placeholder for now
# add_executable(model_converter
#     model_converter/main.cpp
#     model_converter/gltf_loader.cpp
#     model_converter/mesh_processor.cpp
# )

# target_include_directories(model_converter PRIVATE
#     ${CMAKE_CURRENT_DIR}/..
#     ${CMAKE_CURRENT_DIR}/model_converter
# )

# target_link_libraries(model_converter PRIVATE
#     vortex_core
#     tinygltf
# )

# Build scripts
# add_subdirectory(build_scripts)  # Commented out - directory not yet created