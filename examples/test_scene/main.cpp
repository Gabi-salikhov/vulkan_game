#include "vortex_engine.h"
#include <iostream>
#include <chrono>

using namespace VortexEngine;

class TestScene : public ISystem {
public:
    TestScene() {
        m_signature.set(ComponentRegistry::GetComponentType<Transform>());
        m_signature.set(ComponentRegistry::GetComponentType<MeshRenderer>());
        m_signature.set(ComponentRegistry::GetComponentType<Camera>());
    }

    const Signature& getSignature() const override {
        return m_signature;
    }

    void update(float deltaTime) override {
        // Rotate camera around the scene
        if (m_cameraEntity.isValid()) {
            Transform& transform = m_ecsManager->getComponent<Transform>(m_cameraEntity);
            float angle = m_rotationSpeed * deltaTime;
            
            // Circular motion
            float radius = 5.0f;
            transform.position.x = radius * cos(m_rotationAngle);
            transform.position.z = radius * sin(m_rotationAngle);
            transform.position.y = 2.0f;
            
            m_rotationAngle += angle;
        }
        
        // Rotate test objects
        for (Entity entity : m_testEntities) {
            if (entity.isValid()) {
                Transform& transform = m_ecsManager->getComponent<Transform>(entity);
                transform.rotation.y += m_rotationSpeed * deltaTime;
            }
        }
    }

    void setECSManager(ECSManager* ecsManager) {
        m_ecsManager = ecsManager;
    }

    void setCameraEntity(Entity cameraEntity) {
        m_cameraEntity = cameraEntity;
    }

    void addTestEntity(Entity entity) {
        m_testEntities.push_back(entity);
    }

private:
    Signature m_signature;
    ECSManager* m_ecsManager = nullptr;
    Entity m_cameraEntity;
    std::vector<Entity> m_testEntities;
    
    float m_rotationSpeed = 1.0f;
    float m_rotationAngle = 0.0f;
};

int main() {
    // Initialize logger
    Logger::initializeSingleton();
    VORTEX_INFO("Starting Vortex Engine Test Scene");

    // Create engine instance
    VortexEngine engine;
    
    // Configure engine
    engine.setWindowTitle("Vortex Engine - Test Scene");
    engine.setWindowSize(1280, 720);
    engine.enableValidationLayers(true);
    
    // Initialize engine
    if (!engine.initialize()) {
        VORTEX_ERROR("Failed to initialize engine");
        return -1;
    }
    
    VORTEX_INFO("Engine initialized successfully");

    // Create test scene
    TestScene* testScene = new TestScene();
    engine.getECSManager()->addSystem<TestScene>(testScene);
    testScene->setECSManager(engine.getECSManager());

    // Create camera entity
    Entity cameraEntity = engine.getECSManager()->createEntity("MainCamera");
    engine.getECSManager()->addComponent<Camera>(cameraEntity, 45.0f, 0.1f, 100.0f, true);
    engine.getECSManager()->addComponent<Transform>(cameraEntity, glm::vec3(0.0f, 2.0f, 5.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    testScene->setCameraEntity(cameraEntity);

    // Create test objects
    for (int i = 0; i < 5; i++) {
        std::string name = "TestObject_" + std::to_string(i);
        Entity entity = engine.getECSManager()->createEntity(name);
        
        // Add transform component
        float x = (i - 2) * 2.0f;
        engine.getECSManager()->addComponent<Transform>(entity, glm::vec3(x, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f));
        
        // Add mesh renderer component
        engine.getECSManager()->addComponent<MeshRenderer>(entity, "cube", "default_material");
        
        // Add to test scene
        testScene->addTestEntity(entity);
    }

    // Create ground plane
    Entity groundEntity = engine.getECSManager()->createEntity("Ground");
    engine.getECSManager()->addComponent<Transform>(groundEntity, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f), glm::vec3(10.0f, 1.0f, 10.0f));
    engine.getECSManager()->addComponent<MeshRenderer>(groundEntity, "plane", "ground_material");

    // Create lighting
    Entity lightEntity = engine.getECSManager()->createEntity("Light");
    engine.getECSManager()->addComponent<Transform>(lightEntity, glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f), glm::vec3(1.0f));
    engine.getECSManager()->addComponent<Light>(lightEntity, LightType::Directional, glm::vec3(1.0f), 1.0f);

    // Run engine
    VORTEX_INFO("Starting engine main loop");
    engine.run();

    // Shutdown engine
    VORTEX_INFO("Shutting down engine");
    engine.shutdown();
    
    // Cleanup
    delete testScene;
    Logger::shutdownSingleton();
    
    VORTEX_INFO("Engine shutdown complete");
    return 0;
}