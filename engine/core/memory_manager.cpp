#include "memory_manager.h"
#include <iostream>
#include <algorithm>

namespace VortexEngine {

MemoryManager::MemoryManager() {
    m_initialized = false;
    m_totalAllocatedMemory = 0;
    m_totalUsedMemory = 0;
    m_memoryAllocationCount = 0;
    m_stagingBuffer = VK_NULL_HANDLE;
    m_stagingBufferMemory = VK_NULL_HANDLE;
    m_stagingBufferSize = 0;
    m_device = VK_NULL_HANDLE;
    m_physicalDevice = VK_NULL_HANDLE;
    
    std::cout << "Initializing memory manager..." << std::endl;
}

MemoryManager::~MemoryManager() {
    shutdown();
}

bool MemoryManager::initialize(VkDevice device, VkPhysicalDevice physicalDevice) {
    if (m_initialized) {
        std::cout << "Memory manager is already initialized" << std::endl;
        return true;
    }

    try {
        m_device = device;
        m_physicalDevice = physicalDevice;

        // Get memory properties
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &m_memoryProperties);

        // Initialize alignment requirements for different buffer types
        m_alignments[static_cast<size_t>(BufferType::Vertex)] = 256;
        m_alignments[static_cast<size_t>(BufferType::Index)] = 256;
        m_alignments[static_cast<size_t>(BufferType::Uniform)] = 256;
        m_alignments[static_cast<size_t>(BufferType::Storage)] = 256;
        m_alignments[static_cast<size_t>(BufferType::Staging)] = 256;
        m_alignments[static_cast<size_t>(BufferType::Indirect)] = 256;

        m_initialized = true;
        std::cout << "Memory manager initialized successfully" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception during memory manager initialization: " << e.what() << std::endl;
        return false;
    }
}

void MemoryManager::shutdown() {
    if (!m_initialized) {
        return;
    }

    std::cout << "Shutting down memory manager..." << std::endl;

    // Cleanup command pools
    for (VkCommandPool pool : m_commandPools) {
        vkDestroyCommandPool(m_device, pool, nullptr);
    }
    m_commandPools.clear();

    // Cleanup staging buffer
    cleanupStagingBuffer();

    // Cleanup buffers
    cleanupBuffers();

    // Cleanup images
    cleanupImages();

    // Cleanup image views
    cleanupImageViews();

    m_initialized = false;
    std::cout << "Memory manager shutdown complete" << std::endl;
}

VkDeviceMemory MemoryManager::allocateBufferMemory(VkBuffer buffer, VkMemoryPropertyFlags properties) {
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

    uint32_t memoryTypeIndex;
    findMemoryType(memRequirements.memoryTypeBits, properties, &memoryTypeIndex);
    if (memoryTypeIndex == UINT32_MAX) {
        std::cerr << "Failed to find suitable memory type for buffer" << std::endl;
        return VK_NULL_HANDLE;
    }

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    VkDeviceMemory memory;
    VkResult result = vkAllocateMemory(m_device, &allocInfo, nullptr, &memory);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to allocate buffer memory: " << result << std::endl;
        return VK_NULL_HANDLE;
    }

    // Track allocation
    m_memorySizes[memory] = memRequirements.size;
    m_totalAllocatedMemory += memRequirements.size;
    m_memoryAllocationCount++;

    std::cout << "Allocated buffer memory: " << memRequirements.size << " bytes" << std::endl;
    return memory;
}

VkDeviceMemory MemoryManager::allocateImageMemory(VkImage image, VkMemoryPropertyFlags properties) {
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device, image, &memRequirements);

    uint32_t memoryTypeIndex;
    findMemoryType(memRequirements.memoryTypeBits, properties, &memoryTypeIndex);
    if (memoryTypeIndex == UINT32_MAX) {
        std::cerr << "Failed to find suitable memory type for image" << std::endl;
        return VK_NULL_HANDLE;
    }

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    VkDeviceMemory memory;
    VkResult result = vkAllocateMemory(m_device, &allocInfo, nullptr, &memory);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to allocate image memory: " << result << std::endl;
        return VK_NULL_HANDLE;
    }

    // Track allocation
    m_memorySizes[memory] = memRequirements.size;
    m_totalAllocatedMemory += memRequirements.size;
    m_memoryAllocationCount++;

    std::cout << "Allocated image memory: " << memRequirements.size << " bytes" << std::endl;
    return memory;
}

void MemoryManager::deallocateMemory(VkDeviceMemory memory) {
    if (memory == VK_NULL_HANDLE) {
        return;
    }

    // Find and remove from tracking
    auto it = m_memorySizes.find(memory);
    if (it != m_memorySizes.end()) {
        m_totalAllocatedMemory -= it->second;
        m_memorySizes.erase(it);
    }

    vkFreeMemory(m_device, memory, nullptr);
    m_memoryAllocationCount--;

    std::cout << "Deallocated memory" << std::endl;
}

