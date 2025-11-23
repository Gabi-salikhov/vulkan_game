#include "pipeline_system.h"
#include <iostream>
#include <stdexcept>
#include <algorithm>

namespace VortexEngine {

PipelineSystem::PipelineSystem() {
    std::cout << "Initializing pipeline system..." << std::endl;
}

PipelineSystem::~PipelineSystem() {
    shutdown();
}

bool PipelineSystem::initialize(VkDevice device, VkRenderPass renderPass) {
    if (m_initialized) {
        std::cout << "Pipeline system is already initialized" << std::endl;
        return true;
    }

    try {
        m_device = device;
        m_renderPass = renderPass;

        // Initialize pipeline cache if enabled
        if (m_pipelineCacheEnabled) {
            m_pipelineCache = createPipelineCache();
        }

        m_initialized = true;
        std::cout << "Pipeline system initialized successfully" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception during pipeline system initialization: " << e.what() << std::endl;
        return false;
    }
}

void PipelineSystem::shutdown() {
    if (!m_initialized) {
        return;
    }

    std::cout << "Shutting down pipeline system..." << std::endl;

    // Cleanup pipeline cache
    cleanupPipelineCache();

    // Cleanup pipeline states
    destroyAllPipelineStates();

    // Cleanup pipelines
    cleanupPipelines();

    // Cleanup pipeline layouts
    cleanupPipelineLayouts();

    m_initialized = false;
    std::cout << "Pipeline system shutdown complete" << std::endl;
}

VkPipeline PipelineSystem::createGraphicsPipeline(const VkGraphicsPipelineCreateInfo& createInfo) {
    if (!m_initialized) {
        std::cerr << "Pipeline system not initialized" << std::endl;
        return VK_NULL_HANDLE;
    }

    if (!validatePipelineCreateInfo(createInfo)) {
        std::cerr << "Invalid pipeline create info" << std::endl;
        return VK_NULL_HANDLE;
    }

    std::cout << "Creating graphics pipeline..." << std::endl;
    std::cout << "Device: " << m_device << std::endl;
    std::cout << "Cache: " << (m_pipelineCacheEnabled ? m_pipelineCache : VK_NULL_HANDLE) << std::endl;

    // Use pipeline cache if available
    VkPipelineCache cache = m_pipelineCacheEnabled ? m_pipelineCache : VK_NULL_HANDLE;

    VkPipeline pipeline;
    std::cout << "Calling vkCreateGraphicsPipelines with device: " << m_device << std::endl;
    std::cout << "Cache: " << cache << std::endl;
    std::cout << "Create info pointer: " << &createInfo << std::endl;
    
    // Validate shader modules
    for (uint32_t i = 0; i < createInfo.stageCount; i++) {
        std::cout << "Shader stage " << i << ": module=" << createInfo.pStages[i].module
                  << ", stage=" << createInfo.pStages[i].stage
                  << ", name=" << createInfo.pStages[i].pName << std::endl;
    }
    
    VkResult result = vkCreateGraphicsPipelines(m_device, cache, 1, &createInfo, nullptr, &pipeline);
    std::cout << "vkCreateGraphicsPipelines result: " << result << std::endl;
    
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create graphics pipeline: " << result << std::endl;
        
        // Print more detailed error information
        std::cerr << "Error details:" << std::endl;
        std::cerr << "  Device: " << m_device << std::endl;
        std::cerr << "  Cache: " << cache << std::endl;
        std::cerr << "  Stage count: " << createInfo.stageCount << std::endl;
        std::cerr << "  Layout: " << createInfo.layout << std::endl;
        std::cerr << "  Render pass: " << createInfo.renderPass << std::endl;
        
        // Check if we can get more error info
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            std::cerr << "  Error: VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
        } else if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
            std::cerr << "  Error: VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
        } else if (result == VK_ERROR_INITIALIZATION_FAILED) {
            std::cerr << "  Error: VK_ERROR_INITIALIZATION_FAILED" << std::endl;
        } else if (result == VK_ERROR_DEVICE_LOST) {
            std::cerr << "  Error: VK_ERROR_DEVICE_LOST" << std::endl;
        } else if (result == VK_ERROR_EXTENSION_NOT_PRESENT) {
            std::cerr << "  Error: VK_ERROR_EXTENSION_NOT_PRESENT" << std::endl;
        } else if (result == VK_ERROR_FEATURE_NOT_PRESENT) {
            std::cerr << "  Error: VK_ERROR_FEATURE_NOT_PRESENT" << std::endl;
        } else if (result == VK_ERROR_INCOMPATIBLE_DRIVER) {
            std::cerr << "  Error: VK_ERROR_INCOMPATIBLE_DRIVER" << std::endl;
        } else if (result == VK_ERROR_LAYER_NOT_PRESENT) {
            std::cerr << "  Error: VK_ERROR_LAYER_NOT_PRESENT" << std::endl;
        } else if (result == VK_ERROR_FRAGMENTATION) {
            std::cerr << "  Error: VK_ERROR_FRAGMENTATION" << std::endl;
        } else if (result == VK_ERROR_INVALID_SHADER_NV) {
            std::cerr << "  Error: VK_ERROR_INVALID_SHADER_NV" << std::endl;
        } else if (result == VK_ERROR_OUT_OF_POOL_MEMORY) {
            std::cerr << "  Error: VK_ERROR_OUT_OF_POOL_MEMORY" << std::endl;
        } else if (result == VK_ERROR_INVALID_EXTERNAL_HANDLE) {
            std::cerr << "  Error: VK_ERROR_INVALID_EXTERNAL_HANDLE" << std::endl;
        } else if (result == VK_ERROR_FRAGMENTATION_EXT) {
            std::cerr << "  Error: VK_ERROR_FRAGMENTATION_EXT" << std::endl;
        } else if (result == VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS) {
            std::cerr << "  Error: VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS" << std::endl;
        }
        
        return VK_NULL_HANDLE;
    }

    std::cout << "Pipeline created successfully, handle: " << pipeline << std::endl;

    // Store pipeline with generated name
    std::string name = generatePipelineName(createInfo);
    m_pipelines[pipeline] = name;

    std::cout << "Graphics pipeline created: " << name << std::endl;
    return pipeline;
}

