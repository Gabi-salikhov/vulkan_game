#include "command_buffer.h"
#include <iostream>
#include <stdexcept>
#include <algorithm>

namespace VortexEngine {

CommandBuffer::CommandBuffer(VkDevice device, VkCommandPool commandPool)
    : m_device(device)
    , m_commandPool(commandPool)
    , m_commandBuffer(VK_NULL_HANDLE)
    , m_isRecording(false) {
}

CommandBuffer::~CommandBuffer() {
    destroy();
}

bool CommandBuffer::create() {
    if (m_commandBuffer != VK_NULL_HANDLE) {
        std::cout << "Command buffer already created" << std::endl;
        return true;
    }

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkResult result = vkAllocateCommandBuffers(m_device, &allocInfo, &m_commandBuffer);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to allocate command buffer: " << result << std::endl;
        return false;
    }

    std::cout << "Command buffer created successfully" << std::endl;
    return true;
}

void CommandBuffer::destroy() {
    if (m_commandBuffer != VK_NULL_HANDLE) {
        vkFreeCommandBuffers(m_device, m_commandPool, 1, &m_commandBuffer);
        m_commandBuffer = VK_NULL_HANDLE;
        m_isRecording = false;
    }
}

void CommandBuffer::beginRecording(VkCommandBufferUsageFlags flags) {
    if (m_isRecording) {
        std::cerr << "Command buffer is already in recording state" << std::endl;
        return;
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = flags;

    VkResult result = vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to begin command buffer recording: " << result << std::endl;
        return;
    }

    m_isRecording = true;
}

void CommandBuffer::endRecording() {
    if (!m_isRecording) {
        std::cerr << "Command buffer is not in recording state" << std::endl;
        return;
    }

    VkResult result = vkEndCommandBuffer(m_commandBuffer);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to end command buffer recording: " << result << std::endl;
        return;
    }

    m_isRecording = false;
}

void CommandBuffer::reset() {
    if (m_isRecording) {
        std::cerr << "Cannot reset command buffer while recording" << std::endl;
        return;
    }

    VkResult result = vkResetCommandBuffer(m_commandBuffer, 0);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to reset command buffer: " << result << std::endl;
    }
}

