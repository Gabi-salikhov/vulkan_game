#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <string>

namespace VortexEngine {

// Forward declarations
struct SwapChainSupportDetails;
struct SwapChainDetails;

class VulkanContext {
public:
    VulkanContext();
    ~VulkanContext();

    // Vulkan context lifecycle
    bool initialize(const VkInstanceCreateInfo& createInfo);
    void shutdown();

    // Vulkan instance management
    VkInstance getInstance() const { return m_instance; }
    bool isInitialized() const { return m_initialized; }

    // Physical device selection
    VkPhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }
    uint32_t getGraphicsQueueFamilyIndex() const { return m_graphicsQueueFamilyIndex; }
    uint32_t getPresentQueueFamilyIndex() const { return m_presentQueueFamilyIndex; }

    // Logical device management
    VkDevice getDevice() const { return m_device; }
    VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
    VkQueue getPresentQueue() const { return m_presentQueue; }

    // Validation layers
    void enableValidationLayers(bool enable);
    bool areValidationLayersEnabled() const { return m_validationLayersEnabled; }
    const std::vector<const char*>& getValidationLayers() const { return m_validationLayers; }

    // Extension management
    void addExtension(const char* extension);
    const std::vector<const char*>& getInstanceExtensions() const { return m_instanceExtensions; }

    // Debug messenger
    bool setupDebugMessenger();
    void cleanupDebugMessenger();

    // Physical device selection
    bool selectPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    uint32_t findQueueFamilies(VkPhysicalDevice device);

    // Device features
    VkPhysicalDeviceFeatures getEnabledFeatures() const { return m_enabledFeatures; }
    void setEnabledFeatures(const VkPhysicalDeviceFeatures& features) { m_enabledFeatures = features; }
    
    // Device creation
    void createLogicalDevice();
    
    // Device properties
    std::string getPhysicalDeviceName() const;

    // Swapchain management
    bool createSwapChain(VkSurfaceKHR surface);
    void recreateSwapChain(VkSurfaceKHR surface);
    void cleanupSwapChain();
    void setSurface(VkSurfaceKHR surface) { m_surface = surface; }
    
    // Swapchain accessors
    VkSwapchainKHR getSwapChain() const { return m_swapChain; }
    const std::vector<VkImage>& getSwapChainImages() const { return m_swapChainImages; }
    const std::vector<VkImageView>& getSwapChainImageViews() const { return m_swapChainImageViews; }
    VkFormat getSwapChainImageFormat() const { return m_swapChainImageFormat; }
    VkExtent2D getSwapChainExtent() const { return m_swapChainExtent; }
    uint32_t getSwapChainImageCount() const { return static_cast<uint32_t>(m_swapChainImages.size()); }
    
    // Frame presentation
    uint32_t acquireNextImage(VkSemaphore semaphore);
    bool presentFrame(uint32_t imageIndex, VkSemaphore waitSemaphore);

private:
    // Vulkan objects
    VkInstance m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;

    // Queue family indices
    uint32_t m_graphicsQueueFamilyIndex = UINT32_MAX;
    uint32_t m_presentQueueFamilyIndex = UINT32_MAX;

    // Configuration
    bool m_initialized = false;
    bool m_validationLayersEnabled = true;

    // Validation layers
    std::vector<const char*> m_validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    // Extensions
    std::vector<const char*> m_instanceExtensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    };

    // Device features
    VkPhysicalDeviceFeatures m_enabledFeatures{};

    // Swapchain objects
    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;

    // Debug messenger setup
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    );

    VkResult createDebugUtilsMessengerEXT(
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator
    );

    void destroyDebugUtilsMessengerEXT(
        const VkAllocationCallbacks* pAllocator
    );

    // Physical device utilities
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;
    
    // Swapchain utilities
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    void createSwapChainImages(VkSurfaceKHR surface);
    void createSwapChainImageViews();
};

// Structure to hold swap chain support details
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

// Structure to hold swap chain details
struct SwapChainDetails {
    VkFormat format;
    VkExtent2D extent;
    VkPresentModeKHR presentMode;
};

} // namespace VortexEngine