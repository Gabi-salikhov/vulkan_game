#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>

namespace VortexEngine {

class ShaderSystem {
public:
    ShaderSystem();
    ~ShaderSystem();

    // Shader system lifecycle
    bool initialize(VkDevice device);
    void shutdown();

    // Shader module management
    VkShaderModule createShaderModule(const std::vector<char>& code);
    void destroyShaderModule(VkShaderModule shaderModule);

    // Shader loading
    bool loadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);
    bool loadShaderFromSPIRV(const std::string& name, const std::vector<uint32_t>& vertexCode, const std::vector<uint32_t>& fragmentCode);
    void unloadShader(const std::string& name);
    VkShaderModule getVertexShader(const std::string& name) const;
    VkShaderModule getFragmentShader(const std::string& name) const;

    // Shader hot-reload
    void enableHotReload(bool enable);
    bool isHotReloadEnabled() const { return m_hotReloadEnabled; }
    void checkForShaderUpdates();
    void setShaderWatchDirectory(const std::string& directory);

    // Shader compilation (external tool integration)
    bool compileShader(const std::string& sourcePath, const std::string& outputPath, const std::vector<std::string>& defines = {});
    bool compileGLSLToSPIRV(const std::string& glslPath, const std::string& spirvPath, VkShaderStageFlagBits stage);

    // Shader reflection
    struct ShaderReflection {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        std::vector<VkPushConstantRange> pushConstants;
        std::vector<VkSpecializationMapEntry> specializationMapEntries;
    };

    bool getShaderReflection(const std::string& name, ShaderReflection& reflection);
    std::vector<VkDescriptorSetLayoutBinding> getShaderBindings(const std::string& name) const;

    // Shader caching
    void enableShaderCache(bool enable);
    bool isShaderCacheEnabled() const { return m_shaderCacheEnabled; }
    void saveShaderCache(const std::string& filePath);
    void loadShaderCache(const std::string& filePath);

    // Debug information
    void printShaderInfo() const;
    uint32_t getShaderCount() const { return m_shaders.size(); }

private:
    // Vulkan objects
    VkDevice m_device = VK_NULL_HANDLE;

    // Shader storage
    struct ShaderData {
        VkShaderModule vertexShader = VK_NULL_HANDLE;
        VkShaderModule fragmentShader = VK_NULL_HANDLE;
        std::string vertexPath;
        std::string fragmentPath;
        std::vector<uint32_t> vertexCode;
        std::vector<uint32_t> fragmentCode;
        ShaderReflection reflection;
        bool loaded = false;
        bool needsReload = false;
        uint64_t lastModifiedTime = 0;
    };

    std::unordered_map<std::string, ShaderData> m_shaders;
    std::mutex m_shaderMutex;

    // Configuration
    bool m_initialized = false;
    bool m_hotReloadEnabled = false;
    bool m_shaderCacheEnabled = false;
    std::string m_shaderWatchDirectory;

    // Shader cache
    std::unordered_map<std::string, std::vector<uint32_t>> m_shaderCache;

    // File watching
    struct FileWatcher;
    std::unique_ptr<FileWatcher> m_fileWatcher;

    // Internal methods
    bool loadShaderFile(const std::string& path, std::vector<char>& code);
    bool loadSPIRVFile(const std::string& path, std::vector<uint32_t>& code);
    bool compileShaderInternal(const std::string& sourcePath, const std::vector<std::string>& defines, std::vector<char>& output);
    bool generateShaderReflection(const std::string& name, ShaderData& shaderData);
    void cleanupShaders();
    void updateShaderWatchTimes();
    void startFileWatcher();
    void stopFileWatcher();

    // Shader compilation helpers
    std::string getShaderCompilerPath() const;
    std::vector<std::string> getDefaultShaderDefines() const;
};

// Shader creation helper
struct ShaderCreateInfo {
    std::string name;
    std::string vertexPath;
    std::string fragmentPath;
    std::vector<std::string> defines;
    bool enableHotReload = false;
};

// Shader stage info
struct ShaderStageInfo {
    VkShaderStageFlagBits stage;
    VkShaderModule module;
    const char* entryPoint = "main";
    const VkSpecializationInfo* specializationInfo = nullptr;
};

} // namespace VortexEngine