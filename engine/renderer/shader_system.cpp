#include "shader_system.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <cstring>
#include <mutex>

namespace VortexEngine {

ShaderSystem::ShaderSystem() {
    std::cout << "Initializing shader system..." << std::endl;
}

ShaderSystem::~ShaderSystem() {
    shutdown();
}

bool ShaderSystem::initialize(VkDevice device) {
    if (m_initialized) {
        std::cout << "Shader system is already initialized" << std::endl;
        return true;
    }

    try {
        m_device = device;

        // Initialize shader cache
        m_shaderCacheEnabled = true;
        m_hotReloadEnabled = false;

        m_initialized = true;
        std::cout << "Shader system initialized successfully" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception during shader system initialization: " << e.what() << std::endl;
        return false;
    }
}

void ShaderSystem::shutdown() {
    if (!m_initialized) {
        return;
    }

    std::cout << "Shutting down shader system..." << std::endl;

    // Stop file watcher if running
    stopFileWatcher();

    // Destroy all shader modules
    cleanupShaders();

    m_initialized = false;
    std::cout << "Shader system shutdown complete" << std::endl;
}

VkShaderModule ShaderSystem::createShaderModule(const std::vector<char>& code) {
    if (!m_initialized || code.empty()) {
        return VK_NULL_HANDLE;
    }

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    VkResult result = vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create shader module: " << result << std::endl;
        return VK_NULL_HANDLE;
    }

    return shaderModule;
}

void ShaderSystem::destroyShaderModule(VkShaderModule shaderModule) {
    if (!m_initialized || shaderModule == VK_NULL_HANDLE) {
        return;
    }

    vkDestroyShaderModule(m_device, shaderModule, nullptr);
}

bool ShaderSystem::loadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) {
    if (!m_initialized) {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_shaderMutex);

    // Check if shader already exists
    if (m_shaders.find(name) != m_shaders.end()) {
        std::cout << "Shader '" << name << "' already exists, reloading..." << std::endl;
        unloadShader(name);
    }

    ShaderData shaderData;
    shaderData.vertexPath = vertexPath;
    shaderData.fragmentPath = fragmentPath;

    // Load vertex shader
    std::vector<char> vertexCode;
    if (!loadShaderFile(vertexPath, vertexCode)) {
        std::cerr << "Failed to load vertex shader: " << vertexPath << std::endl;
        return false;
    }

    // Load fragment shader
    std::vector<char> fragmentCode;
    if (!loadShaderFile(fragmentPath, fragmentCode)) {
        std::cerr << "Failed to load fragment shader: " << fragmentPath << std::endl;
        return false;
    }

    // Create shader modules
    shaderData.vertexShader = createShaderModule(vertexCode);
    if (shaderData.vertexShader == VK_NULL_HANDLE) {
        std::cerr << "Failed to create vertex shader module for: " << name << std::endl;
        return false;
    }

    shaderData.fragmentShader = createShaderModule(fragmentCode);
    if (shaderData.fragmentShader == VK_NULL_HANDLE) {
        std::cerr << "Failed to create fragment shader module for: " << name << std::endl;
        vkDestroyShaderModule(m_device, shaderData.vertexShader, nullptr);
        return false;
    }

    // Generate shader reflection
    if (!generateShaderReflection(name, shaderData)) {
        std::cout << "Warning: Failed to generate shader reflection for: " << name << std::endl;
    }

    // Get file modification times for hot-reload
    if (m_hotReloadEnabled) {
        try {
            shaderData.lastModifiedTime = std::filesystem::last_write_time(vertexPath).time_since_epoch().count();
        } catch (...) {
            shaderData.lastModifiedTime = 0;
        }
    }

    shaderData.loaded = true;
    m_shaders[name] = shaderData;

    std::cout << "Shader '" << name << "' loaded successfully" << std::endl;
    return true;
}

