#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>

namespace VortexEngine {

class BufferAllocator {
public:
    BufferAllocator();
    ~BufferAllocator();

    // Buffer allocator lifecycle
    bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, void* memoryManager = nullptr);
    void shutdown();

    // Buffer types
    enum class BufferType {
        Vertex,
        Index,
        Uniform,
        Storage,
        Staging,
        Indirect,
        Count
    };

    // Buffer allocation
    struct BufferAllocation {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkDeviceSize size = 0;
        VkDeviceSize offset = 0;
        void* mappedPtr = nullptr;
        BufferType type = BufferType::Count;
        bool persistentMapping = false;
    };

    BufferAllocation allocateBuffer(BufferType type, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    void deallocateBuffer(const BufferAllocation& allocation);

    // Vertex buffer
    BufferAllocation createVertexBuffer(VkDeviceSize size, const void* data = nullptr);
    void updateVertexBuffer(const BufferAllocation& allocation, const void* data, VkDeviceSize size, VkDeviceSize offset = 0);

    // Index buffer
    BufferAllocation createIndexBuffer(VkDeviceSize size, const void* data = nullptr);
    void updateIndexBuffer(const BufferAllocation& allocation, const void* data, VkDeviceSize size, VkDeviceSize offset = 0);

    // Uniform buffer
    BufferAllocation createUniformBuffer(VkDeviceSize size);
    void updateUniformBuffer(const BufferAllocation& allocation, const void* data, VkDeviceSize size, VkDeviceSize offset = 0);

    // Storage buffer
    BufferAllocation createStorageBuffer(VkDeviceSize size);
    void updateStorageBuffer(const BufferAllocation& allocation, const void* data, VkDeviceSize size, VkDeviceSize offset = 0);

    // Staging buffer
    BufferAllocation createStagingBuffer(VkDeviceSize size);
    void copyFromStaging(const BufferAllocation& staging, const BufferAllocation& destination, VkDeviceSize size);

    // Buffer mapping
    void* mapBuffer(const BufferAllocation& allocation);
    void unmapBuffer(const BufferAllocation& allocation);

    // Buffer copying
    void copyBuffer(VkCommandBuffer commandBuffer, const BufferAllocation& src, const BufferAllocation& dst, VkDeviceSize size);

    // Buffer descriptor info
    VkDescriptorBufferInfo getDescriptorBufferInfo(const BufferAllocation& allocation) const;

    // Buffer utilities
    VkDeviceSize getAlignmentRequirement(BufferType type) const;
    VkBufferUsageFlags getBufferUsageFlags(BufferType type) const;
    VkMemoryPropertyFlags getMemoryPropertyFlags(BufferType type) const;

    // Memory management
    void flushMappedMemory(const BufferAllocation& allocation, VkDeviceSize offset, VkDeviceSize size);
    void invalidateMappedMemory(const BufferAllocation& allocation, VkDeviceSize offset, VkDeviceSize size);

    // Debug information
    void printBufferInfo() const;
    size_t getTotalAllocatedMemory() const { return m_totalAllocatedMemory; }
    size_t getBufferCount() const { return m_bufferCount; }
    uint32_t getAllocationCount() const { return m_allocationCount; }

    // Buffer pool management
    struct BufferPool {
        BufferType type;
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkDeviceSize totalSize = 0;
        VkDeviceSize usedSize = 0;
        VkDeviceSize alignment = 0;
        std::vector<BufferAllocation> allocations;
    };

    bool createBufferPool(BufferType type, VkDeviceSize size);
    BufferAllocation allocateFromPool(BufferType type, VkDeviceSize size);
    void deallocateFromPool(const BufferAllocation& allocation);
    void cleanupBufferPools();

private:
    // Vulkan objects
    VkDevice m_device = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    void* m_memoryManager = nullptr;

    // Buffer storage
    std::vector<BufferAllocation> m_allocations;
    std::unordered_map<VkBuffer, BufferAllocation> m_bufferMap;
    std::vector<BufferPool> m_bufferPools;

    // Memory tracking
    size_t m_totalAllocatedMemory = 0;
    size_t m_bufferCount = 0;
    uint32_t m_allocationCount = 0;

    // Configuration
    bool m_initialized = false;
    std::mutex m_bufferMutex;

    // Alignment requirements for different buffer types
    VkDeviceSize m_alignments[static_cast<size_t>(BufferType::Count)] = {};

    // Internal methods
    VkDeviceSize calculateAlignment(BufferType type, VkDeviceSize size) const;
    BufferAllocation createBufferInternal(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, BufferType type);
    void destroyBufferInternal(const BufferAllocation& allocation);
    void updateBufferTracking(const BufferAllocation& allocation, bool allocate);
    BufferPool* findBufferPool(BufferType type, VkDeviceSize size);
    void createBufferPoolInternal(BufferPool& pool, BufferType type, VkDeviceSize size);
    void destroyBufferPoolInternal(BufferPool& pool);
};

// Buffer creation helpers
struct BufferCreateInfo {
    BufferAllocator::BufferType type;
    VkDeviceSize size;
    const void* data = nullptr;
    VkDeviceSize offset = 0;
    bool persistentMapping = false;
};

// Common buffer sizes
namespace BufferSizes {
    constexpr VkDeviceSize Uniform = 256 * 1024;      // 256 KB
    constexpr VkDeviceSize Vertex = 16 * 1024 * 1024;  // 16 MB
    constexpr VkDeviceSize Index = 4 * 1024 * 1024;    // 4 MB
    constexpr VkDeviceSize Storage = 64 * 1024 * 1024; // 64 MB
    constexpr VkDeviceSize Staging = 16 * 1024 * 1024; // 16 MB
}

} // namespace VortexEngine