void CommandBuffer::beginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, 
                                   VkExtent2D extent, VkClearValue* clearValues, 
                                   uint32_t clearValueCount) {
    if (!m_isRecording) {
        std::cerr << "Cannot begin render pass - command buffer not recording" << std::endl;
        return;
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = extent;
    renderPassInfo.clearValueCount = clearValueCount;
    renderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::endRenderPass() {
    if (!m_isRecording) {
        std::cerr << "Cannot end render pass - command buffer not recording" << std::endl;
        return;
    }

    vkCmdEndRenderPass(m_commandBuffer);
}

void CommandBuffer::bindPipeline(VkPipeline pipeline) {
    if (!m_isRecording) {
        std::cerr << "Cannot bind pipeline - command buffer not recording" << std::endl;
        return;
    }

    vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void CommandBuffer::bindVertexBuffers(VkBuffer vertexBuffer, VkDeviceSize offset) {
    if (!m_isRecording) {
        std::cerr << "Cannot bind vertex buffer - command buffer not recording" << std::endl;
        return;
    }

    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {offset};
    
    vkCmdBindVertexBuffers(m_commandBuffer, 0, 1, vertexBuffers, offsets);
}

void CommandBuffer::bindIndexBuffer(VkBuffer indexBuffer, VkDeviceSize offset) {
    if (!m_isRecording) {
        std::cerr << "Cannot bind index buffer - command buffer not recording" << std::endl;
        return;
    }

    vkCmdBindIndexBuffer(m_commandBuffer, indexBuffer, offset, VK_INDEX_TYPE_UINT32);
}

void CommandBuffer::bindDescriptorSets(VkPipelineLayout pipelineLayout, 
                                      const std::vector<VkDescriptorSet>& descriptorSets) {
    if (!m_isRecording) {
        std::cerr << "Cannot bind descriptor sets - command buffer not recording" << std::endl;
        return;
    }

    vkCmdBindDescriptorSets(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
                           pipelineLayout, 0, static_cast<uint32_t>(descriptorSets.size()), 
                           descriptorSets.data(), 0, nullptr);
}

void CommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount, 
                        uint32_t firstVertex, uint32_t firstInstance) {
    if (!m_isRecording) {
        std::cerr << "Cannot draw - command buffer not recording" << std::endl;
        return;
    }

    vkCmdDraw(m_commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount, 
                               uint32_t firstIndex, int32_t vertexOffset, 
                               uint32_t firstInstance) {
    if (!m_isRecording) {
        std::cerr << "Cannot draw indexed - command buffer not recording" << std::endl;
        return;
    }

    vkCmdDrawIndexed(m_commandBuffer, indexCount, instanceCount, firstIndex, 
                    vertexOffset, firstInstance);
}

void CommandBuffer::setViewport(uint32_t firstViewport, uint32_t viewportCount, 
                               const VkViewport* viewports) {
    if (!m_isRecording) {
        std::cerr << "Cannot set viewport - command buffer not recording" << std::endl;
        return;
    }

    vkCmdSetViewport(m_commandBuffer, firstViewport, viewportCount, viewports);
}

void CommandBuffer::setScissor(uint32_t firstScissor, uint32_t scissorCount, 
                              const VkRect2D* scissors) {
    if (!m_isRecording) {
        std::cerr << "Cannot set scissor - command buffer not recording" << std::endl;
        return;
    }

    vkCmdSetScissor(m_commandBuffer, firstScissor, scissorCount, scissors);
}

void CommandBuffer::setLineWidth(float lineWidth) {
    if (!m_isRecording) {
        std::cerr << "Cannot set line width - command buffer not recording" << std::endl;
        return;
    }

    vkCmdSetLineWidth(m_commandBuffer, lineWidth);
}

void CommandBuffer::setDepthBias(float depthBiasConstantFactor, float depthBiasSlopeFactor, 
                                float depthBiasClamp) {
    if (!m_isRecording) {
        std::cerr << "Cannot set depth bias - command buffer not recording" << std::endl;
        return;
    }

    vkCmdSetDepthBias(m_commandBuffer, depthBiasConstantFactor, 
                     depthBiasClamp, depthBiasSlopeFactor);
}

void CommandBuffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, 
                              VkDeviceSize size, VkDeviceSize srcOffset, 
                              VkDeviceSize dstOffset) {
    if (!m_isRecording) {
        std::cerr << "Cannot copy buffer - command buffer not recording" << std::endl;
        return;
    }

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = srcOffset;
    copyRegion.dstOffset = dstOffset;
    copyRegion.size = size;

    vkCmdCopyBuffer(m_commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
}

void CommandBuffer::transitionImageLayout(VkImage image, VkImageLayout oldLayout, 
                                         VkImageLayout newLayout, VkImageAspectFlags aspectMask) {
    if (!m_isRecording) {
        std::cerr << "Cannot transition image layout - command buffer not recording" << std::endl;
        return;
    }

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspectMask;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
               newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | 
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else {
        throw std::invalid_argument("Unsupported layout transition!");
    }

    vkCmdPipelineBarrier(m_commandBuffer, sourceStage, destinationStage, 0,
                        0, nullptr,
                        0, nullptr,
                        1, &barrier);
}

void CommandBuffer::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, 
                                     uint32_t height, uint32_t layerCount) {
    if (!m_isRecording) {
        std::cerr << "Cannot copy buffer to image - command buffer not recording" << std::endl;
        return;
    }

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = layerCount;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(m_commandBuffer, buffer, image, 
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void CommandBuffer::blitImage(VkImage srcImage, VkImage dstImage, 
                             const VkImageBlit* region, VkFilter filter) {
    if (!m_isRecording) {
        std::cerr << "Cannot blit image - command buffer not recording" << std::endl;
        return;
    }

    vkCmdBlitImage(m_commandBuffer, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                  dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, region, filter);
}

// CommandBufferManager implementation
CommandBufferManager::CommandBufferManager(VkDevice device, VkCommandPool commandPool)
    : m_device(device)
    , m_commandPool(commandPool) {
}

CommandBufferManager::~CommandBufferManager() {
    waitForAllCommandBuffers();
    
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& cmdBuffer : m_activeCommandBuffers) {
        cmdBuffer->destroy();
    }
    m_activeCommandBuffers.clear();
}

std::shared_ptr<CommandBuffer> CommandBufferManager::allocateCommandBuffer() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto cmdBuffer = std::make_shared<CommandBuffer>(m_device, m_commandPool);
    if (!cmdBuffer->create()) {
        return nullptr;
    }
    
    m_activeCommandBuffers.push_back(cmdBuffer);
    return cmdBuffer;
}

void CommandBufferManager::freeCommandBuffer(std::shared_ptr<CommandBuffer> commandBuffer) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = std::find(m_activeCommandBuffers.begin(), m_activeCommandBuffers.end(), commandBuffer);
    if (it != m_activeCommandBuffers.end()) {
        m_activeCommandBuffers.erase(it);
    }
}

void CommandBufferManager::resetAllCommandBuffers() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    for (auto& cmdBuffer : m_activeCommandBuffers) {
        cmdBuffer->reset();
    }
}

void CommandBufferManager::waitForAllCommandBuffers() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    for (auto& cmdBuffer : m_activeCommandBuffers) {
        if (cmdBuffer->isRecording()) {
            cmdBuffer->endRecording();
        }
    }
}

} // namespace VortexEngine