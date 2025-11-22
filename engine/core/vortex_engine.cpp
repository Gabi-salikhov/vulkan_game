#include "vortex_engine.h"
#include <iostream>
#include <chrono>

namespace VortexEngine {

VortexEngine::VortexEngine() {
    VORTEX_INFO("Initializing Vortex Engine...");
}

VortexEngine::~VortexEngine() {
    shutdown();
}

bool VortexEngine::initialize() {
    if (m_initialized) {
        VORTEX_WARNING("Engine is already initialized");
        return true;
    }

    try {
        // Initialize logger first
        m_logger = std::make_unique<Logger>();
        if (!m_logger->initialize()) {
            VORTEX_ERROR("Failed to initialize logger");
            return false;
        }

        VORTEX_INFO("Logger initialized successfully");

        // Initialize subsystems
        if (!initializeSubsystems()) {
            VORTEX_ERROR("Failed to initialize subsystems");
            return false;
        }

        m_initialized = true;
        VORTEX_INFO("Vortex Engine initialized successfully");
        return true;
    }
    catch (const std::exception& e) {
        VORTEX_ERROR("Exception during engine initialization: " << e.what());
        return false;
    }
}

void VortexEngine::run() {
    if (!m_initialized) {
        VORTEX_ERROR("Engine is not initialized");
        return;
    }

    if (m_running) {
        VORTEX_WARNING("Engine is already running");
        return;
    }

    m_running = true;
    VORTEX_INFO("Starting engine main loop");

    auto lastTime = std::chrono::high_resolution_clock::now();
    float deltaTime = 0.0f;

    while (m_running && m_window->shouldClose()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Handle input events
        handleEvents();

        // Update engine
        update(deltaTime);

        // Render frame
        renderFrame();

        // Update window
        m_window->swapBuffers();
        m_window->pollEvents();
    }

    VORTEX_INFO("Engine main loop ended");
}

void VortexEngine::shutdown() {
    if (!m_initialized) {
        return;
    }

    VORTEX_INFO("Shutting down Vortex Engine...");

    m_running = false;

    // Shutdown subsystems in reverse order
    shutdownSubsystems();

    m_initialized = false;
    VORTEX_INFO("Vortex Engine shutdown complete");
}

void VortexEngine::setWindowTitle(const std::string& title) {
    m_windowTitle = title;
    if (m_window && m_window->isInitialized()) {
        m_window->setWindowTitle(title);
    }
}

void VortexEngine::setWindowSize(int width, int height) {
    m_windowWidth = width;
    m_windowHeight = height;
    if (m_window && m_window->isInitialized()) {
        m_window->setWindowSize(width, height);
    }
}

void VortexEngine::enableValidationLayers(bool enable) {
    m_validationLayersEnabled = enable;
    if (m_vulkanContext && m_vulkanContext->isInitialized()) {
        m_vulkanContext->enableValidationLayers(enable);
    }
}

void VortexEngine::setEngineVersion(const std::string& version) {
    m_engineVersion = version;
}

bool VortexEngine::initializeSubsystems() {
    VORTEX_INFO("Initializing subsystems...");

    try {
        // Initialize window
        m_window = std::make_unique<Window>();
        if (!m_window->initialize(m_windowTitle, m_windowWidth, m_windowHeight)) {
            VORTEX_ERROR("Failed to initialize window");
            return false;
        }
        VORTEX_INFO("Window initialized successfully");

        // Initialize Vulkan context
        m_vulkanContext = std::make_unique<VulkanContext>();
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.enabledExtensionCount = 0;
        createInfo.ppEnabledExtensionNames = nullptr;
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;

        if (!m_vulkanContext->initialize(createInfo)) {
            VORTEX_ERROR("Failed to initialize Vulkan context");
            return false;
        }
        VORTEX_INFO("Vulkan context initialized successfully");

        // Initialize memory manager
        m_memoryManager = std::make_unique<MemoryManager>();
        if (!m_memoryManager->initialize(m_vulkanContext->getDevice(), m_vulkanContext->getPhysicalDevice())) {
            VORTEX_ERROR("Failed to initialize memory manager");
            return false;
        }
        VORTEX_INFO("Memory manager initialized successfully");

        // Initialize shader system
        m_shaderSystem = std::make_unique<ShaderSystem>();
        if (!m_shaderSystem->initialize(m_vulkanContext->getDevice())) {
            VORTEX_ERROR("Failed to initialize shader system");
            return false;
        }
        VORTEX_INFO("Shader system initialized successfully");

        // Initialize pipeline system
        m_pipelineSystem = std::make_unique<PipelineSystem>();
        if (!m_pipelineSystem->initialize(m_vulkanContext->getDevice(), nullptr)) {
            VORTEX_ERROR("Failed to initialize pipeline system");
            return false;
        }
        VORTEX_INFO("Pipeline system initialized successfully");

        // Initialize buffer allocator
        m_bufferAllocator = std::make_unique<BufferAllocator>();
        if (!m_bufferAllocator->initialize(m_vulkanContext->getDevice(), m_vulkanContext->getPhysicalDevice(), m_memoryManager.get())) {
            VORTEX_ERROR("Failed to initialize buffer allocator");
            return false;
        }
        VORTEX_INFO("Buffer allocator initialized successfully");

        // Initialize ECS manager
        m_ecsManager = std::make_unique<ECSManager>();
        m_ecsManager->initialize();
        VORTEX_INFO("ECS manager initialized successfully");

        // Initialize scene manager
        m_sceneManager = std::make_unique<SceneManager>();
        if (!m_sceneManager->initialize(m_ecsManager.get())) {
            VORTEX_ERROR("Failed to initialize scene manager");
            return false;
        }
        VORTEX_INFO("Scene manager initialized successfully");

        // Initialize Python engine
        m_pythonEngine = std::make_unique<PythonEngine>();
        if (!m_pythonEngine->initialize()) {
            VORTEX_ERROR("Failed to initialize Python engine");
            return false;
        }
        VORTEX_INFO("Python engine initialized successfully");

        VORTEX_INFO("All subsystems initialized successfully");
        return true;
    }
    catch (const std::exception& e) {
        VORTEX_ERROR("Exception during subsystem initialization: " << e.what());
        return false;
    }
}

void VortexEngine::shutdownSubsystems() {
    VORTEX_INFO("Shutting down subsystems...");

    // Shutdown in reverse order of initialization
    if (m_pythonEngine) {
        m_pythonEngine->shutdown();
        VORTEX_INFO("Python engine shutdown");
    }

    if (m_sceneManager) {
        m_sceneManager->shutdown();
        VORTEX_INFO("Scene manager shutdown");
    }

    if (m_ecsManager) {
        m_ecsManager->shutdown();
        VORTEX_INFO("ECS manager shutdown");
    }

    if (m_bufferAllocator) {
        m_bufferAllocator->shutdown();
        VORTEX_INFO("Buffer allocator shutdown");
    }

    if (m_pipelineSystem) {
        m_pipelineSystem->shutdown();
        VORTEX_INFO("Pipeline system shutdown");
    }

    if (m_shaderSystem) {
        m_shaderSystem->shutdown();
        VORTEX_INFO("Shader system shutdown");
    }

    if (m_memoryManager) {
        m_memoryManager->shutdown();
        VORTEX_INFO("Memory manager shutdown");
    }

    if (m_vulkanContext) {
        m_vulkanContext->shutdown();
        VORTEX_INFO("Vulkan context shutdown");
    }

    if (m_window) {
        m_window->shutdown();
        VORTEX_INFO("Window shutdown");
    }

    VORTEX_INFO("All subsystems shutdown complete");
}

void VortexEngine::renderFrame() {
    // This is a simplified render frame implementation
    // In a real engine, this would handle:
    // - Command buffer recording
    // - Render pass begin/end
    // - Pipeline binding
    // - Draw calls
    // - Synchronization

    VORTEX_DEBUG("Rendering frame...");
}

void VortexEngine::handleEvents() {
    // This would handle window events, input events, etc.
    // For now, it's a placeholder
}

void VortexEngine::update(float deltaTime) {
    // Update all systems
    if (m_ecsManager) {
        // Update ECS systems
        m_ecsManager->updateSystems(deltaTime);
    }

    if (m_sceneManager) {
        // Update scene
        m_sceneManager->update(deltaTime);
    }

    if (m_pythonEngine) {
        // Update Python scripts
        m_pythonEngine->checkForScriptUpdates();
    }
}

} // namespace VortexEngine