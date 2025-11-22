#include "vortex_engine.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <vulkan/vulkan.h>

// Simple vertex data for a triangle
const std::vector<float> triangleVertices = {
    // positions           // colors
    -0.5f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f, 1.0f,  // bottom left
     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f, 1.0f,  // bottom right
     0.0f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f, 1.0f   // top
};

class VortexTriangleApp {
public:
    VortexTriangleApp() {
        std::cout << "Initializing Vortex Triangle Application..." << std::endl;
    }

    ~VortexTriangleApp() {
        shutdown();
    }

    bool initialize() {
        try {
            // Initialize Vortex Engine
            m_engine = std::make_unique<VortexEngine>();
            
            // Configure engine
            m_engine->setWindowTitle("Vortex Engine - Triangle Demo");
            m_engine->setWindowSize(800, 600);
            m_engine->enableValidationLayers(true);
            
            // Initialize engine
            if (!m_engine->initialize()) {
                std::cerr << "Failed to initialize Vortex Engine" << std::endl;
                return false;
            }

            std::cout << "Vortex Engine initialized successfully" << std::endl;

            // Get subsystems
            auto vulkanContext = m_engine->getVulkanContext();
            auto bufferAllocator = m_engine->getBufferAllocator();
            auto shaderSystem = m_engine->getShaderSystem();
            auto pipelineSystem = m_engine->getPipelineSystem();

            // Create vertex buffer
            auto vertexBuffer = bufferAllocator->createVertexBuffer(
                triangleVertices.size() * sizeof(float),
                triangleVertices.data()
            );

            if (vertexBuffer.buffer == VK_NULL_HANDLE) {
                std::cerr << "Failed to create vertex buffer" << std::endl;
                return false;
            }

            std::cout << "Vertex buffer created successfully" << std::endl;

            // Load shaders
            if (!m_shaderSystem->loadShader("triangle", 
                "shaders/common/common.vert", 
                "shaders/common/common.frag")) {
                std::cerr << "Failed to load shaders" << std::endl;
                return false;
            }

            std::cout << "Shaders loaded successfully" << std::endl;

            m_initialized = true;
            return true;
        }
        catch (const std::exception& e) {
            std::cerr << "Exception during initialization: " << e.what() << std::endl;
            return false;
        }
    }

    void run() {
        if (!m_initialized) {
            std::cerr << "Application is not initialized" << std::endl;
            return;
        }

        std::cout << "Starting Vortex Triangle Demo..." << std::endl;
        std::cout << "You should see a window with a colored triangle!" << std::endl;
        std::cout << "Press ESC or close the window to exit" << std::endl;

        auto lastTime = std::chrono::high_resolution_clock::now();
        float deltaTime = 0.0f;

        while (!m_engine->getWindow()->shouldClose()) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;

            // Handle input
            handleInput();

            // Update
            update(deltaTime);

            // Render
            render();

            // Update window
            m_engine->getWindow()->swapBuffers();
            m_engine->getWindow()->pollEvents();
        }

        std::cout << "Vortex Triangle Demo ended" << std::endl;
    }

    void shutdown() {
        if (!m_initialized) {
            return;
        }

        std::cout << "Shutting down Vortex Triangle Application..." << std::endl;

        // Cleanup resources
        if (vertexBuffer.buffer != VK_NULL_HANDLE) {
            bufferAllocator->deallocateBuffer(vertexBuffer);
        }

        m_initialized = false;
        std::cout << "Vortex Triangle Application shutdown complete" << std::endl;
    }

private:
    void handleInput() {
        // Simple input handling
        auto window = m_engine->getWindow();
        if (window->isKeyPressed(GLFW_KEY_ESCAPE)) {
            std::cout << "ESC pressed, closing window..." << std::endl;
        }
    }

    void update(float deltaTime) {
        // Update logic would go here
        // For now, just print the frame time occasionally
        static float timeAccumulator = 0.0f;
        timeAccumulator += deltaTime;
        
        if (timeAccumulator >= 1.0f) {
            std::cout << "Frame time: " << deltaTime * 1000.0f << " ms" << std::endl;
            timeAccumulator = 0.0f;
        }
    }

    void render() {
        // This is a simplified render function
        // In a real implementation, this would:
        // 1. Acquire swapchain image
        // 2. Begin render pass
        // 3. Bind pipeline and vertex buffer
        // 4. Draw the triangle
        // 5. End render pass
        // 6. Present the image
        
        // For now, just indicate that we're rendering
        static int frameCount = 0;
        if (frameCount % 60 == 0) { // Print every 60 frames
            std::cout << "Rendering frame " << frameCount << std::endl;
        }
        frameCount++;
    }

    // Member variables
    std::unique_ptr<VortexEngine> m_engine;
    
    bool m_initialized = false;
};

int main() {
    std::cout << "Starting Vortex Engine Triangle Demo" << std::endl;
    
    VortexTriangleApp app;
    
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