void PipelineSystem::destroyPipeline(VkPipeline pipeline) {
    if (!m_initialized || pipeline == VK_NULL_HANDLE) {
        return;
    }

    auto it = m_pipelines.find(pipeline);
    if (it != m_pipelines.end()) {
        vkDestroyPipeline(m_device, pipeline, nullptr);
        m_pipelines.erase(it);
        std::cout << "Pipeline destroyed successfully" << std::endl;
    }
}

VkPipelineLayout PipelineSystem::createPipelineLayout(const VkPipelineLayoutCreateInfo& createInfo) {
    if (!m_initialized) {
        return VK_NULL_HANDLE;
    }

    VkPipelineLayout layout;
    VkResult result = vkCreatePipelineLayout(m_device, &createInfo, nullptr, &layout);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create pipeline layout: " << result << std::endl;
        return VK_NULL_HANDLE;
    }

    // Store layout with generated name
    std::string name = "PipelineLayout_" + std::to_string(m_pipelineLayouts.size());
    m_pipelineLayouts[layout] = name;

    std::cout << "Pipeline layout created: " << name << std::endl;
    return layout;
}

void PipelineSystem::destroyPipelineLayout(VkPipelineLayout layout) {
    if (!m_initialized || layout == VK_NULL_HANDLE) {
        return;
    }

    auto it = m_pipelineLayouts.find(layout);
    if (it != m_pipelineLayouts.end()) {
        vkDestroyPipelineLayout(m_device, layout, nullptr);
        m_pipelineLayouts.erase(it);
        std::cout << "Pipeline layout destroyed successfully" << std::endl;
    }
}

