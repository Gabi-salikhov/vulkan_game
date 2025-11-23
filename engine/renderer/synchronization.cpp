#include "synchronization.h"
#include <iostream>
#include <stdexcept>
#include <limits>

namespace VortexEngine {

// Fence implementation
Fence::Fence(VkDevice device)
    : m_device(device)
    , m_fence(VK_NULL_HANDLE)
    , m_isValid(false) {
}

Fence::~Fence() {
    destroy();
}

bool Fence::create(bool signaled) {
    if (m_fence != VK_NULL_HANDLE) {
        std::cout << "Fence already created" << std::endl;
        return true;
    }

    VkFenceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    createInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

    VkResult result = vkCreateFence(m_device, &createInfo, nullptr, &m_fence);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create fence: " << result << std::endl;
        return false;
    }

    m_isValid = true;
    std::cout << "Fence created successfully" << std::endl;
    return true;
}

void Fence::destroy() {
    if (m_fence != VK_NULL_HANDLE) {
        vkDestroyFence(m_device, m_fence, nullptr);
        m_fence = VK_NULL_HANDLE;
        m_isValid = false;
    }
}

void Fence::reset() {
    if (!m_isValid) {
        std::cerr << "Cannot reset invalid fence" << std::endl;
        return;
    }

    VkResult result = vkResetFences(m_device, 1, &m_fence);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to reset fence: " << result << std::endl;
    }
}

void Fence::wait(uint64_t timeout) {
    if (!m_isValid) {
        std::cerr << "Cannot wait on invalid fence" << std::endl;
        return;
    }

    VkResult result = vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, timeout);
    if (result != VK_SUCCESS && result != VK_TIMEOUT) {
        std::cerr << "Failed to wait for fence: " << result << std::endl;
    }
}

bool Fence::isSignaled() {
    if (!m_isValid) {
        return false;
    }

    VkResult result = vkGetFenceStatus(m_device, m_fence);
    return result == VK_SUCCESS;
}

// Semaphore implementation
Semaphore::Semaphore(VkDevice device)
    : m_device(device)
    , m_semaphore(VK_NULL_HANDLE)
    , m_isValid(false) {
}

Semaphore::~Semaphore() {
    destroy();
}

bool Semaphore::create() {
    if (m_semaphore != VK_NULL_HANDLE) {
        std::cout << "Semaphore already created" << std::endl;
        return true;
    }

    VkSemaphoreCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkResult result = vkCreateSemaphore(m_device, &createInfo, nullptr, &m_semaphore);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create semaphore: " << result << std::endl;
        return false;
    }

    m_isValid = true;
    std::cout << "Semaphore created successfully" << std::endl;
    return true;
}

void Semaphore::destroy() {
    if (m_semaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(m_device, m_semaphore, nullptr);
        m_semaphore = VK_NULL_HANDLE;
        m_isValid = false;
    }
}

// SyncObjects implementation
SyncObjects::SyncObjects(VkDevice device, uint32_t maxFramesInFlight)
    : m_device(device)
    , m_maxFramesInFlight(maxFramesInFlight)
    , m_currentFrame(0)
    , m_framesInFlight(0)
    , m_isValid(false) {
}

SyncObjects::~SyncObjects() {
    destroy();
}

bool SyncObjects::create() {
    if (m_isValid) {
        std::cout << "Sync objects already created" << std::endl;
        return true;
    }

    try {
        if (!createSemaphores()) {
            return false;
        }

        if (!createFences()) {
            cleanupSemaphores();
            return false;
        }

        m_isValid = true;
        std::cout << "Sync objects created successfully" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception during sync objects creation: " << e.what() << std::endl;
        cleanupSemaphores();
        cleanupFences();
        return false;
    }
}

