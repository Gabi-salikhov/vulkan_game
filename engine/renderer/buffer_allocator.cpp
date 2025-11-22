#include "buffer_allocator.h"
#include "../core/memory_manager.h"
#include <iostream>
#include <cstring>
#include <algorithm>
#include <cmath>

namespace VortexEngine {

BufferAllocator::BufferAllocator() {
    std::cout << "Initializing buffer allocator..." << std::endl;
}

BufferAllocator::~BufferAllocator() {
    shutdown();
}

bool BufferAllocator::initialize(VkDevice device, VkPhysicalDevice physicalDevice, void* memoryManager) {
    if (m_initialized) {
        std::cout << "Buffer allocator is already initialized" << std::endl;
        return true;
    }

    try {
        m_device = device;
        m_physicalDevice = physicalDevice;
        m_memoryManager = memoryManager;

        // Calculate alignment requirements for different buffer types
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicalDevice, &properties);

        // Alignments based on buffer type and device limits
        m_alignments[static_cast<size_t>(BufferType::Vertex)] = std::max(properties.limits.minUniformBufferOffsetAlignment, static_cast<VkDeviceSize>(1));
        m_alignments[static_cast<size_t>(BufferType::Index)] = std::max(properties.limits.minUniformBufferOffsetAlignment, static_cast<VkDeviceSize>(1));
        m_alignments[static_cast<size_t>(BufferType::Uniform)] = std::max(properties.limits.minUniformBufferOffsetAlignment, static_cast<VkDeviceSize>(256));
        m_alignments[static_cast<size_t>(BufferType::Storage)] = std::max(properties.limits.minStorageBufferOffsetAlignment, static_cast<VkDeviceSize>(1));
        m_alignments[static_cast<size_t>(BufferType::Staging)] = std::max(properties.limits.minUniformBufferOffsetAlignment, static_cast<VkDeviceSize>(1));
        m_alignments[static_cast<size_t>(BufferType::Indirect)] = std::max(properties.limits.minUniformBufferOffsetAlignment, static_cast<VkDeviceSize>(1));

        m_initialized = true;
        std::cout << "Buffer allocator initialized successfully" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception during buffer allocator initialization: " << e.what() << std::endl;
        return false;
    }
}

void BufferAllocator::shutdown() {
    if (!m_initialized) {
        return;
    }

    std::cout << "Shutting down buffer allocator..." << std::endl;

    // Cleanup buffer pools
    cleanupBufferPools();

    // Destroy all individual allocations
    std::lock_guard<std::mutex> lock(m_bufferMutex);
    for (auto& allocation : m_allocations) {
        destroyBufferInternal(allocation);
    }
    m_allocations.clear();
    m_bufferMap.clear();

    m_initialized = false;
    std::cout << "Buffer allocator shutdown complete" << std::endl;
}

BufferAllocator::BufferAllocation BufferAllocator::allocateBuffer(BufferType type, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
    if (!m_initialized) {
        std::cerr << "Buffer allocator is not initialized" << std::endl;
        return {};
    }

    std::lock_guard<std::mutex> lock(m_bufferMutex);
    return createBufferInternal(size, usage, properties, type);
}

void BufferAllocator::deallocateBuffer(const BufferAllocation& allocation) {
    if (!m_initialized || allocation.buffer == VK_NULL_HANDLE) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_bufferMutex);
    destroyBufferInternal(allocation);
    updateBufferTracking(allocation, false);
}

BufferAllocator::BufferAllocation BufferAllocator::createVertexBuffer(VkDeviceSize size, const void* data) {
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    
    BufferAllocation allocation = allocateBuffer(BufferType::Vertex, size, usage, properties);
    
    if (data && allocation.buffer != VK_NULL_HANDLE) {
        // Use staging buffer to upload data
        BufferAllocation staging = createStagingBuffer(size);
        if (staging.buffer != VK_NULL_HANDLE) {
            memcpy(mapBuffer(staging), data, static_cast<size_t>(size));
            unmapBuffer(staging);
            // Note: copy operation requires command buffer - will be handled by renderer
        }
        deallocateBuffer(staging);
    }
    
    return allocation;
}