void PipelineSystem::enablePipelineCache(bool enable) {
    m_pipelineCacheEnabled = enable;
    if (enable && m_pipelineCache == VK_NULL_HANDLE) {
        m_pipelineCache = createPipelineCache();
    } else if (!enable && m_pipelineCache != VK_NULL_HANDLE) {
        destroyPipelineCache(m_pipelineCache);
        m_pipelineCache = VK_NULL_HANDLE;
    }
}

VkPipelineCache PipelineSystem::createPipelineCache() {
    if (!m_initialized) {
        return VK_NULL_HANDLE;
    }

    VkPipelineCacheCreateInfo cacheInfo{};
    cacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

    VkPipelineCache cache;
    VkResult result = vkCreatePipelineCache(m_device, &cacheInfo, nullptr, &cache);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create pipeline cache: " << result << std::endl;
        return VK_NULL_HANDLE;
    }

    return cache;
}

void PipelineSystem::destroyPipelineCache(VkPipelineCache cache) {
    if (!m_initialized || cache == VK_NULL_HANDLE) {
        return;
    }

    vkDestroyPipelineCache(m_device, cache, nullptr);
}

VkPipeline PipelineSystem::createDynamicStatePipeline(const VkGraphicsPipelineCreateInfo& createInfo, const std::vector<VkDynamicState>& dynamicStates) {
    if (!m_initialized || dynamicStates.empty()) {
        return VK_NULL_HANDLE;
    }

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // Create a copy of the create info and add dynamic state
    VkGraphicsPipelineCreateInfo modifiedCreateInfo = createInfo;
    modifiedCreateInfo.pDynamicState = &dynamicState;

    return createGraphicsPipeline(modifiedCreateInfo);
}

void PipelineSystem::setDynamicState(VkPipeline pipeline, VkDynamicState state, VkBool32 value) {
    // Dynamic state setting would require pipeline modification or re-creation
    // This is a placeholder for future implementation
    std::cout << "Dynamic state setting not yet implemented" << std::endl;
}

VkPipeline PipelineSystem::createSpecializedPipeline(const VkGraphicsPipelineCreateInfo& createInfo, const PipelineSpecialization& specialization) {
    if (!m_initialized || specialization.data.empty()) {
        return VK_NULL_HANDLE;
    }

    VkPipelineShaderStageCreateInfo modifiedStage = createInfo.pStages[0]; // Assuming first stage
    modifiedStage.pSpecializationInfo = reinterpret_cast<const VkSpecializationInfo*>(1); // Placeholder

    // Create a copy of the create info and add specialization
    VkGraphicsPipelineCreateInfo modifiedCreateInfo = createInfo;
    modifiedCreateInfo.pStages = &modifiedStage;

    return createGraphicsPipeline(modifiedCreateInfo);
}

