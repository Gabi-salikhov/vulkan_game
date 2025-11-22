#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <array>

namespace VortexEngine {

// Buffer type enumeration
enum class BufferType {
    Vertex,
    Index,
    Uniform,
    Storage,
    Staging,
    Indirect
};

class MemoryManager {
public:
    MemoryManager();
    ~MemoryManager();
    
    // Graphics queue setter
    void setGraphicsQueue(VkQueue queue) { m_graphicsQueue = queue; }

    // Memory manager lifecycle
    bool initialize(VkDevice device, VkPhysicalDevice physicalDevice);
    void shutdown();

    // Memory allocation
    VkDeviceMemory allocateBufferMemory(VkBuffer buffer, VkMemoryPropertyFlags properties);
    VkDeviceMemory allocateImageMemory(VkImage image, VkMemoryPropertyFlags properties);
    void deallocateMemory(VkDeviceMemory memory);

    // Buffer management
    VkBuffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    void destroyBuffer(VkBuffer buffer);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    // Image management
    VkImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, 
                       VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
    void destroyImage(VkImage image);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    void destroyImageView(VkImageView imageView);

    // Staging buffer for CPU to GPU transfers
    VkBuffer createStagingBuffer(VkDeviceSize size);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t offset_x, uint32_t offset_y);

    // Memory mapping
    void* mapMemory(VkDeviceMemory memory, VkDeviceSize size = VK_WHOLE_SIZE);
    void unmapMemory(VkDeviceMemory memory);

    // Memory properties
    VkMemoryPropertyFlags getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, uint32_t* typeIndex);

    // Memory tracking
    size_t getTotalAllocatedMemory() const { return m_totalAllocatedMemory; }
    size_t getTotalUsedMemory() const { return m_totalUsedMemory; }
    uint32_t getMemoryAllocationCount() const { return m_memoryAllocationCount; }

    // Debug information
    void printMemoryInfo() const;

private:
    // Vulkan objects
    VkDevice m_device = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    bool m_initialized = false;

    // Memory tracking
    size_t m_totalAllocatedMemory = 0;
    size_t m_totalUsedMemory = 0;
    uint32_t m_memoryAllocationCount = 0;

    // Memory allocation tracking
    std::unordered_map<VkDeviceMemory, VkDeviceSize> m_memorySizes;
    std::mutex m_memoryMutex;

    // Buffer tracking
    std::vector<VkBuffer> m_buffers;
    std::vector<VkDeviceMemory> m_bufferMemories;

    // Image tracking
    std::vector<VkImage> m_images;
    std::vector<VkDeviceMemory> m_imageMemories;
    std::vector<VkImageView> m_imageViews;

    // Staging buffer
    VkBuffer m_stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_stagingBufferMemory = VK_NULL_HANDLE;
    VkDeviceSize m_stagingBufferSize = 0;

    // Memory type properties
    VkPhysicalDeviceMemoryProperties m_memoryProperties;
    
    // Alignment requirements for different buffer types
    std::array<size_t, static_cast<size_t>(BufferType::Indirect) + 1> m_alignments;
    
    // Command pools for one-time commands
    std::vector<VkCommandPool> m_commandPools;
    
    // Graphics queue for command submission
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;

    // Internal methods
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, uint32_t* memoryTypeIndex);
    void cleanupStagingBuffer();
    void cleanupBuffers();
    void cleanupImages();
    void cleanupImageViews();
    
    // Command buffer management for one-time operations
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
};

// Image creation helper
struct ImageCreateInfo {
    uint32_t width;
    uint32_t height;
    VkFormat format;
    VkImageTiling tiling;
    VkImageUsageFlags usage;
    VkMemoryPropertyFlags properties;
    uint32_t mipLevels = 1;
    uint32_t arrayLayers = 1;
    VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
    VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
};

} // namespace VortexEngine