void BufferAllocator::updateVertexBuffer(const BufferAllocation& allocation, const void* data, VkDeviceSize size, VkDeviceSize offset) {
    if (!data || allocation.buffer == VK_NULL_HANDLE) return;
    
    // For vertex buffers, we typically need to use a staging buffer and command buffer
    // This is a simplified version - real implementation would use command buffer
    if (allocation.mappedPtr) {
        memcpy(static_cast<char*>(allocation.mappedPtr) + offset, data, static_cast<size_t>(size));
        flushMappedMemory(allocation, offset, size);
    }
}

BufferAllocator::BufferAllocation BufferAllocator::createIndexBuffer(VkDeviceSize size, const void* data) {
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    
    BufferAllocation allocation = allocateBuffer(BufferType::Index, size, usage, properties);
    
    if (data && allocation.buffer != VK_NULL_HANDLE) {
        // Use staging buffer to upload data
        BufferAllocation staging = createStagingBuffer(size);
        if (staging.buffer != VK_NULL_HANDLE) {
            memcpy(mapBuffer(staging), data, static_cast<size_t>(size));
            unmapBuffer(staging);
            // Note: copy operation requires command buffer - will be handled by renderer
        }
        deallocateBuffer(staging);
    }
    
    return allocation;
}

void BufferAllocator::updateIndexBuffer(const BufferAllocation& allocation, const void* data, VkDeviceSize size, VkDeviceSize offset) {
    if (!data || allocation.buffer == VK_NULL_HANDLE) return;
    
    if (allocation.mappedPtr) {
        memcpy(static_cast<char*>(allocation.mappedPtr) + offset, data, static_cast<size_t>(size));
        flushMappedMemory(allocation, offset, size);
    }
}

BufferAllocator::BufferAllocation BufferAllocator::createUniformBuffer(VkDeviceSize size) {
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    
    BufferAllocation allocation = allocateBuffer(BufferType::Uniform, size, usage, properties);
    
    if (allocation.buffer != VK_NULL_HANDLE) {
        allocation.persistentMapping = true;
        mapBuffer(allocation);
    }
    
    return allocation;
}

void BufferAllocator::updateUniformBuffer(const BufferAllocation& allocation, const void* data, VkDeviceSize size, VkDeviceSize offset) {
    if (!data || allocation.buffer == VK_NULL_HANDLE) return;
    
    if (allocation.mappedPtr) {
        memcpy(static_cast<char*>(allocation.mappedPtr) + offset, data, static_cast<size_t>(size));
        flushMappedMemory(allocation, offset, size);
    }
}

BufferAllocator::BufferAllocation BufferAllocator::createStorageBuffer(VkDeviceSize size) {
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    
    return allocateBuffer(BufferType::Storage, size, usage, properties);
}

void BufferAllocator::updateStorageBuffer(const BufferAllocation& allocation, const void* data, VkDeviceSize size, VkDeviceSize offset) {
    if (!data || allocation.buffer == VK_NULL_HANDLE) return;
    
    // Storage buffers typically require compute shaders for updates
    // This is a placeholder for future implementation
}

BufferAllocator::BufferAllocation BufferAllocator::createStagingBuffer(VkDeviceSize size) {
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    
    BufferAllocation allocation = allocateBuffer(BufferType::Staging, size, usage, properties);
    
    if (allocation.buffer != VK_NULL_HANDLE) {
        allocation.persistentMapping = true;
        mapBuffer(allocation);
    }
    
    return allocation;
}

void BufferAllocator::copyFromStaging(const BufferAllocation& staging, const BufferAllocation& destination, VkDeviceSize size) {
    if (staging.buffer == VK_NULL_HANDLE || destination.buffer == VK_NULL_HANDLE) return;
    
    // This requires a command buffer to execute the copy
    // Implementation will be handled by the renderer system
}