void PipelineSystem::bindPipeline(VkCommandBuffer commandBuffer, VkPipeline pipeline) {
    if (!m_initialized || pipeline == VK_NULL_HANDLE || commandBuffer == VK_NULL_HANDLE) {
        return;
    }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void PipelineSystem::bindPipelineLayout(VkCommandBuffer commandBuffer, VkPipelineLayout layout) {
    if (!m_initialized || layout == VK_NULL_HANDLE || commandBuffer == VK_NULL_HANDLE) {
        return;
    }

    // Pipeline layout binding is typically done when binding descriptor sets
    // This is a placeholder for future implementation
    std::cout << "Pipeline layout binding not yet fully implemented" << std::endl;
}

bool PipelineSystem::createPipelineState(const std::string& name, const VkGraphicsPipelineCreateInfo& createInfo) {
    if (!m_initialized || name.empty()) {
        return false;
    }

    PipelineState state{};
    state.name = name;

    VkPipeline pipeline = createGraphicsPipeline(createInfo);
    if (pipeline == VK_NULL_HANDLE) {
        return false;
    }

    state.pipeline = pipeline;
    state.layout = createInfo.layout;
    state.dynamicState = (createInfo.pDynamicState != nullptr);

    m_pipelineStates[name] = state;
    return true;
}

bool PipelineSystem::getPipelineState(const std::string& name, PipelineState& state) const {
    if (!m_initialized || name.empty()) {
        return false;
    }

    auto it = m_pipelineStates.find(name);
    if (it != m_pipelineStates.end()) {
        state = it->second;
        return true;
    }

    return false;
}

void PipelineSystem::destroyPipelineState(const std::string& name) {
    if (!m_initialized || name.empty()) {
        return;
    }

    auto it = m_pipelineStates.find(name);
    if (it != m_pipelineStates.end()) {
        if (it->second.pipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(m_device, it->second.pipeline, nullptr);
        }
        m_pipelineStates.erase(it);
        std::cout << "Pipeline state '" << name << "' destroyed successfully" << std::endl;
    }
}

void PipelineSystem::destroyAllPipelineStates() {
    if (!m_initialized) {
        return;
    }

    for (auto& [name, state] : m_pipelineStates) {
        if (state.pipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(m_device, state.pipeline, nullptr);
        }
    }
    m_pipelineStates.clear();

    std::cout << "All pipeline states destroyed successfully" << std::endl;
}

VkPipeline PipelineSystem::createPipelineFromConfig(const PipelineConfig& config) {
    if (!m_initialized) {
        std::cerr << "Pipeline system not initialized" << std::endl;
        return VK_NULL_HANDLE;
    }

    std::cout << "Creating pipeline from config..." << std::endl;
    std::cout << "Vertex shader: " << config.vertexShader << std::endl;
    std::cout << "Fragment shader: " << config.fragmentShader << std::endl;
    std::cout << "Layout: " << config.layout << std::endl;
    std::cout << "Render pass: " << config.renderPass << std::endl;

    // Create a minimal pipeline configuration
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    
    // Shader stages
    VkPipelineShaderStageCreateInfo vertexShaderStage{};
    vertexShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStage.module = config.vertexShader;
    vertexShaderStage.pName = "main";
    
    VkPipelineShaderStageCreateInfo fragmentShaderStage{};
    fragmentShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStage.module = config.fragmentShader;
    fragmentShaderStage.pName = "main";
    
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStage, fragmentShaderStage};
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    
    // Vertex input state - empty for now
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    
    // Input assembly state
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    
    // Viewport state
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    viewportState.pViewports = nullptr; // Will be set dynamically
    viewportState.pScissors = nullptr; // Will be set dynamically
    pipelineInfo.pViewportState = &viewportState;
    
    // Rasterization state
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    pipelineInfo.pRasterizationState = &rasterizer;
    
    // Multisampling state
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipelineInfo.pMultisampleState = &multisampling;
    
    // Depth stencil state - disabled
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;
    pipelineInfo.pDepthStencilState = &depthStencil;
    
    // Color blending state
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    pipelineInfo.pColorBlendState = &colorBlending;
    
    // Dynamic state
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;
    pipelineInfo.pDynamicState = &dynamicState;
    
    // Pipeline layout and render pass
    pipelineInfo.layout = config.layout;
    pipelineInfo.renderPass = config.renderPass;
    pipelineInfo.subpass = 0;
    
    std::cout << "Pipeline info created, calling createGraphicsPipeline..." << std::endl;

    VkPipeline result = createGraphicsPipeline(pipelineInfo);
    std::cout << "Pipeline creation result: " << result << std::endl;
    return result;
}

VkPipelineLayout PipelineSystem::createPipelineLayoutFromConfig(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, const std::vector<VkPushConstantRange>& pushConstants) {
    if (!m_initialized) {
        return VK_NULL_HANDLE;
    }

    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    layoutInfo.pSetLayouts = descriptorSetLayouts.data();
    layoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
    layoutInfo.pPushConstantRanges = pushConstants.data();

    return createPipelineLayout(layoutInfo);
}