bool ShaderSystem::loadShaderFromSPIRV(const std::string& name, const std::vector<uint32_t>& vertexCode, const std::vector<uint32_t>& fragmentCode) {
    if (!m_initialized) {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_shaderMutex);

    // Check if shader already exists
    if (m_shaders.find(name) != m_shaders.end()) {
        std::cout << "Shader '" << name << "' already exists, reloading..." << std::endl;
        unloadShader(name);
    }

    ShaderData shaderData;
    shaderData.vertexCode.assign(vertexCode.begin(), vertexCode.end());
    shaderData.fragmentCode.assign(fragmentCode.begin(), fragmentCode.end());

    // Create shader modules from SPIR-V
    // Create shader modules from SPIR-V data
    std::vector<char> vertexShaderData(vertexCode.size() * sizeof(uint32_t));
    memcpy(vertexShaderData.data(), vertexCode.data(), vertexShaderData.size());
    
    std::vector<char> fragmentShaderData(fragmentCode.size() * sizeof(uint32_t));
    memcpy(fragmentShaderData.data(), fragmentCode.data(), fragmentShaderData.size());
    
    shaderData.vertexShader = createShaderModule(vertexShaderData);
    if (shaderData.vertexShader == VK_NULL_HANDLE) {
        std::cerr << "Failed to create vertex shader module for: " << name << std::endl;
        return false;
    }

    shaderData.fragmentShader = createShaderModule(fragmentShaderData);
    if (shaderData.fragmentShader == VK_NULL_HANDLE) {
        std::cerr << "Failed to create fragment shader module for: " << name << std::endl;
        vkDestroyShaderModule(m_device, shaderData.vertexShader, nullptr);
        return false;
    }

    // Generate shader reflection
    if (!generateShaderReflection(name, shaderData)) {
        std::cout << "Warning: Failed to generate shader reflection for: " << name << std::endl;
    }

    shaderData.loaded = true;
    m_shaders[name] = shaderData;

    std::cout << "Shader '" << name << "' loaded from SPIR-V successfully" << std::endl;
    return true;
}

void ShaderSystem::unloadShader(const std::string& name) {
    if (!m_initialized) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_shaderMutex);

    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        ShaderData& shaderData = it->second;

        if (shaderData.vertexShader != VK_NULL_HANDLE) {
            vkDestroyShaderModule(m_device, shaderData.vertexShader, nullptr);
            shaderData.vertexShader = VK_NULL_HANDLE;
        }

        if (shaderData.fragmentShader != VK_NULL_HANDLE) {
            vkDestroyShaderModule(m_device, shaderData.fragmentShader, nullptr);
            shaderData.fragmentShader = VK_NULL_HANDLE;
        }

        m_shaders.erase(it);
        std::cout << "Shader '" << name << "' unloaded successfully" << std::endl;
    }
}

VkShaderModule ShaderSystem::getVertexShader(const std::string& name) const {
    std::lock_guard lock(m_shaderMutex);

    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        return it->second.vertexShader;
    }

    return VK_NULL_HANDLE;
}

VkShaderModule ShaderSystem::getFragmentShader(const std::string& name) const {
    std::lock_guard lock(m_shaderMutex);

    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        return it->second.fragmentShader;
    }

    return VK_NULL_HANDLE;
}

void ShaderSystem::enableHotReload(bool enable) {
    m_hotReloadEnabled = enable;
    if (enable && !m_fileWatcher) {
        startFileWatcher();
    } else if (!enable && m_fileWatcher) {
        stopFileWatcher();
    }
}

void ShaderSystem::checkForShaderUpdates() {
    if (!m_initialized || !m_hotReloadEnabled) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_shaderMutex);

    for (auto& [name, shaderData] : m_shaders) {
        bool needsReload = false;

        // Check vertex shader
        if (!shaderData.vertexPath.empty()) {
            try {
                auto newTime = std::filesystem::last_write_time(shaderData.vertexPath).time_since_epoch().count();
                if (newTime > shaderData.lastModifiedTime) {
                    needsReload = true;
                    shaderData.lastModifiedTime = newTime;
                }
            } catch (...) {
                // File might not exist or other error
            }
        }

        // Check fragment shader
        if (!needsReload && !shaderData.fragmentPath.empty()) {
            try {
                auto newTime = std::filesystem::last_write_time(shaderData.fragmentPath).time_since_epoch().count();
                if (newTime > shaderData.lastModifiedTime) {
                    needsReload = true;
                    shaderData.lastModifiedTime = newTime;
                }
            } catch (...) {
                // File might not exist or other error
            }
        }

        if (needsReload) {
            shaderData.needsReload = true;
            std::cout << "Shader '" << name << "' needs to be reloaded" << std::endl;
        }
    }
}

