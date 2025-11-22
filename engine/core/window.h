#pragma once

#include <GLFW/glfw3.h>
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
    GLFWwindow* getGLFWWindow() const { return m_window; }
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
    void setCursorMode(int mode); // GLFW_CURSOR_NORMAL, GLFW_CURSOR_HIDDEN, GLFW_CURSOR_DISABLED

    // Window hints
    static void setHint(int hint, int value);
    static void setHints(const std::vector<std::pair<int, int>>& hints);

private:
    // GLFW window
    GLFWwindow* m_window = nullptr;
    
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
    static void keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallbackStatic(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallbackStatic(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallbackStatic(GLFWwindow* window, double xoffset, double yoffset);
    static void resizeCallbackStatic(GLFWwindow* window, int width, int height);

    // Initialize GLFW
    bool initializeGLFW();
    void shutdownGLFW();

    // Error callback
    static void errorCallbackStatic(int error, const char* description);
};

} // namespace VortexEngine