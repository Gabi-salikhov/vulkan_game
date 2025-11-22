#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>

namespace VortexEngine {

class PipelineSystem {
public:
    PipelineSystem();
    ~PipelineSystem();

    // Pipeline system lifecycle
    bool initialize(VkDevice device, VkRenderPass renderPass);
    void shutdown();

    // Graphics pipeline creation
    VkPipeline createGraphicsPipeline(const VkGraphicsPipelineCreateInfo& createInfo);
    void destroyPipeline(VkPipeline pipeline);

    // Pipeline layout management
    VkPipelineLayout createPipelineLayout(const VkPipelineLayoutCreateInfo& createInfo);
    void destroyPipelineLayout(VkPipelineLayout layout);

    // Render pass management
    void setRenderPass(VkRenderPass renderPass) { m_renderPass = renderPass; }
    VkRenderPass getRenderPass() const { return m_renderPass; }

    // Pipeline cache
    void enablePipelineCache(bool enable);
    bool isPipelineCacheEnabled() const { return m_pipelineCacheEnabled; }
    VkPipelineCache createPipelineCache();
    void destroyPipelineCache(VkPipelineCache cache);

    // Dynamic state
    VkPipeline createDynamicStatePipeline(const VkGraphicsPipelineCreateInfo& createInfo, const std::vector<VkDynamicState>& dynamicStates);
    void setDynamicState(VkPipeline pipeline, VkDynamicState state, VkBool32 value);

    // Pipeline specialization
    struct PipelineSpecialization {
        std::vector<VkSpecializationMapEntry> mapEntries;
        std::vector<uint8_t> data;
    };

    VkPipeline createSpecializedPipeline(const VkGraphicsPipelineCreateInfo& createInfo, const PipelineSpecialization& specialization);

    // Pipeline binding
    void bindPipeline(VkCommandBuffer commandBuffer, VkPipeline pipeline);
    void bindPipelineLayout(VkCommandBuffer commandBuffer, VkPipelineLayout layout);

    // Pipeline state objects (PSO) management
    struct PipelineState {
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout layout = VK_NULL_HANDLE;
        std::string name;
        bool dynamicState = false;
    };

    bool createPipelineState(const std::string& name, const VkGraphicsPipelineCreateInfo& createInfo);
    bool getPipelineState(const std::string& name, PipelineState& state) const;
    void destroyPipelineState(const std::string& name);
    void destroyAllPipelineStates();

    // Pipeline configuration helpers
    struct PipelineConfig {
        VkShaderModule vertexShader;
        VkShaderModule fragmentShader;
        VkPipelineLayout layout;
        VkRenderPass renderPass;
        uint32_t subpass = 0;
        std::vector<VkVertexInputBindingDescription> vertexBindings;
        std::vector<VkVertexInputAttributeDescription> vertexAttributes;
        std::vector<VkDynamicState> dynamicStates;
        VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
        VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
        VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE;
        float lineWidth = 1.0f;
        bool depthTest = true;
        bool depthWrite = true;
        VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS;
        bool blendEnable = false;
        VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    };

    VkPipeline createPipelineFromConfig(const PipelineConfig& config);
    VkPipelineLayout createPipelineLayoutFromConfig(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, const std::vector<VkPushConstantRange>& pushConstants);

    // Debug information
    void printPipelineInfo() const;
    uint32_t getPipelineCount() const { return m_pipelines.size(); }
    uint32_t getPipelineLayoutCount() const { return m_pipelineLayouts.size(); }

private:
    // Vulkan objects
    VkDevice m_device = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;

    // Pipeline storage
    std::unordered_map<VkPipeline, std::string> m_pipelines;
    std::unordered_map<VkPipelineLayout, std::string> m_pipelineLayouts;
    std::unordered_map<std::string, PipelineState> m_pipelineStates;

    // Pipeline cache
    VkPipelineCache m_pipelineCache = VK_NULL_HANDLE;
    bool m_pipelineCacheEnabled = false;

    // Configuration
    bool m_initialized = false;

    // Internal methods
    void cleanupPipelines();
    void cleanupPipelineLayouts();
    void cleanupPipelineCache();
    std::string generatePipelineName(const VkGraphicsPipelineCreateInfo& createInfo) const;
    bool validatePipelineCreateInfo(const VkGraphicsPipelineCreateInfo& createInfo) const;
};

// Pipeline creation helpers
struct VertexInputBindingDescription {
    uint32_t binding;
    uint32_t stride;
    VkVertexInputRate inputRate;
};

struct VertexInputAttributeDescription {
    uint32_t location;
    uint32_t binding;
    VkFormat format;
    uint32_t offset;
};

// Common pipeline configurations
namespace PipelineConfigs {
    // Forward declaration
    struct PipelineConfig;
    
    // Basic triangle pipeline
    extern const PipelineConfig BasicTriangle;
    
    // PBR pipeline
    extern const PipelineConfig PBR;
    
    // Shadow mapping pipeline
    extern const PipelineConfig Shadow;
    
    // Post-processing pipeline
    extern const PipelineConfig PostProcess;
}

} // namespace VortexEngine