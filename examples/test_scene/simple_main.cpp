#include <iostream>
#include <chrono>

int main() {
    std::cout << "Starting Vortex Engine Test Scene" << std::endl;
    
    // Create engine instance
    // For now, just a simple placeholder
    
    std::cout << "Engine initialized successfully" << std::endl;

    // Create test scene
    std::cout << "Creating test scene" << std::endl;

    // Create camera
    std::cout << "Creating camera" << std::endl;

    // Create test objects
    for (int i = 0; i < 5; i++) {
        std::string name = "TestObject_" + std::to_string(i);
        std::cout << "Creating test object: " << name << std::endl;
    }

    // Create ground plane
    std::cout << "Creating ground plane" << std::endl;

    // Create lighting
    std::cout << "Creating lighting" << std::endl;

    // Run engine
    std::cout << "Starting engine main loop" << std::endl;
    
    // Simple loop for now
    auto startTime = std::chrono::high_resolution_clock::now();
    while (true) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - startTime).count();
        
        if (deltaTime > 5.0f) break; // Run for 5 seconds
        
        // Update logic would go here
    }

    // Shutdown engine
    std::cout << "Shutting down engine" << std::endl;
    
    std::cout << "Engine shutdown complete" << std::endl;
    return 0;
}