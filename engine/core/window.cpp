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
        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
            return false;
        }

        // Create window
        m_window = SDL_CreateWindow(
            title.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            width,
            height,
            SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
        );
        
        if (!m_window) {
            std::cerr << "Failed to create SDL window: " << SDL_GetError() << std::endl;
            return false;
        }

        // Set window properties
        m_title = title;
        m_width = width;
        m_height = height;

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
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    // Shutdown SDL
    SDL_Quit();

    m_initialized = false;
    std::cout << "Window shutdown complete" << std::endl;
}

bool Window::shouldClose() const {
    return SDL_GetWindowFlags(m_window) & SDL_WINDOW_MINIMIZED;
}

void Window::pollEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                if (m_keyCallback) {
                    m_keyCallback(event.key.keysym.sym, event.key.keysym.scancode, event.key.state, event.key.keysym.mod);
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                if (m_mouseButtonCallback) {
                    m_mouseButtonCallback(event.button.button, event.button.state, event.button.state);
                }
                break;
            case SDL_MOUSEMOTION:
                if (m_cursorPosCallback) {
                    m_cursorPosCallback(event.motion.x, event.motion.y);
                }
                break;
            case SDL_MOUSEWHEEL:
                if (m_scrollCallback) {
                    m_scrollCallback(event.wheel.x, event.wheel.y);
                }
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    if (m_resizeCallback) {
                        m_resizeCallback(event.window.data1, event.window.data2);
                    }
                }
                break;
        }
    }
}

void Window::swapBuffers() {
    // SDL doesn't need explicit buffer swapping
}

void Window::setWindowTitle(const std::string& title) {
    m_title = title;
    if (m_window) {
        SDL_SetWindowTitle(m_window, title.c_str());
    }
}

void Window::setWindowSize(int width, int height) {
    m_width = width;
    m_height = height;
    if (m_window) {
        SDL_SetWindowSize(m_window, width, height);
    }
}

void Window::setWindowPos(int x, int y) {
    if (m_window) {
        SDL_SetWindowPosition(m_window, x, y);
    }
}

void Window::setFullscreen(bool fullscreen) {
    if (!m_window) return;

    if (fullscreen) {
        SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    } else {
        SDL_SetWindowFullscreen(m_window, 0);
        SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }
}

void Window::setVSync(bool enabled) {
    m_vsyncEnabled = enabled;
    SDL_GL_SetSwapInterval(enabled ? 1 : 0);
}

bool Window::isKeyPressed(int key) const {
    const Uint8* state = SDL_GetKeyboardState(NULL);
    return state[key];
}

bool Window::isMouseButtonPressed(int button) const {
    return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(button);
}

void Window::getCursorPos(double& xpos, double& ypos) const {
    int x, y;
    SDL_GetMouseState(&x, &y);
    xpos = x;
    ypos = y;
}

void Window::setCursorPos(double xpos, double ypos) {
    SDL_WarpMouseInWindow(m_window, xpos, ypos);
}

void Window::setCursorMode(int mode) {
    SDL_SetRelativeMouseMode(mode == SDL_DISABLE ? SDL_TRUE : SDL_FALSE);
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
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
}

void Window::setHints(const std::vector<std::pair<int, int>>& hints) {
    for (const auto& hint : hints) {
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    }
}

void Window::keyCallbackStatic(SDL_Event* event) {
    // Static callback is not needed with SDL event handling
}

void Window::mouseButtonCallbackStatic(SDL_Event* event) {
    // Static callback is not needed with SDL event handling
}

void Window::cursorPosCallbackStatic(SDL_Event* event) {
    // Static callback is not needed with SDL event handling
}

void Window::scrollCallbackStatic(SDL_Event* event) {
    // Static callback is not needed with SDL event handling
}

void Window::resizeCallbackStatic(SDL_Event* event) {
    // Static callback is not needed with SDL event handling
}

bool Window::createVulkanSurface(VkInstance instance, VkSurfaceKHR* surface) const {
    if (!m_window) {
        std::cerr << "Cannot create Vulkan surface: window is not initialized" << std::endl;
        return false;
    }
    
    if (!SDL_Vulkan_CreateSurface(m_window, instance, surface)) {
        std::cerr << "Failed to create Vulkan surface: " << SDL_GetError() << std::endl;
        return false;
    }
    
    std::cout << "Vulkan surface created successfully" << std::endl;
    return true;
}

} // namespace VortexEngine