VkBuffer MemoryManager::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
    VkBuffer buffer;
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create buffer: " << result << std::endl;
        return VK_NULL_HANDLE;
    }

    // Track buffer
    m_buffers.push_back(buffer);

    VkDeviceMemory memory = allocateBufferMemory(buffer, properties);
    if (memory == VK_NULL_HANDLE) {
        vkDestroyBuffer(m_device, buffer, nullptr);
        m_buffers.pop_back();
        return VK_NULL_HANDLE;
    }

    vkBindBufferMemory(m_device, buffer, memory, 0);

    std::cout << "Created buffer: " << size << " bytes" << std::endl;
    return buffer;
}

void MemoryManager::destroyBuffer(VkBuffer buffer) {
    if (buffer == VK_NULL_HANDLE) {
        return;
    }

    // Find buffer in tracking
    auto it = std::find(m_buffers.begin(), m_buffers.end(), buffer);
    if (it != m_buffers.end()) {
        m_buffers.erase(it);
    }

    // Find and deallocate memory
    for (size_t i = 0; i < m_bufferMemories.size(); ++i) {
        // Note: This is a simplified approach. In a real implementation,
        // you'd need to track which memory belongs to which buffer
        deallocateMemory(m_bufferMemories[i]);
    }
    m_bufferMemories.clear();

    vkDestroyBuffer(m_device, buffer, nullptr);
    std::cout << "Destroyed buffer" << std::endl;
}

void MemoryManager::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

VkImage MemoryManager::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                                  VkImageUsageFlags usage, VkMemoryPropertyFlags properties) {
    VkImage image;
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateImage(m_device, &imageInfo, nullptr, &image);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create image: " << result << std::endl;
        return VK_NULL_HANDLE;
    }

    // Track image
    m_images.push_back(image);

    VkDeviceMemory memory = allocateImageMemory(image, properties);
    if (memory == VK_NULL_HANDLE) {
        vkDestroyImage(m_device, image, nullptr);
        m_images.pop_back();
        return VK_NULL_HANDLE;
    }

    vkBindImageMemory(m_device, image, memory, 0);

    std::cout << "Created image: " << width << "x" << height << std::endl;
    return image;
}

void MemoryManager::destroyImage(VkImage image) {
    if (image == VK_NULL_HANDLE) {
        return;
    }

    // Find image in tracking
    auto it = std::find(m_images.begin(), m_images.end(), image);
    if (it != m_images.end()) {
        m_images.erase(it);
    }

    // Find and deallocate memory
    for (size_t i = 0; i < m_imageMemories.size(); ++i) {
        // Note: This is a simplified approach. In a real implementation,
        // you'd need to track which memory belongs to which image
        deallocateMemory(m_imageMemories[i]);
    }
    m_imageMemories.clear();

    vkDestroyImage(m_device, image, nullptr);
    std::cout << "Destroyed image" << std::endl;
}

VkImageView MemoryManager::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageView imageView;
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkResult result = vkCreateImageView(m_device, &viewInfo, nullptr, &imageView);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create image view: " << result << std::endl;
        return VK_NULL_HANDLE;
    }

    // Track image view
    m_imageViews.push_back(imageView);

    std::cout << "Created image view" << std::endl;
    return imageView;
}

void MemoryManager::destroyImageView(VkImageView imageView) {
    if (imageView == VK_NULL_HANDLE) {
        return;
    }

    // Find image view in tracking
    auto it = std::find(m_imageViews.begin(), m_imageViews.end(), imageView);
    if (it != m_imageViews.end()) {
        m_imageViews.erase(it);
    }

    vkDestroyImageView(m_device, imageView, nullptr);
    std::cout << "Destroyed image view" << std::endl;
}

VkBuffer MemoryManager::createStagingBuffer(VkDeviceSize size) {
    VkBuffer stagingBuffer = createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (stagingBuffer == VK_NULL_HANDLE) {
        return VK_NULL_HANDLE;
    }

    // Track staging buffer
    m_stagingBuffer = stagingBuffer;
    m_stagingBufferSize = size;

    std::cout << "Created staging buffer: " << size << " bytes" << std::endl;
    return stagingBuffer;
}

void MemoryManager::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(commandBuffer);
}

void MemoryManager::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t offset_x, uint32_t offset_y) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {static_cast<int32_t>(offset_x), static_cast<int32_t>(offset_y), 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(commandBuffer);
}

