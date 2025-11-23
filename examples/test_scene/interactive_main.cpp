#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <iostream>
#include <chrono>
#include <cmath>
#include <cstring>
#include <vector>
#include <optional>
#include <thread>
#include <vulkan/vulkan.h>

class VortexApp {
public:
    VortexApp() {
        window = nullptr;
        vulkanInstance = VK_NULL_HANDLE;
        vulkanDevice = VK_NULL_HANDLE;
        vulkanSurface = VK_NULL_HANDLE;
        vulkanSwapchain = VK_NULL_HANDLE;
        vulkanGraphicsQueue = VK_NULL_HANDLE;
        vulkanPresentQueue = VK_NULL_HANDLE;
    }

    ~VortexApp() {
        cleanup();
    }

    bool initialize() {
        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
            return false;
        }

        // Create window
        window = SDL_CreateWindow(
            "Vortex Engine - Test Scene",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            800,
            600,
            SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN
        );
        
        if (!window) {
            std::cerr << "Failed to create SDL window: " << SDL_GetError() << std::endl;
            SDL_Quit();
            return false;
        }

        // Create Vulkan instance
        if (!createVulkanInstance()) {
            return false;
        }

        // Create Vulkan surface
        if (!createVulkanSurface()) {
            return false;
        }

        // Pick physical device
        if (!pickPhysicalDevice()) {
            return false;
        }

        // Create logical device
        if (!createLogicalDevice()) {
            return false;
        }

        // Create swap chain
        if (!createSwapChain()) {
            return false;
        }

        return true;
    }

    void run() {
        std::cout << "Starting interactive test scene..." << std::endl;
        std::cout << "You should see a window with animated background!" << std::endl;
        std::cout << "Press ESC or close the window to exit" << std::endl;
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        bool running = true;
        SDL_Event event;
        while (running) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false;
                }
            }
            
            // Simple animation - change window title with time
            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float>(currentTime - startTime).count();
            
            // Calculate animated color values
            float r = (sin(time * 0.5) + 1.0f) * 0.5f;
            float g = (sin(time * 0.7) + 1.0f) * 0.5f;
            float b = (sin(time * 0.9) + 1.0f) * 0.5f;
            
            // Update window title to show the "color"
            char title[256];
            snprintf(title, sizeof(title), "Vortex Engine - RGB(%.2f, %.2f, %.2f)", r, g, b);
            SDL_SetWindowTitle(window, title);
            
            // Print color info every second
            if ((int)time != (int)(time - 0.016)) { // Roughly every second
                std::cout << "Screen color: RGB(" << r << ", " << g << ", " << b << ")" << std::endl;
            }
            
            // Small delay to control frame rate
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }

private:
    SDL_Window* window;
    VkInstance vulkanInstance;
    VkPhysicalDevice vulkanPhysicalDevice;
    VkDevice vulkanDevice;
    VkSurfaceKHR vulkanSurface;
    VkSwapchainKHR vulkanSwapchain;
    VkQueue vulkanGraphicsQueue;
    VkQueue vulkanPresentQueue;

    bool createVulkanInstance() {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Vortex Engine Test";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Vortex Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        uint32_t extensionCount = 0;
        SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr);
        
        std::vector<const char*> extensions(extensionCount);
        SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensions.data());

        createInfo.enabledExtensionCount = extensionCount;
        createInfo.ppEnabledExtensionNames = extensions.data();
        createInfo.enabledLayerCount = 0;

        if (vkCreateInstance(&createInfo, nullptr, &vulkanInstance) != VK_SUCCESS) {
            std::cerr << "Failed to create Vulkan instance" << std::endl;
            return false;
        }

        std::cout << "Vulkan instance created successfully" << std::endl;
        return true;
    }

    bool createVulkanSurface() {
        if (!SDL_Vulkan_CreateSurface(window, vulkanInstance, &vulkanSurface)) {
            std::cerr << "Failed to create window surface" << std::endl;
            return false;
        }

        std::cout << "Vulkan surface created successfully" << std::endl;
        return true;
    }

    bool pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, nullptr);
        
        if (deviceCount == 0) {
            std::cerr << "Failed to find GPUs with Vulkan support" << std::endl;
            return false;
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, devices.data());

        // Pick the first suitable device
        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                vulkanPhysicalDevice = device;
                break;
            }
        }

        if (vulkanPhysicalDevice == VK_NULL_HANDLE) {
            std::cerr << "Failed to find a suitable GPU" << std::endl;
            return false;
        }

        std::cout << "Vulkan physical device selected" << std::endl;
        return true;
    }

    bool isDeviceSuitable(VkPhysicalDevice device) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    }

    bool createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(vulkanPhysicalDevice);

        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;
        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceFeatures deviceFeatures{};

        // Required device extensions for swapchain support
        const char* deviceExtensions[] = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = 1;
        createInfo.ppEnabledExtensionNames = deviceExtensions;
        createInfo.enabledLayerCount = 0;

        if (vkCreateDevice(vulkanPhysicalDevice, &createInfo, nullptr, &vulkanDevice) != VK_SUCCESS) {
            std::cerr << "Failed to create logical device" << std::endl;
            return false;
        }

        vkGetDeviceQueue(vulkanDevice, indices.graphicsFamily.value(), 0, &vulkanGraphicsQueue);
        vkGetDeviceQueue(vulkanDevice, indices.presentFamily.value(), 0, &vulkanPresentQueue);

        std::cout << "Vulkan logical device created successfully" << std::endl;
        return true;
    }

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vulkanSurface, &presentSupport);

            if (presentSupport) {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }

    bool createSwapChain() {
        // Simplified swap chain creation
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(vulkanPhysicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = vulkanSurface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(vulkanPhysicalDevice);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(vulkanDevice, &createInfo, nullptr, &vulkanSwapchain) != VK_SUCCESS) {
            std::cerr << "Failed to create swap chain" << std::endl;
            return false;
        }

        std::cout << "Vulkan swap chain created successfully" << std::endl;
        return true;
    }

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vulkanSurface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkanSurface, &formatCount, nullptr);
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, vulkanSurface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkanSurface, &presentModeCount, nullptr);
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, vulkanSurface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            SDL_GetWindowSize(window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }

    void cleanup() {
        if (vulkanSwapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(vulkanDevice, vulkanSwapchain, nullptr);
        }

        if (vulkanDevice != VK_NULL_HANDLE) {
            vkDestroyDevice(vulkanDevice, nullptr);
        }

        if (vulkanSurface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(vulkanInstance, vulkanSurface, nullptr);
        }

        if (vulkanInstance != VK_NULL_HANDLE) {
            vkDestroyInstance(vulkanInstance, nullptr);
        }

        if (window) {
            SDL_DestroyWindow(window);
        }

        SDL_Quit();
    }
};

int main() {
    std::cout << "Starting Vortex Engine Interactive Test Scene" << std::endl;
    
    VortexApp app;
    
    if (!app.initialize()) {
        std::cerr << "Failed to initialize application" << std::endl;
        return -1;
    }
    
    std::cout << "Engine initialized successfully" << std::endl;
    std::cout << "Window created - you should see a window now!" << std::endl;
    std::cout << "Press ESC or close the window to exit" << std::endl;

    app.run();

    std::cout << "Shutting down engine" << std::endl;
    return 0;
}