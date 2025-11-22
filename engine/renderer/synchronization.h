#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <mutex>

namespace VortexEngine {

class Fence {
public:
    Fence(VkDevice device);
    ~Fence();

    // Fence lifecycle
    bool create(bool signaled = false);
    void destroy();
    
    // Fence operations
    void reset();
    void wait(uint64_t timeout = std::numeric_limits<uint64_t>::max());
    bool isSignaled();
    
    // Accessors
    VkFence getHandle() const { return m_fence; }
    bool isValid() const { return m_fence != VK_NULL_HANDLE; }

private:
    VkDevice m_device;
    VkFence m_fence;
    bool m_isValid;
};

class Semaphore {
public:
    Semaphore(VkDevice device);
    ~Semaphore();

    // Semaphore lifecycle
    bool create();
    void destroy();
    
    // Accessors
    VkSemaphore getHandle() const { return m_semaphore; }
    bool isValid() const { return m_semaphore != VK_NULL_HANDLE; }

private:
    VkDevice m_device;
    VkSemaphore m_semaphore;
    bool m_isValid;
};

class SyncObjects {
public:
    SyncObjects(VkDevice device, uint32_t maxFramesInFlight = 2);
    ~SyncObjects();

    // Sync objects lifecycle
    bool create();
    void destroy();
    
    // Frame synchronization
    void waitForFrame();
    void beginFrame();
    void endFrame();
    
    // Accessors
    uint32_t getCurrentFrame() const { return m_currentFrame; }
    uint32_t getMaxFramesInFlight() const { return m_maxFramesInFlight; }
    VkSemaphore getImageAvailableSemaphore() const { return m_imageAvailableSemaphores[m_currentFrame]; }
    VkSemaphore getRenderFinishedSemaphore() const { return m_renderFinishedSemaphores[m_currentFrame]; }
    VkFence getInFlightFence() const { return m_inFlightFences[m_currentFrame]; }
    
    // Frame management
    void nextFrame();
    bool isFrameComplete() const;

private:
    VkDevice m_device;
    uint32_t m_maxFramesInFlight;
    uint32_t m_currentFrame;
    uint32_t m_framesInFlight;
    
    // Synchronization objects
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    std::vector<VkFence> m_imagesInFlight;
    
    // Management
    std::mutex m_mutex;
    bool m_isValid;
    
    // Helper methods
    bool createSemaphores();
    bool createFences();
    void cleanupSemaphores();
    void cleanupFences();
};

} // namespace VortexEngine