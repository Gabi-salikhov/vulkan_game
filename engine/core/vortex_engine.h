#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../ecs/ecs_manager.h"
#include "../renderer/buffer_allocator.h"
#include "../renderer/pipeline_system.h"
#include "../renderer/shader_system.h"
#include "../scene/scene_manager.h"
#include "../scripting/python_engine.h"
#include "../utils/logger.h"
#include "memory_manager.h"
#include "vulkan_context.h"
#include "window.h"

namespace VortexEngine {

class VortexEngine {
   public:
    VortexEngine();
    ~VortexEngine();

    // Engine lifecycle
    bool initialize();
    void run();
    void shutdown();

    // Engine configuration
    void setWindowTitle(const std::string& title);
    void setWindowSize(int width, int height);
    void enableValidationLayers(bool enable);
    void setEngineVersion(const std::string& version);

    // Engine state
    bool isInitialized() const { return m_initialized; }
    bool isRunning() const { return m_running; }

    // Subsystem access
    VulkanContext* getVulkanContext() { return m_vulkanContext.get(); }
    Window* getWindow() { return m_window.get(); }
    MemoryManager* getMemoryManager() { return m_memoryManager.get(); }
    ShaderSystem* getShaderSystem() { return m_shaderSystem.get(); }
    PipelineSystem* getPipelineSystem() { return m_pipelineSystem.get(); }
    BufferAllocator* getBufferAllocator() { return m_bufferAllocator.get(); }
    ECSManager* getECSManager() { return m_ecsManager.get(); }
    SceneManager* getSceneManager() { return m_sceneManager.get(); }
    PythonEngine* getPythonEngine() { return m_pythonEngine.get(); }

   private:
    // Engine state
    bool m_initialized = false;
    bool m_running = false;
    bool m_validationLayersEnabled = true;

    // Configuration
    std::string m_windowTitle = "Vortex Engine";
    int m_windowWidth = 1280;
    int m_windowHeight = 720;
    std::string m_engineVersion = "1.0.0";

    // Subsystems
    std::unique_ptr<VulkanContext> m_vulkanContext;
    std::unique_ptr<Window> m_window;
    std::unique_ptr<MemoryManager> m_memoryManager;
    std::unique_ptr<ShaderSystem> m_shaderSystem;
    std::unique_ptr<PipelineSystem> m_pipelineSystem;
    std::unique_ptr<BufferAllocator> m_bufferAllocator;
    std::unique_ptr<ECSManager> m_ecsManager;
    std::unique_ptr<SceneManager> m_sceneManager;
    std::unique_ptr<PythonEngine> m_pythonEngine;

    // Logger
    std::unique_ptr<Logger> m_logger;

    // Internal methods
    bool initializeSubsystems();
    void shutdownSubsystems();
    void renderFrame();
    void handleEvents();
    void update(float deltaTime);
};

}  // namespace VortexEngine