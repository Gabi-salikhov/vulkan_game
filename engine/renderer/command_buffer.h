#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <mutex>

namespace VortexEngine {

class CommandBuffer {
public:
    CommandBuffer(VkDevice device, VkCommandPool commandPool);
    ~CommandBuffer();

    // Command buffer lifecycle
    bool create();
    void destroy();
    
    // Command buffer recording
    void beginRecording(VkCommandBufferUsageFlags flags = 0);
    void endRecording();
    void reset();
    
    // Command buffer submission
    VkCommandBuffer getHandle() const { return m_commandBuffer; }
    bool isRecording() const { return m_isRecording; }
    
    // Drawing commands
    void beginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, 
                        VkExtent2D extent, VkClearValue* clearValues, uint32_t clearValueCount);
    void endRenderPass();
    
    void bindPipeline(VkPipeline pipeline);
    void bindVertexBuffers(VkBuffer vertexBuffer, VkDeviceSize offset = 0);
    void bindIndexBuffer(VkBuffer indexBuffer, VkDeviceSize offset = 0);
    void bindDescriptorSets(VkPipelineLayout pipelineLayout, 
                           const std::vector<VkDescriptorSet>& descriptorSets);
    
    void draw(uint32_t vertexCount, uint32_t instanceCount = 1, 
             uint32_t firstVertex = 0, uint32_t firstInstance = 0);
    void drawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, 
                    uint32_t firstIndex = 0, int32_t vertexOffset = 0, 
                    uint32_t firstInstance = 0);
    
    // Viewport and scissor
    void setViewport(uint32_t firstViewport, uint32_t viewportCount, 
                    const VkViewport* viewports);
    void setScissor(uint32_t firstScissor, uint32_t scissorCount, 
                   const VkRect2D* scissors);
    
    // State setting
    void setLineWidth(float lineWidth);
    void setDepthBias(float depthBiasConstantFactor, float depthBiasSlopeFactor, 
                     float depthBiasClamp);
    
    // Copy operations
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, 
                   VkDeviceSize size, VkDeviceSize srcOffset = 0, 
                   VkDeviceSize dstOffset = 0);
    
    // Image operations
    void transitionImageLayout(VkImage image, VkImageLayout oldLayout, 
                              VkImageLayout newLayout, VkImageAspectFlags aspectMask);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, 
                          uint32_t height, uint32_t layerCount = 1);
    void blitImage(VkImage srcImage, VkImage dstImage, 
                  const VkImageBlit* region, VkFilter filter);

private:
    VkDevice m_device;
    VkCommandPool m_commandPool;
    VkCommandBuffer m_commandBuffer;
    bool m_isRecording;
};

class CommandBufferManager {
public:
    CommandBufferManager(VkDevice device, VkCommandPool commandPool);
    ~CommandBufferManager();
    
    // Command buffer management
    std::shared_ptr<CommandBuffer> allocateCommandBuffer();
    void freeCommandBuffer(std::shared_ptr<CommandBuffer> commandBuffer);
    void resetAllCommandBuffers();
    
    // Synchronization
    void waitForAllCommandBuffers();
    
    // Accessors
    VkCommandPool getCommandPool() const { return m_commandPool; }
    uint32_t getActiveCommandBufferCount() const { return m_activeCommandBuffers.size(); }
    
private:
    VkDevice m_device;
    VkCommandPool m_commandPool;
    std::vector<std::shared_ptr<CommandBuffer>> m_activeCommandBuffers;
    std::mutex m_mutex;
};

} // namespace VortexEngine