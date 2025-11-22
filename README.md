# Vortex Engine - AAA Vulkan-Based Game Engine

A modern, high-performance, cross-platform AAA game engine built on Vulkan as the rendering backend, C++20 as the core engine language, and Python as the scripting layer.

## 🚀 Quick Start

### Prerequisites

Before building and running the Vortex Engine, ensure you have the following dependencies installed:

#### System Requirements
- **Operating System**: Arch Linux (Primary), Ubuntu 20.04+, or Windows 10+
- **CPU**: Modern multi-core processor (4+ cores recommended)
- **RAM**: 8GB minimum, 16GB+ recommended
- **GPU**: Vulkan-compatible GPU (NVIDIA GTX 1060+, AMD RX 580+, Intel Iris Xe+)
- **Storage**: 2GB+ free space for build artifacts

#### Required Dependencies

**Arch Linux:**
```bash
sudo pacman -S base-devel cmake vulkan-devel glfw python python-pip
```

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install build-essential cmake libvulkan-dev libglfw3-dev python3 python3-pip
```

**Windows:**
- Install Visual Studio 2019 or later with C++ development tools
- Install Vulkan SDK from [LunarG](https://vulkan.lunarg.com/)
- Install CMake from [cmake.org](https://cmake.org/)
- Install Python 3.8+ from [python.org](https://python.org/)

### Building the Engine

1. **Clone the repository:**
```bash
git clone https://github.com/yourusername/vortex_engine.git
cd vortex_engine
```

2. **Create build directory:**
```bash
mkdir build
cd build
```

3. **Configure with CMake:**
```bash
cmake ..
```

4. **Build the engine:**
```bash
# Release build (recommended)
cmake --build . --config Release

# Debug build (for development)
cmake --build . --config Debug
```

5. **Build the test scene:**
```bash
cmake --build . --target test_scene
```

### Running the Test Scene

After building, you can run the test scene:

**Linux:**
```bash
./examples/test_scene/test_scene
```

**Windows:**
```bash
.\examples\test_scene\Release\test_scene.exe
```

### Expected Output

When you run the test scene, you should see:
- A window titled "Vortex Engine - Test Scene"
- A rotating camera view around 3D objects
- Basic lighting and rendering
- Console output showing engine initialization and runtime information

## 📁 Project Structure

```
vortex_engine/
├── engine/                    # Core engine modules
│   ├── core/                 # Core engine systems
│   │   ├── vortex_engine.h   # Main engine class
│   │   ├── vulkan_context.h  # Vulkan management
│   │   ├── window.h          # Window system
│   │   └── memory_manager.h  # GPU memory management
│   ├── renderer/             # Rendering subsystem
│   │   ├── shader_system.h   # Shader management
│   │   ├── pipeline_system.h # Graphics pipeline
│   │   └── buffer_allocator.h # Buffer allocation
│   ├── ecs/                  # Entity Component System
│   ├── scene/                # Scene management
│   ├── scripting/            # Python integration
│   └── utils/                # Utility systems
├── shaders/                  # Shader files
│   ├── common/               # Common shaders
│   ├── pbr/                  # PBR shaders
│   └── postfx/               # Post-processing
├── tools/                    # Development tools
├── examples/                 # Example applications
└── assets/                   # Asset directories
```

## 🔧 Configuration

### Environment Variables

Optional environment variables you can set:

```bash
# Vulkan validation layers (for debugging)
export VK_LAYER_KHRONOS_VALIDATION=1

# Python script directory
export VORTEX_SCRIPTS_DIR="scripts"

# Asset directory
export VORTEX_ASSETS_DIR="assets"
```

### CMake Options

You can configure the build with additional CMake options:

```bash
# Enable/disable validation layers
cmake .. -DVORTEX_VALIDATION_LAYERS=ON

# Set build type
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Enable/disable Python integration
cmake .. -DVORTEX_PYTHON=ON

# Set install prefix
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
```

## 🎮 Troubleshooting

### Common Issues

**Vulkan Not Available:**
```bash
# Check Vulkan support
vulkaninfo

# Install Vulkan drivers (Ubuntu)
sudo apt install mesa-vulkan-drivers
```

**GLFW Not Found:**
```bash
# Install GLFW development libraries
sudo apt install libglfw3-dev  # Ubuntu
sudo pacman -S glfw           # Arch
```

**Python Integration Issues:**
```bash
# Install Python dependencies
pip3 install numpy pybind11
```

**Build Errors:**
```bash
# Clean build
rm -rf build/*
mkdir build
cd build
cmake ..
cmake --build .
```

### Debug Mode

For development, use the debug build:

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug
```

This enables:
- Vulkan validation layers
- Debug logging
- Memory leak detection
- Performance profiling

## 📚 Documentation

### Core Concepts

1. **Entity Component System (ECS)**
   - Entities are game objects
   - Components are data containers
   - Systems process components

2. **Vulkan Rendering Pipeline**
   - Command buffer recording
   - Descriptor sets and uniforms
   - Shader modules and pipelines

3. **Python Scripting**
   - Embedded Python interpreter
   - Entity scripting support
   - Hot-reload capabilities

### API Usage

Basic engine usage:
```cpp
#include "vortex_engine.h"

int main() {
    VortexEngine engine;
    engine.initialize();
    
    // Create entities and components
    Entity entity = engine.getECSManager()->createEntity();
    engine.getECSManager()->addComponent<Transform>(entity, position, rotation, scale);
    
    engine.run();
    engine.shutdown();
    return 0;
}
```

## 🛠️ Development

### Adding New Features

1. **New Component Types:**
   - Define component class
   - Register in ECS manager
   - Create systems to process

2. **New Shaders:**
   - Add GLSL files to shaders/
   - Update shader system
   - Create pipeline configurations

3. **New Tools:**
   - Add to tools/ directory
   - Integrate with CMake build system

### Code Style

- Follow C++20 best practices
- Use RAII for resource management
- Implement proper error handling
- Add comprehensive logging

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## 📞 Support

For support and questions:
- Create an issue on GitHub
- Check the documentation
- Review the example code
- Join the community Discord

---

**Vortex Engine** - Building the future of game development with Vulkan, C++20, and Python.# vulkan_game