void ShaderSystem::setShaderWatchDirectory(const std::string& directory) {
    m_shaderWatchDirectory = directory;
    if (m_hotReloadEnabled && !directory.empty()) {
        startFileWatcher();
    }
}

bool ShaderSystem::compileShader(const std::string& sourcePath, const std::string& outputPath, const std::vector<std::string>& defines) {
    if (!m_initialized) {
        return false;
    }

    std::vector<char> output;
    if (!compileShaderInternal(sourcePath, defines, output)) {
        return false;
    }

    // Write output to file
    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile) {
        std::cerr << "Failed to open output file: " << outputPath << std::endl;
        return false;
    }

    outFile.write(output.data(), output.size());
    return true;
}

bool ShaderSystem::compileGLSLToSPIRV(const std::string& glslPath, const std::string& spirvPath, VkShaderStageFlagBits stage) {
    if (!m_initialized) {
        return false;
    }

    std::vector<char> spirvCode;
    if (!compileShaderInternal(glslPath, {}, spirvCode)) {
        return false;
    }

    // Write SPIR-V to file
    std::ofstream outFile(spirvPath, std::ios::binary);
    if (!outFile) {
        std::cerr << "Failed to open SPIR-V output file: " << spirvPath << std::endl;
        return false;
    }

    // Convert char vector to uint32_t vector for SPIR-V
    std::vector<uint32_t> spirvData;
    spirvData.reserve(spirvCode.size() / sizeof(uint32_t));
    memcpy(spirvData.data(), spirvCode.data(), spirvCode.size());

    outFile.write(reinterpret_cast<const char*>(spirvData.data()), spirvData.size() * sizeof(uint32_t));
    return true;
}

bool ShaderSystem::getShaderReflection(const std::string& name, ShaderReflection& reflection) {
    std::lock_guard<std::mutex> lock(m_shaderMutex);

    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        reflection = it->second.reflection;
        return true;
    }

    return false;
}

std::vector<VkDescriptorSetLayoutBinding> ShaderSystem::getShaderBindings(const std::string& name) const {
    std::lock_guard lock(m_shaderMutex);

    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        return it->second.reflection.bindings;
    }

    return {};
}

void ShaderSystem::enableShaderCache(bool enable) {
    m_shaderCacheEnabled = enable;
}

void ShaderSystem::saveShaderCache(const std::string& filePath) {
    if (!m_shaderCacheEnabled || m_shaderCache.empty()) {
        return;
    }

    // Implementation for saving shader cache to file
    // This would serialize the shader cache to disk
}

void ShaderSystem::loadShaderCache(const std::string& filePath) {
    if (!m_shaderCacheEnabled) {
        return;
    }

    // Implementation for loading shader cache from file
    // This would deserialize the shader cache from disk
}

void ShaderSystem::printShaderInfo() const {
    std::cout << "Shader System Info:" << std::endl;
    std::cout << "  Shader Count: " << m_shaders.size() << std::endl;
    std::cout << "  Hot Reload: " << (m_hotReloadEnabled ? "Enabled" : "Disabled") << std::endl;
    std::cout << "  Shader Cache: " << (m_shaderCacheEnabled ? "Enabled" : "Disabled") << std::endl;
    std::cout << "  Watch Directory: " << (m_shaderWatchDirectory.empty() ? "None" : m_shaderWatchDirectory) << std::endl;

    for (const auto& [name, shaderData] : m_shaders) {
        std::cout << "  - " << name << ": " 
                  << (shaderData.loaded ? "Loaded" : "Not Loaded")
                  << " (V: " << (!shaderData.vertexPath.empty() ? shaderData.vertexPath : "SPIR-V")
                  << ", F: " << (!shaderData.fragmentPath.empty() ? shaderData.fragmentPath : "SPIR-V") << ")" << std::endl;
    }
}

