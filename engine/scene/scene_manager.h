#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

#include "../ecs/ecs_manager.h"

namespace VortexEngine {

// Forward declarations
class Entity;
class Camera;
class Light;
class Mesh;
class Material;

// Scene node
class SceneNode {
public:
    SceneNode(const std::string& name = "SceneNode");
    ~SceneNode();

    // Scene node lifecycle
    void initialize();
    void shutdown();

    // Scene node hierarchy
    SceneNode* createChild(const std::string& name);
    void addChild(SceneNode* child);
    void removeChild(SceneNode* child);
    void removeChild(const std::string& name);
    void clearChildren();
    SceneNode* getParent() const { return m_parent; }
    const std::vector<SceneNode*>& getChildren() const { return m_children; }

    // Entity management
    Entity createEntity();
    Entity createEntity(const std::string& name);
    void destroyEntity(Entity entity);
    bool hasEntity(Entity entity) const;
    Entity getEntity(const std::string& name) const;
    const std::vector<Entity>& getEntities() const { return m_entities; }

    // Transform
    void setPosition(const glm::vec3& position);
    void setRotation(const glm::vec3& rotation);
    void setScale(const glm::vec3& scale);
    void setTransform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale);
    
    const glm::vec3& getPosition() const { return m_position; }
    const glm::vec3& getRotation() const { return m_rotation; }
    const glm::vec3& getScale() const { return m_scale; }
    
    glm::mat4 getTransformMatrix() const;
    glm::mat4 getWorldTransformMatrix() const;

    // Scene node properties
    void setName(const std::string& name) { m_name = name; }
    const std::string& getName() const { return m_name; }
    void setActive(bool active) { m_active = active; }
    bool isActive() const { return m_active; }
    void setTag(const std::string& tag) { m_tag = tag; }
    const std::string& getTag() const { return m_tag; }

    // Scene node updates
    void update(float deltaTime);
    void updateTransform();
    void updateWorldTransform();

    // Scene node queries
    SceneNode* findNode(const std::string& name);
    std::vector<SceneNode*> findNodesWithTag(const std::string& tag);
    bool hasTag(const std::string& tag) const { return m_tag == tag; }

    // Serialization
    void serialize(std::ostream& stream) const;
    void deserialize(std::istream& stream);

private:
    // Scene node properties
    std::string m_name;
    std::string m_tag;
    bool m_active = true;

    // Transform
    glm::vec3 m_position = glm::vec3(0.0f);
    glm::vec3 m_rotation = glm::vec3(0.0f);
    glm::vec3 m_scale = glm::vec3(1.0f);
    glm::mat4 m_transformMatrix = glm::mat4(1.0f);
    glm::mat4 m_worldTransformMatrix = glm::mat4(1.0f);

    // Hierarchy
    SceneNode* m_parent = nullptr;
    std::vector<SceneNode*> m_children;

    // Entities
    std::vector<Entity> m_entities;
    ECSManager* m_ecsManager = nullptr;

    // Internal methods
    void updateWorldTransformRecursive();
    void removeChildInternal(SceneNode* child);
    void cleanupChildren();
};

// Scene manager
class SceneManager {
public:
    SceneManager();
    ~SceneManager();

    // Scene manager lifecycle
    bool initialize(ECSManager* ecsManager);
    void shutdown();

    // Scene management
    bool loadScene(const std::string& scenePath);
    bool saveScene(const std::string& scenePath);
    bool createScene(const std::string& name);
    bool destroyScene(const std::string& name);
    bool setActiveScene(const std::string& name);
    std::string getActiveScene() const { return m_activeScene; }

    // Scene queries
    SceneNode* getRootNode() { return m_rootNode.get(); }
    const SceneNode* getRootNode() const { return m_rootNode.get(); }
    SceneNode* findNode(const std::string& name);
    std::vector<SceneNode*> findNodesWithTag(const std::string& tag);
    std::vector<std::string> getSceneNames() const;

    // Entity management
    Entity createEntity(const std::string& name = "Entity");
    void destroyEntity(Entity entity);
    bool hasEntity(Entity entity) const;
    Entity getEntity(const std::string& name) const;

    // Scene updates
    void update(float deltaTime);
    void render();

    // Scene events
    using SceneEvent = std::function<void(const std::string&)>;
    using EntityEvent = std::function<void(Entity)>;

    void onSceneLoaded(SceneEvent callback);
    void onSceneSaved(SceneEvent callback);
    void onSceneCreated(SceneEvent callback);
    void onSceneDestroyed(SceneEvent callback);
    void onEntityCreated(EntityEvent callback);
    void onEntityDestroyed(EntityEvent callback);

    // Scene configuration
    void setSceneDirectory(const std::string& directory) { m_sceneDirectory = directory; }
    const std::string& getSceneDirectory() const { return m_sceneDirectory; }
    void setAutoSave(bool enabled) { m_autoSave = enabled; }
    bool isAutoSaveEnabled() const { return m_autoSave; }

    // Debug information
    void printSceneInfo() const;
    size_t getSceneCount() const { return m_scenes.size(); }
    size_t getTotalEntityCount() const { return m_totalEntityCount; }

private:
    // Scene storage
    std::unordered_map<std::string, std::unique_ptr<SceneNode>> m_scenes;
    std::unique_ptr<SceneNode> m_rootNode;
    std::string m_activeScene;
    std::string m_sceneDirectory = "scenes";
    bool m_autoSave = false;

    // ECS integration
    ECSManager* m_ecsManager = nullptr;

    // Event callbacks
    std::vector<SceneEvent> m_sceneLoadedCallbacks;
    std::vector<SceneEvent> m_sceneSavedCallbacks;
    std::vector<SceneEvent> m_sceneCreatedCallbacks;
    std::vector<SceneEvent> m_sceneDestroyedCallbacks;
    std::vector<EntityEvent> m_entityCreatedCallbacks;
    std::vector<EntityEvent> m_entityDestroyedCallbacks;

    // Statistics
    size_t m_totalEntityCount = 0;

    // Internal methods
    bool validateSceneName(const std::string& name) const;
    void notifySceneLoaded(const std::string& sceneName);
    void notifySceneSaved(const std::string& sceneName);
    void notifySceneCreated(const std::string& sceneName);
    void notifySceneDestroyed(const std::string& sceneName);
    void notifyEntityCreated(Entity entity);
    void notifyEntityDestroyed(Entity entity);
    void cleanupScenes();
    void serializeSceneNode(SceneNode* node, std::ostream& stream) const;
    void deserializeSceneNode(SceneNode* node, std::istream& stream);
};

// Scene serialization format
struct SceneHeader {
    std::string version;
    std::string name;
    std::string timestamp;
    uint32_t entityCount;
    uint32_t nodeCount;
};

// Scene component helpers
namespace SceneComponents {
    // Transform component
    struct Transform {
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;
    };

    // Mesh renderer component
    struct MeshRenderer {
        std::string meshPath;
        std::string materialPath;
        bool castShadows = true;
        bool receiveShadows = true;
    };

    // Camera component
    struct Camera {
        float fov = 45.0f;
        float near = 0.1f;
        float far = 1000.0f;
        bool isMain = false;
        CameraType type = CameraType::Perspective;
    };

    // Light component
    struct Light {
        LightType type = LightType::Directional;
        glm::vec3 color = glm::vec3(1.0f);
        float intensity = 1.0f;
        float range = 10.0f;
        float spotAngle = 45.0f;
        bool castShadows = true;
    };

    // Tag component
    struct Tag {
        std::string value;
    };
}

} // namespace VortexEngine