void* BufferAllocator::mapBuffer(const BufferAllocation& allocation) {
    if (allocation.buffer == VK_NULL_HANDLE || allocation.mappedPtr) {
        return allocation.mappedPtr;
    }

    VkResult result = vkMapMemory(m_device, allocation.memory, allocation.offset, allocation.size, 0, &const_cast<void*&>(allocation.mappedPtr));
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to map buffer memory: " << result << std::endl;
        return nullptr;
    }

    return allocation.mappedPtr;
}

void BufferAllocator::unmapBuffer(const BufferAllocation& allocation) {
    if (allocation.buffer == VK_NULL_HANDLE || !allocation.mappedPtr) {
        return;
    }

    vkUnmapMemory(m_device, allocation.memory);
    const_cast<BufferAllocation&>(allocation).mappedPtr = nullptr;
}

void BufferAllocator::copyBuffer(VkCommandBuffer commandBuffer, const BufferAllocation& src, const BufferAllocation& dst, VkDeviceSize size) {
    if (src.buffer == VK_NULL_HANDLE || dst.buffer == VK_NULL_HANDLE || !commandBuffer) {
        return;
    }

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = src.offset;
    copyRegion.dstOffset = dst.offset;
    copyRegion.size = size;

    vkCmdCopyBuffer(commandBuffer, src.buffer, dst.buffer, 1, &copyRegion);
}

VkDescriptorBufferInfo BufferAllocator::getDescriptorBufferInfo(const BufferAllocation& allocation) const {
    VkDescriptorBufferInfo info{};
    info.buffer = allocation.buffer;
    info.offset = allocation.offset;
    info.range = allocation.size;
    return info;
}

VkDeviceSize BufferAllocator::getAlignmentRequirement(BufferType type) const {
    return m_alignments[static_cast<size_t>(type)];
}

VkBufferUsageFlags BufferAllocator::getBufferUsageFlags(BufferType type) const {
    switch (type) {
        case BufferType::Vertex:
            return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        case BufferType::Index:
            return VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        case BufferType::Uniform:
            return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        case BufferType::Storage:
            return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        case BufferType::Staging:
            return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        case BufferType::Indirect:
            return VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        default:
            return 0;
    }
}

VkMemoryPropertyFlags BufferAllocator::getMemoryPropertyFlags(BufferType type) const {
    switch (type) {
        case BufferType::Vertex:
        case BufferType::Index:
        case BufferType::Storage:
            return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        case BufferType::Uniform:
            return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        case BufferType::Staging:
            return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        case BufferType::Indirect:
            return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        default:
            return 0;
    }
}

void BufferAllocator::flushMappedMemory(const BufferAllocation& allocation, VkDeviceSize offset, VkDeviceSize size) {
    if (allocation.buffer == VK_NULL_HANDLE || !allocation.mappedPtr) {
        return;
    }

    VkMappedMemoryRange range{};
    range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range.memory = allocation.memory;
    range.offset = allocation.offset + offset;
    range.size = size;

    vkFlushMappedMemoryRanges(m_device, 1, &range);
}

void BufferAllocator::invalidateMappedMemory(const BufferAllocation& allocation, VkDeviceSize offset, VkDeviceSize size) {
    if (allocation.buffer == VK_NULL_HANDLE || !allocation.mappedPtr) {
        return;
    }

    VkMappedMemoryRange range{};
    range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range.memory = allocation.memory;
    range.offset = allocation.offset + offset;
    range.size = size;

    vkInvalidateMappedMemoryRanges(m_device, 1, &range);
}

void BufferAllocator::printBufferInfo() const {
    std::cout << "Buffer Allocator Info:" << std::endl;
    std::cout << "  Total Allocated Memory: " << m_totalAllocatedMemory / 1024 / 1024 << " MB" << std::endl;
    std::cout << "  Buffer Count: " << m_bufferCount << std::endl;
    std::cout << "  Allocation Count: " << m_allocationCount << std::endl;
}