// Private implementation methods
bool ShaderSystem::loadShaderFile(const std::string& path, std::vector<char>& code) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << path << std::endl;
        return false;
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    code.resize(fileSize);
    if (!file.read(code.data(), fileSize)) {
        std::cerr << "Failed to read shader file: " << path << std::endl;
        return false;
    }

    return true;
}

bool ShaderSystem::loadSPIRVFile(const std::string& path, std::vector<uint32_t>& code) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open SPIR-V file: " << path << std::endl;
        return false;
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    code.resize(fileSize / sizeof(uint32_t));
    if (!file.read(reinterpret_cast<char*>(code.data()), fileSize)) {
        std::cerr << "Failed to read SPIR-V file: " << path << std::endl;
        return false;
    }

    return true;
}

bool ShaderSystem::compileShaderInternal(const std::string& sourcePath, const std::vector<std::string>& defines, std::vector<char>& output) {
    // This is a placeholder for actual shader compilation
    // In a real implementation, this would call glslc or another compiler
    
    std::string compilerPath = getShaderCompilerPath();
    if (compilerPath.empty()) {
        std::cerr << "Shader compiler not found" << std::endl;
        return false;
    }

    // For now, just read the source file as output
    return loadShaderFile(sourcePath, output);
}

bool ShaderSystem::generateShaderReflection(const std::string& name, ShaderData& shaderData) {
    // This is a placeholder for actual shader reflection
    // In a real implementation, this would parse SPIR-V to extract descriptor bindings,
    // push constants, etc.

    // For now, create some default bindings
    VkDescriptorSetLayoutBinding binding{};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    binding.pImmutableSamplers = nullptr;

    shaderData.reflection.bindings.push_back(binding);

    return true;
}

void ShaderSystem::cleanupShaders() {
    std::lock_guard<std::mutex> lock(m_shaderMutex);

    for (auto& [name, shaderData] : m_shaders) {
        if (shaderData.vertexShader != VK_NULL_HANDLE) {
            vkDestroyShaderModule(m_device, shaderData.vertexShader, nullptr);
            shaderData.vertexShader = VK_NULL_HANDLE;
        }

        if (shaderData.fragmentShader != VK_NULL_HANDLE) {
            vkDestroyShaderModule(m_device, shaderData.fragmentShader, nullptr);
            shaderData.fragmentShader = VK_NULL_HANDLE;
        }
    }

    m_shaders.clear();
}

void ShaderSystem::updateShaderWatchTimes() {
    // Implementation for updating file modification times
    // This would be called periodically by the file watcher
}

void ShaderSystem::startFileWatcher() {
    if (m_fileWatcher) {
        return;
    }

    // Start file watcher thread
    // This is a simplified implementation
    m_fileWatcher = std::make_unique<FileWatcher>();
    // In a real implementation, this would use a proper file watching library
}

void ShaderSystem::stopFileWatcher() {
    if (m_fileWatcher) {
        m_fileWatcher.reset();
    }
}

std::string ShaderSystem::getShaderCompilerPath() const {
    // Return path to shader compiler (glslc)
    // This could be configurable
    return "glslc";
}

std::vector<std::string> ShaderSystem::getDefaultShaderDefines() const {
    return {
        "VORTEX_ENGINE",
        "VK_USE_PLATFORM_XLIB_KHR"
    };
}

// FileWatcher implementation (simplified)
struct ShaderSystem::FileWatcher {
    FileWatcher() {
        // Initialize file watcher
    }

    ~FileWatcher() {
        // Cleanup file watcher
    }
};

} // namespace VortexEngine