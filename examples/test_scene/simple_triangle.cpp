#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <vulkan/vulkan.h>

// Simple vertex data for a triangle
const std::vector<float> triangleVertices = {
    // positions           // colors
    -0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f, 1.0f,  // bottom left
     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f, 1.0f,  // bottom right
     0.0f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f, 1.0f   // top
};

class SimpleTriangleApp {
public:
    SimpleTriangleApp() {
        std::cout << "Initializing Simple Triangle Application..." << std::endl;
    }

    ~SimpleTriangleApp() {
        cleanup();
    }

    bool initialize() {
        // Initialize GLFW
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }

        // Create window
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        
        window = glfwCreateWindow(800, 600, "Vortex Engine - Simple Triangle", nullptr, nullptr);
        if (!window) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }

        // Create Vulkan instance
        if (!createVulkanInstance()) {
            return false;
        }

        std::cout << "Simple Triangle Application initialized successfully" << std::endl;
        return true;
    }

    void run() {
        std::cout << "Starting Simple Triangle Demo..." << std::endl;
        std::cout << "You should see a window with a title!" << std::endl;
        std::cout << "Press ESC or close the window to exit" << std::endl;

        auto lastTime = std::chrono::high_resolution_clock::now();
        float deltaTime = 0.0f;
        int frameCount = 0;

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            
            auto currentTime = std::chrono::high_resolution_clock::now();
            deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;

            // Print frame info occasionally
            frameCount++;
            if (frameCount % 60 == 0) {
                std::cout << "Frame " << frameCount << " - Time: " << deltaTime * 1000.0f << " ms" << std::endl;
            }

            // Small delay to control frame rate
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }

        std::cout << "Simple Triangle Demo ended" << std::endl;
    }

private:
    void cleanup() {
        std::cout << "Cleaning up Simple Triangle Application..." << std::endl;

        if (vulkanInstance != VK_NULL_HANDLE) {
            vkDestroyInstance(vulkanInstance, nullptr);
            vulkanInstance = VK_NULL_HANDLE;
        }

        if (window) {
            glfwDestroyWindow(window);
            window = nullptr;
        }

        glfwTerminate();
        std::cout << "Cleanup complete" << std::endl;
    }

    bool createVulkanInstance() {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Vortex Engine Simple Demo";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Vortex Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
        createInfo.enabledLayerCount = 0;

        if (vkCreateInstance(&createInfo, nullptr, &vulkanInstance) != VK_SUCCESS) {
            std::cerr << "Failed to create Vulkan instance" << std::endl;
            return false;
        }

        std::cout << "Vulkan instance created successfully" << std::endl;
        return true;
    }

    GLFWwindow* window = nullptr;
    VkInstance vulkanInstance = VK_NULL_HANDLE;
};

int main() {
    std::cout << "Starting Vortex Engine Simple Triangle Demo" << std::endl;
    
    SimpleTriangleApp app;
    
    if (!app.initialize()) {
        std::cerr << "Failed to initialize application" << std::endl;
        return -1;
    }
    
    std::cout << "Application initialized successfully" << std::endl;
    std::cout << "Window created - you should see a window now!" << std::endl;
    std::cout << "Press ESC or close the window to exit" << std::endl;

    app.run();

    std::cout << "Shutting down application" << std::endl;
    return 0;
}