void* MemoryManager::mapMemory(VkDeviceMemory memory, VkDeviceSize size) {
    void* data;
    VkResult result = vkMapMemory(m_device, memory, 0, size, 0, &data);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to map memory: " << result << std::endl;
        return nullptr;
    }
    return data;
}

void MemoryManager::unmapMemory(VkDeviceMemory memory) {
    vkUnmapMemory(m_device, memory);
}

uint32_t MemoryManager::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, uint32_t* memoryTypeIndex) {
    for (uint32_t i = 0; i < m_memoryProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (m_memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            *memoryTypeIndex = i;
            return i;
        }
    }
    
    std::cerr << "Failed to find suitable memory type" << std::endl;
    return UINT32_MAX;
}

VkMemoryPropertyFlags MemoryManager::getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, uint32_t* typeIndex) {
    for (uint32_t i = 0; i < m_memoryProperties.memoryTypeCount; i++) {
        if ((typeBits & (1 << i)) && (m_memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            *typeIndex = i;
            return m_memoryProperties.memoryTypes[i].propertyFlags;
        }
    }
    return 0;
}

void MemoryManager::printMemoryInfo() const {
    std::cout << "=== Memory Manager Info ===" << std::endl;
    std::cout << "Total Allocated Memory: " << m_totalAllocatedMemory << " bytes" << std::endl;
    std::cout << "Total Used Memory: " << m_totalUsedMemory << " bytes" << std::endl;
    std::cout << "Memory Allocation Count: " << m_memoryAllocationCount << std::endl;
    std::cout << "Buffer Count: " << m_buffers.size() << std::endl;
    std::cout << "Image Count: " << m_images.size() << std::endl;
    std::cout << "Image View Count: " << m_imageViews.size() << std::endl;
    std::cout << "==========================" << std::endl;
}

VkCommandBuffer MemoryManager::beginSingleTimeCommands() {
    // Create a command pool for one-time commands
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = 0; // Use the first queue family

    VkCommandPool commandPool;
    VkResult result = vkCreateCommandPool(m_device, &poolInfo, nullptr, &commandPool);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create command pool: " << result << std::endl;
        return VK_NULL_HANDLE;
    }

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    result = vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to allocate command buffer: " << result << std::endl;
        vkDestroyCommandPool(m_device, commandPool, nullptr);
        return VK_NULL_HANDLE;
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to begin command buffer: " << result << std::endl;
        vkFreeCommandBuffers(m_device, commandPool, 1, &commandBuffer);
        vkDestroyCommandPool(m_device, commandPool, nullptr);
        return VK_NULL_HANDLE;
    }

    // Store the command pool for cleanup
    m_commandPools.push_back(commandPool);

    return commandBuffer;
}

void MemoryManager::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    // Submit the command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkFence fence;
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkCreateFence(m_device, &fenceInfo, nullptr, &fence);

    VkResult result = vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, fence);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to submit command buffer: " << result << std::endl;
    }

    vkWaitForFences(m_device, 1, &fence, VK_TRUE, UINT64_MAX);
    vkDestroyFence(m_device, fence, nullptr);

    // Free the command buffer and destroy the command pool
    if (!m_commandPools.empty()) {
        vkFreeCommandBuffers(m_device, m_commandPools.back(), 1, &commandBuffer);
        vkDestroyCommandPool(m_device, m_commandPools.back(), nullptr);
        m_commandPools.pop_back();
    }
}

void MemoryManager::cleanupStagingBuffer() {
    if (m_stagingBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_device, m_stagingBuffer, nullptr);
        m_stagingBuffer = VK_NULL_HANDLE;
    }
    if (m_stagingBufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device, m_stagingBufferMemory, nullptr);
        m_stagingBufferMemory = VK_NULL_HANDLE;
    }
    m_stagingBufferSize = 0;
}

void MemoryManager::cleanupBuffers() {
    for (VkBuffer buffer : m_buffers) {
        vkDestroyBuffer(m_device, buffer, nullptr);
    }
    m_buffers.clear();

    for (VkDeviceMemory memory : m_bufferMemories) {
        vkFreeMemory(m_device, memory, nullptr);
    }
    m_bufferMemories.clear();
}

void MemoryManager::cleanupImages() {
    for (VkImage image : m_images) {
        vkDestroyImage(m_device, image, nullptr);
    }
    m_images.clear();

    for (VkDeviceMemory memory : m_imageMemories) {
        vkFreeMemory(m_device, memory, nullptr);
    }
    m_imageMemories.clear();
}

void MemoryManager::cleanupImageViews() {
    for (VkImageView imageView : m_imageViews) {
        vkDestroyImageView(m_device, imageView, nullptr);
    }
    m_imageViews.clear();
}

} // namespace VortexEngine