bool BufferAllocator::createBufferPool(BufferType type, VkDeviceSize size) {
    if (!m_initialized) {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_bufferMutex);
    
    BufferPool pool{};
    pool.type = type;
    pool.totalSize = size;
    pool.alignment = getAlignmentRequirement(type);
    
    createBufferPoolInternal(pool, type, size);
    
    if (pool.buffer != VK_NULL_HANDLE) {
        m_bufferPools.push_back(pool);
        return true;
    }
    
    return false;
}

BufferAllocator::BufferAllocation BufferAllocator::allocateFromPool(BufferType type, VkDeviceSize size) {
    if (!m_initialized) {
        return {};
    }

    std::lock_guard<std::mutex> lock(m_bufferMutex);
    
    // Find suitable pool
    for (auto& pool : m_bufferPools) {
        if (pool.type == type && (pool.totalSize - pool.usedSize) >= size) {
            BufferAllocation allocation{};
            allocation.buffer = pool.buffer;
            allocation.type = type;
            allocation.size = size;
            allocation.offset = pool.usedSize;
            allocation.persistentMapping = false;
            
            pool.usedSize += size;
            pool.allocations.push_back(allocation);
            
            updateBufferTracking(allocation, true);
            return allocation;
        }
    }
    
    return {};
}

void BufferAllocator::deallocateFromPool(const BufferAllocation& allocation) {
    if (!m_initialized || allocation.buffer == VK_NULL_HANDLE) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_bufferMutex);
    
    // Find the pool containing this allocation
    for (auto& pool : m_bufferPools) {
        if (pool.buffer == allocation.buffer) {
            // Remove allocation from pool's tracking
            auto it = std::remove_if(pool.allocations.begin(), pool.allocations.end(),
                [&allocation](const BufferAllocation& alloc) {
                    return alloc.offset == allocation.offset && alloc.size == allocation.size;
                });
            pool.allocations.erase(it, pool.allocations.end());
            
            // Note: In a real implementation, we would need to track free blocks
            // For now, we'll just update the used size (this is simplified)
            updateBufferTracking(allocation, false);
            break;
        }
    }
}

void BufferAllocator::cleanupBufferPools() {
    std::lock_guard<std::mutex> lock(m_bufferMutex);
    
    for (auto& pool : m_bufferPools) {
        destroyBufferPoolInternal(pool);
    }
    m_bufferPools.clear();
}

// Private implementation methods
VkDeviceSize BufferAllocator::calculateAlignment(BufferType type, VkDeviceSize size) const {
    VkDeviceSize alignment = getAlignmentRequirement(type);
    return (size + alignment - 1) & ~(alignment - 1);
}

BufferAllocator::BufferAllocation BufferAllocator::createBufferInternal(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, BufferType type) {
    BufferAllocation allocation{};
    allocation.type = type;
    allocation.size = size;
    allocation.offset = 0;
    allocation.mappedPtr = nullptr;
    allocation.persistentMapping = false;

    // Create buffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer(m_device, &bufferInfo, nullptr, &allocation.buffer);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create buffer: " << result << std::endl;
        return {};
    }

    // Get buffer memory requirements
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device, allocation.buffer, &memRequirements);

    // Allocate memory
    if (m_memoryManager) {
        allocation.memory = m_memoryManager->allocateBufferMemory(allocation.buffer, properties);
    } else {
        // Fallback allocation if memory manager is not available
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = 0; // This should be properly determined

        result = vkAllocateMemory(m_device, &allocInfo, nullptr, &allocation.memory);
        if (result != VK_SUCCESS) {
            std::cerr << "Failed to allocate buffer memory: " << result << std::endl;
            vkDestroyBuffer(m_device, allocation.buffer, nullptr);
            allocation.buffer = VK_NULL_HANDLE;
            return {};
        }
    }

    // Bind buffer to memory
    result = vkBindBufferMemory(m_device, allocation.buffer, allocation.memory, allocation.offset);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to bind buffer memory: " << result << std::endl;
        vkDestroyBuffer(m_device, allocation.buffer, nullptr);
        vkFreeMemory(m_device, allocation.memory, nullptr);
        allocation.buffer = VK_NULL_HANDLE;
        allocation.memory = VK_NULL_HANDLE;
        return {};
    }

    // Store in tracking structures
    m_allocations.push_back(allocation);
    m_bufferMap[allocation.buffer] = allocation;
    updateBufferTracking(allocation, true);

    return allocation;
}