void SyncObjects::destroy() {
    if (!m_isValid) {
        return;
    }

    std::cout << "Destroying sync objects..." << std::endl;
    
    cleanupSemaphores();
    cleanupFences();
    
    m_imageAvailableSemaphores.clear();
    m_renderFinishedSemaphores.clear();
    m_inFlightFences.clear();
    m_imagesInFlight.clear();
    
    m_isValid = false;
    std::cout << "Sync objects destroyed" << std::endl;
}

void SyncObjects::waitForFrame() {
    if (!m_isValid) {
        std::cerr << "Cannot wait for frame - sync objects not created" << std::endl;
        return;
    }

    // Wait for the fence for the current frame to be signaled
    if (m_framesInFlight > 0) {
        vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, 
                       std::numeric_limits<uint64_t>::max());
    }
}

void SyncObjects::beginFrame() {
    if (!m_isValid) {
        std::cerr << "Cannot begin frame - sync objects not created" << std::endl;
        return;
    }

    waitForFrame();

    // Reset the fence for the current frame
    vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]);
    
    m_framesInFlight++;
}

void SyncObjects::endFrame() {
    if (!m_isValid) {
        std::cerr << "Cannot end frame - sync objects not created" << std::endl;
        return;
    }

    m_framesInFlight--;
}

bool SyncObjects::isFrameComplete() const {
    return m_framesInFlight == 0;
}

void SyncObjects::nextFrame() {
    if (!m_isValid) {
        std::cerr << "Cannot advance frame - sync objects not created" << std::endl;
        return;
    }

    m_currentFrame = (m_currentFrame + 1) % m_maxFramesInFlight;
}

bool SyncObjects::createSemaphores() {
    m_imageAvailableSemaphores.resize(m_maxFramesInFlight);
    m_renderFinishedSemaphores.resize(m_maxFramesInFlight);

    for (size_t i = 0; i < m_maxFramesInFlight; i++) {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkResult result = vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, 
                                          &m_imageAvailableSemaphores[i]);
        if (result != VK_SUCCESS) {
            std::cerr << "Failed to create image available semaphore: " << result << std::endl;
            return false;
        }

        result = vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, 
                                  &m_renderFinishedSemaphores[i]);
        if (result != VK_SUCCESS) {
            std::cerr << "Failed to create render finished semaphore: " << result << std::endl;
            return false;
        }
    }

    std::cout << "Created " << m_maxFramesInFlight << " image available semaphores" << std::endl;
    std::cout << "Created " << m_maxFramesInFlight << " render finished semaphores" << std::endl;
    return true;
}

bool SyncObjects::createFences() {
    m_inFlightFences.resize(m_maxFramesInFlight);
    m_imagesInFlight.resize(m_maxFramesInFlight, VK_NULL_HANDLE);

    for (size_t i = 0; i < m_maxFramesInFlight; i++) {
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkResult result = vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFences[i]);
        if (result != VK_SUCCESS) {
            std::cerr << "Failed to create in-flight fence: " << result << std::endl;
            return false;
        }
    }

    std::cout << "Created " << m_maxFramesInFlight << " in-flight fences" << std::endl;
    return true;
}

void SyncObjects::cleanupSemaphores() {
    for (size_t i = 0; i < m_imageAvailableSemaphores.size(); i++) {
        if (m_imageAvailableSemaphores[i] != VK_NULL_HANDLE) {
            vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
            m_imageAvailableSemaphores[i] = VK_NULL_HANDLE;
        }
    }

    for (size_t i = 0; i < m_renderFinishedSemaphores.size(); i++) {
        if (m_renderFinishedSemaphores[i] != VK_NULL_HANDLE) {
            vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
            m_renderFinishedSemaphores[i] = VK_NULL_HANDLE;
        }
    }
}

void SyncObjects::cleanupFences() {
    for (size_t i = 0; i < m_inFlightFences.size(); i++) {
        if (m_inFlightFences[i] != VK_NULL_HANDLE) {
            vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
            m_inFlightFences[i] = VK_NULL_HANDLE;
        }
    }
}

} // namespace VortexEngine