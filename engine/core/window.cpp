#include "window.h"
#include <iostream>

namespace VortexEngine {

Window::Window() {
    std::cout << "Initializing window system..." << std::endl;
}

Window::~Window() {
    shutdown();
}

bool Window::initialize(const std::string& title, int width, int height) {
    if (m_initialized) {
        std::cout << "Window is already initialized" << std::endl;
        return true;
    }

    try {
        // Initialize GLFW
        if (!initializeGLFW()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }

        // Set window hints
        setHints({
            {GLFW_CLIENT_API, GLFW_NO_API},
            {GLFW_RESIZABLE, GLFW_TRUE},
            {GLFW_VISIBLE, GLFW_TRUE},
            {GLFW_DECORATED, GLFW_TRUE},
            {GLFW_FOCUSED, GLFW_TRUE},
            {GLFW_AUTO_ICONIFY, GLFW_TRUE},
            {GLFW_FLOATING, GLFW_FALSE},
            {GLFW_MAXIMIZED, GLFW_FALSE},
            {GLFW_CENTER_CURSOR, GLFW_FALSE},
            {GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE},
            {GLFW_FOCUS_ON_SHOW, GLFW_TRUE},
            {GLFW_SCALE_TO_MONITOR, GLFW_FALSE}
        });

        // Create window
        m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (!m_window) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            return false;
        }

        // Set window properties
        m_title = title;
        m_width = width;
        m_height = height;

        // Set up callbacks
        glfwSetWindowUserPointer(m_window, this);
        glfwSetKeyCallback(m_window, keyCallbackStatic);
        glfwSetMouseButtonCallback(m_window, mouseButtonCallbackStatic);
        glfwSetCursorPosCallback(m_window, cursorPosCallbackStatic);
        glfwSetScrollCallback(m_window, scrollCallbackStatic);
        glfwSetFramebufferSizeCallback(m_window, resizeCallbackStatic);

        m_initialized = true;
        std::cout << "Window initialized successfully: " << title << " (" << width << "x" << height << ")" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception during window initialization: " << e.what() << std::endl;
        return false;
    }
}

void Window::shutdown() {
    if (!m_initialized) {
        return;
    }

    std::cout << "Shutting down window..." << std::endl;

    // Destroy window
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }

    // Shutdown GLFW
    shutdownGLFW();

    m_initialized = false;
    std::cout << "Window shutdown complete" << std::endl;
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::swapBuffers() {
    if (m_vsyncEnabled) {
        glfwSwapBuffers(m_window);
    }
}

void Window::setWindowTitle(const std::string& title) {
    m_title = title;
    if (m_window) {
        glfwSetWindowTitle(m_window, title.c_str());
    }
}

void Window::setWindowSize(int width, int height) {
    m_width = width;
    m_height = height;
    if (m_window) {
        glfwSetWindowSize(m_window, width, height);
    }
}

void Window::setWindowPos(int x, int y) {
    if (m_window) {
        glfwSetWindowPos(m_window, x, y);
    }
}

void Window::setFullscreen(bool fullscreen) {
    if (!m_window) return;

    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if (fullscreen) {
        glfwSetWindowMonitor(m_window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
    } else {
        glfwSetWindowMonitor(m_window, nullptr, 100, 100, m_width, m_height, mode->refreshRate);
    }
}

void Window::setVSync(bool enabled) {
    m_vsyncEnabled = enabled;
    glfwSwapInterval(enabled ? 1 : 0);
}

bool Window::isKeyPressed(int key) const {
    return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool Window::isMouseButtonPressed(int button) const {
    return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
}

void Window::getCursorPos(double& xpos, double& ypos) const {
    glfwGetCursorPos(m_window, &xpos, &ypos);
}

void Window::setCursorPos(double xpos, double ypos) {
    glfwSetCursorPos(m_window, xpos, ypos);
}

void Window::setCursorMode(int mode) {
    glfwSetInputMode(m_window, GLFW_CURSOR, mode);
}

void Window::setKeyCallback(KeyCallback callback) {
    m_keyCallback = callback;
}

void Window::setMouseButtonCallback(MouseButtonCallback callback) {
    m_mouseButtonCallback = callback;
}

void Window::setCursorPosCallback(CursorPosCallback callback) {
    m_cursorPosCallback = callback;
}

void Window::setScrollCallback(ScrollCallback callback) {
    m_scrollCallback = callback;
}

void Window::setResizeCallback(ResizeCallback callback) {
    m_resizeCallback = callback;
}

void Window::setHint(int hint, int value) {
    glfwWindowHint(hint, value);
}

void Window::setHints(const std::vector<std::pair<int, int>>& hints) {
    for (const auto& hint : hints) {
        glfwWindowHint(hint.first, hint.second);
    }
}

bool Window::initializeGLFW() {
    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // Set error callback
    glfwSetErrorCallback(errorCallbackStatic);

    std::cout << "GLFW initialized successfully" << std::endl;
    return true;
}

void Window::shutdownGLFW() {
    glfwTerminate();
    std::cout << "GLFW shutdown complete" << std::endl;
}

void Window::errorCallbackStatic(int error, const char* description) {
    std::cerr << "GLFW Error [" << error << "]: " << description << std::endl;
}

void Window::keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (instance && instance->m_keyCallback) {
        instance->m_keyCallback(key, scancode, action, mods);
    }
}

void Window::mouseButtonCallbackStatic(GLFWwindow* window, int button, int action, int mods) {
    Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (instance && instance->m_mouseButtonCallback) {
        instance->m_mouseButtonCallback(button, action, mods);
    }
}

void Window::cursorPosCallbackStatic(GLFWwindow* window, double xpos, double ypos) {
    Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (instance && instance->m_cursorPosCallback) {
        instance->m_cursorPosCallback(xpos, ypos);
    }
}

void Window::scrollCallbackStatic(GLFWwindow* window, double xoffset, double yoffset) {
    Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (instance && instance->m_scrollCallback) {
        instance->m_scrollCallback(xoffset, yoffset);
    }
}

void Window::resizeCallbackStatic(GLFWwindow* window, int width, int height) {
    Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (instance && instance->m_resizeCallback) {
        instance->m_resizeCallback(width, height);
    }
}

} // namespace VortexEngine