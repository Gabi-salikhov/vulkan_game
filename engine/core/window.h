#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <string>
#include <functional>
#include <memory>

namespace VortexEngine {

class Window {
public:
    using KeyCallback = std::function<void(int key, int scancode, int action, int mods)>;
    using MouseButtonCallback = std::function<void(int button, int action, int mods)>;
    using CursorPosCallback = std::function<void(double xpos, double ypos)>;
    using ScrollCallback = std::function<void(double xoffset, double yoffset)>;
    using ResizeCallback = std::function<void(int width, int height)>;

    Window();
    ~Window();

    // Window lifecycle
    bool initialize(const std::string& title, int width, int height);
    void shutdown();
    bool shouldClose() const;
    void pollEvents();
    void swapBuffers();

    // Window state
    void setWindowTitle(const std::string& title);
    void setWindowSize(int width, int height);
    void setWindowPos(int x, int y);
    void setFullscreen(bool fullscreen);
    void setVSync(bool enabled);

    // Window properties
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    SDL_Window* getSDLWindow() const { return m_window; }
    bool isInitialized() const { return m_initialized; }
    bool isVSyncEnabled() const { return m_vsyncEnabled; }

    // Callback registration
    void setKeyCallback(KeyCallback callback);
    void setMouseButtonCallback(MouseButtonCallback callback);
    void setCursorPosCallback(CursorPosCallback callback);
    void setScrollCallback(ScrollCallback callback);
    void setResizeCallback(ResizeCallback callback);

    // Input state
    bool isKeyPressed(int key) const;
    bool isMouseButtonPressed(int button) const;
    void getCursorPos(double& xpos, double& ypos) const;
    void setCursorPos(double xpos, double ypos);

    // Cursor modes
    void setCursorMode(int mode); // SDL_CURSOR_NORMAL, SDL_CURSOR_HIDDEN, SDL_CURSOR_DISABLED

    // Window hints
    static void setHint(int hint, int value);
    static void setHints(const std::vector<std::pair<int, int>>& hints);
    
    // Vulkan surface creation
    bool createVulkanSurface(VkInstance instance, VkSurfaceKHR* surface) const;

private:
    // GLFW window
    SDL_Window* m_window = nullptr;
    
    // Window state
    bool m_initialized = false;
    bool m_vsyncEnabled = true;
    int m_width = 1280;
    int m_height = 720;
    std::string m_title = "Vortex Engine";

    // Callbacks
    KeyCallback m_keyCallback;
    MouseButtonCallback m_mouseButtonCallback;
    CursorPosCallback m_cursorPosCallback;
    ScrollCallback m_scrollCallback;
    ResizeCallback m_resizeCallback;

    // Static callback functions
    static void keyCallbackStatic(SDL_Event* event);
    static void mouseButtonCallbackStatic(SDL_Event* event);
    static void cursorPosCallbackStatic(SDL_Event* event);
    static void scrollCallbackStatic(SDL_Event* event);
    static void resizeCallbackStatic(SDL_Event* event);

    // Initialize GLFW
    bool initializeGLFW();
    void shutdownGLFW();

    // Error callback
    static void errorCallbackStatic(int error, const char* description);
};

} // namespace VortexEngine