void BufferAllocator::destroyBufferInternal(const BufferAllocation& allocation) {
    if (allocation.buffer == VK_NULL_HANDLE) {
        return;
    }

    // Unmap if mapped
    if (allocation.mappedPtr) {
        vkUnmapMemory(m_device, allocation.memory);
        const_cast<BufferAllocation&>(allocation).mappedPtr = nullptr;
    }

    // Destroy buffer
    vkDestroyBuffer(m_device, allocation.buffer, nullptr);
    const_cast<VkBuffer&>(allocation.buffer) = VK_NULL_HANDLE;

    // Free memory
    if (allocation.memory != VK_NULL_HANDLE) {
        if (m_memoryManager) {
            m_memoryManager->deallocateMemory(allocation.memory);
        } else {
            vkFreeMemory(m_device, allocation.memory, nullptr);
        }
        const_cast<VkDeviceMemory&>(allocation.memory) = VK_NULL_HANDLE;
    }

    // Remove from tracking
    m_bufferMap.erase(allocation.buffer);
}

void BufferAllocator::updateBufferTracking(const BufferAllocation& allocation, bool allocate) {
    if (allocate) {
        m_totalAllocatedMemory += allocation.size;
        m_bufferCount++;
        m_allocationCount++;
    } else {
        m_totalAllocatedMemory = std::max(static_cast<size_t>(0), m_totalAllocatedMemory - allocation.size);
        m_bufferCount = std::max(static_cast<size_t>(0), m_bufferCount - 1);
        m_allocationCount = std::max(static_cast<uint32_t>(0), m_allocationCount - 1);
    }
}

BufferAllocator::BufferPool* BufferAllocator::findBufferPool(BufferType type, VkDeviceSize size) {
    for (auto& pool : m_bufferPools) {
        if (pool.type == type && (pool.totalSize - pool.usedSize) >= size) {
            return &pool;
        }
    }
    return nullptr;
}

void BufferAllocator::createBufferPoolInternal(BufferPool& pool, BufferType type, VkDeviceSize size) {
    VkBufferUsageFlags usage = getBufferUsageFlags(type);
    VkMemoryPropertyFlags properties = getMemoryPropertyFlags(type);

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer(m_device, &bufferInfo, nullptr, &pool.buffer);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create pool buffer: " << result << std::endl;
        return;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device, pool.buffer, &memRequirements);

    if (m_memoryManager) {
        pool.memory = m_memoryManager->allocateBufferMemory(pool.buffer, properties);
    } else {
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = 0;

        result = vkAllocateMemory(m_device, &allocInfo, nullptr, &pool.memory);
        if (result != VK_SUCCESS) {
            std::cerr << "Failed to allocate pool memory: " << result << std::endl;
            vkDestroyBuffer(m_device, pool.buffer, nullptr);
            pool.buffer = VK_NULL_HANDLE;
            return;
        }
    }

    result = vkBindBufferMemory(m_device, pool.buffer, pool.memory, 0);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to bind pool memory: " << result << std::endl;
        vkDestroyBuffer(m_device, pool.buffer, nullptr);
        vkFreeMemory(m_device, pool.memory, nullptr);
        pool.buffer = VK_NULL_HANDLE;
        pool.memory = VK_NULL_HANDLE;
        return;
    }
}

void BufferAllocator::destroyBufferPoolInternal(BufferPool& pool) {
    if (pool.buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(m_device, pool.buffer, nullptr);
        pool.buffer = VK_NULL_HANDLE;
    }

    if (pool.memory != VK_NULL_HANDLE) {
        if (m_memoryManager) {
            m_memoryManager->deallocateMemory(pool.memory);
        } else {
            vkFreeMemory(m_device, pool.memory, nullptr);
        }
        pool.memory = VK_NULL_HANDLE;
    }

    pool.allocations.clear();
    pool.usedSize = 0;
}

} // namespace VortexEngine