void PipelineSystem::printPipelineInfo() const {
    std::cout << "Pipeline System Info:" << std::endl;
    std::cout << "  Pipeline Count: " << m_pipelines.size() << std::endl;
    std::cout << "  Pipeline Layout Count: " << m_pipelineLayouts.size() << std::endl;
    std::cout << "  Pipeline State Count: " << m_pipelineStates.size() << std::endl;
    std::cout << "  Pipeline Cache: " << (m_pipelineCacheEnabled ? "Enabled" : "Disabled") << std::endl;

    for (const auto& [pipeline, name] : m_pipelines) {
        std::cout << "  - Pipeline: " << name 
                  << " (Handle: " << pipeline << ")" << std::endl;
    }

    for (const auto& [layout, name] : m_pipelineLayouts) {
        std::cout << "  - Layout: " << name 
                  << " (Handle: " << layout << ")" << std::endl;
    }

    for (const auto& [name, state] : m_pipelineStates) {
        std::cout << "  - State: " << name 
                  << " (Pipeline: " << state.pipeline 
                  << ", Layout: " << state.layout 
                  << ", Dynamic: " << (state.dynamicState ? "Yes" : "No") << ")" << std::endl;
    }
}

// Private implementation methods
void PipelineSystem::cleanupPipelines() {
    for (auto& [pipeline, name] : m_pipelines) {
        if (pipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(m_device, pipeline, nullptr);
        }
    }
    m_pipelines.clear();
}

void PipelineSystem::cleanupPipelineLayouts() {
    for (auto& [layout, name] : m_pipelineLayouts) {
        if (layout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(m_device, layout, nullptr);
        }
    }
    m_pipelineLayouts.clear();
}

void PipelineSystem::cleanupPipelineCache() {
    if (m_pipelineCache != VK_NULL_HANDLE) {
        vkDestroyPipelineCache(m_device, m_pipelineCache, nullptr);
        m_pipelineCache = VK_NULL_HANDLE;
    }
}

std::string PipelineSystem::generatePipelineName(const VkGraphicsPipelineCreateInfo& createInfo) const {
    std::string name = "Pipeline_";
    
    // Add shader stages info
    for (uint32_t i = 0; i < createInfo.stageCount; ++i) {
        name += std::to_string(createInfo.pStages[i].stage) + "_";
    }
    
    // Add layout and render pass info
    name += "L" + std::to_string(reinterpret_cast<uint64_t>(createInfo.layout)) + "_";
    name += "R" + std::to_string(reinterpret_cast<uint64_t>(createInfo.renderPass));
    
    return name;
}

bool PipelineSystem::validatePipelineCreateInfo(const VkGraphicsPipelineCreateInfo& createInfo) const {
    std::cout << "Validating pipeline create info..." << std::endl;
    std::cout << "sType: " << createInfo.sType << std::endl;
    std::cout << "stageCount: " << createInfo.stageCount << std::endl;
    std::cout << "layout: " << createInfo.layout << std::endl;
    std::cout << "renderPass: " << createInfo.renderPass << std::endl;
    
    if (createInfo.sType != VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO) {
        std::cerr << "Invalid sType: " << createInfo.sType << std::endl;
        return false;
    }
    
    if (createInfo.stageCount == 0) {
        std::cerr << "No shader stages" << std::endl;
        return false;
    }
    
    if (createInfo.layout == VK_NULL_HANDLE) {
        std::cerr << "Null layout" << std::endl;
        return false;
    }
    
    if (createInfo.renderPass == VK_NULL_HANDLE) {
        std::cerr << "Null render pass" << std::endl;
        return false;
    }
    
    std::cout << "Pipeline create info is valid" << std::endl;
    return true;
}

} // namespace VortexEngine