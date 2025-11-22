#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <functional>
#include <typeindex>
#include <bitset>
#include <queue>

namespace VortexEngine {

// Entity ID type
using Entity = uint32_t;
using EntityId = uint32_t;
constexpr Entity INVALID_ENTITY = 0;

// Component types
using ComponentType = uint8_t;
constexpr ComponentType MAX_COMPONENTS = 64;

// Signature (bitset of components)
using Signature = std::bitset<MAX_COMPONENTS>;

// Base component interface
class IComponent {
public:
    virtual ~IComponent() = default;
};

// Component registry
class ComponentRegistry {
public:
    template<typename T>
    static ComponentType GetComponentType() {
        static ComponentType type = s_nextComponentType++;
        return type;
    }

    static ComponentType GetNextComponentType() { return s_nextComponentType; }

private:
    static ComponentType s_nextComponentType;
};

// Entity wrapper class
class Entity {
public:
    Entity(uint32_t id = INVALID_ENTITY, ECSManager* manager = nullptr)
        : m_id(id), m_manager(manager) {}
    
    Entity(const Entity& other) = default;
    Entity& operator=(const Entity& other) = default;
    Entity(Entity&& other) noexcept = default;
    Entity& operator=(Entity&& other) noexcept = default;
    
    ~Entity() = default;

    // Entity operations
    bool isValid() const { return m_id != INVALID_ENTITY && m_manager != nullptr; }
    void destroy();

    // Component operations
    template<typename T>
    bool hasComponent() const;

    template<typename T, typename... Args>
    T& addComponent(Args&&... args);

    template<typename T>
    T& getComponent() const;

    template<typename T>
    void removeComponent();

    // Entity relationships
    Entity createChild();
    Entity getParent() const;
    void setParent(Entity parent);
    const std::vector<Entity>& getChildren() const;

    // Utility
    uint32_t getId() const { return m_id; }
    operator uint32_t() const { return m_id; }

private:
    uint32_t m_id;
    ECSManager* m_manager;
};

// System base class
class ISystem {
public:
    virtual ~ISystem() = default;
    virtual void update(float deltaTime) = 0;
    virtual void onEntityAdded(Entity entity) {}
    virtual void onEntityRemoved(Entity entity) {}
    virtual void onComponentAdded(Entity entity, ComponentType componentType) {}
    virtual void onComponentRemoved(Entity entity, ComponentType componentType) {}
    virtual const Signature& getSignature() const = 0;
};

// System registration
template<typename T>
class System : public ISystem {
public:
    static_assert(std::is_base_of<ISystem, T>::value, "T must inherit from ISystem");

    System() {
        m_signature = T::GetSignature();
    }

    const Signature& getSignature() const override {
        return m_signature;
    }

private:
    Signature m_signature;
};

// ECS Manager
class ECSManager {
public:
    ECSManager();
    ~ECSManager();

    // ECS lifecycle
    void initialize();
    void shutdown();

    // Entity management
    Entity createEntity();
    void destroyEntity(Entity entity);
    bool isEntityValid(Entity entity) const;
    EntityId getEntityId(Entity entity) const;

    // Component management
    template<typename T>
    void registerComponent();

    template<typename T>
    bool hasComponent(Entity entity) const;

    template<typename T, typename... Args>
    T& addComponent(Entity entity, Args&&... args);

    template<typename T>
    T& getComponent(Entity entity) const;

    template<typename T>
    void removeComponent(Entity entity);

    // System management
    template<typename T>
    T& addSystem();

    template<typename T>
    void removeSystem();

    template<typename T>
    bool hasSystem() const;

    // System execution
    void updateSystems(float deltaTime);
    void registerSystemUpdateOrder(const std::vector<std::type_index>& order);

    // Entity queries
    std::vector<Entity> getEntitiesWithSignature(const Signature& signature) const;
    std::vector<Entity> getEntitiesWithComponent(ComponentType componentType) const;
    std::vector<Entity> getEntitiesWithComponents(const std::vector<ComponentType>& componentTypes) const;

    // Event system
    using EntityEvent = std::function<void(Entity)>;
    using ComponentEvent = std::function<void(Entity, ComponentType)>;

    void onEntityAdded(EntityEvent callback);
    void onEntityRemoved(EntityEvent callback);
    void onComponentAdded(ComponentEvent callback);
    void onComponentRemoved(ComponentEvent callback);

    // Debug information
    void printECSInfo() const;
    size_t getEntityCount() const { return m_entities.size(); }
    size_t getComponentCount() const { return m_componentPools.size(); }
    size_t getSystemCount() const { return m_systems.size(); }

private:
    // Entity management
    std::queue<Entity> m_availableEntities;
    std::vector<Entity> m_entities;
    std::vector<Signature> m_entitySignatures;
    std::unordered_map<EntityId, Entity> m_entityMap;

    // Component management
    std::unordered_map<ComponentType, std::unique_ptr<class IComponentPool>> m_componentPools;

    // System management
    std::vector<std::unique_ptr<ISystem>> m_systems;
    std::unordered_map<std::type_index, ISystem*> m_systemMap;
    std::vector<std::type_index> m_systemUpdateOrder;

    // Event callbacks
    std::vector<EntityEvent> m_entityAddedCallbacks;
    std::vector<EntityEvent> m_entityRemovedCallbacks;
    std::vector<ComponentEvent> m_componentAddedCallbacks;
    std::vector<ComponentEvent> m_componentRemovedCallbacks;

    // Configuration
    bool m_initialized = false;

    // Internal methods
    void addEntityToSystems(Entity entity);
    void removeEntityFromSystems(Entity entity);
    void updateEntitySignature(Entity entity, const Signature& newSignature);
    void notifyEntityAdded(Entity entity);
    void notifyEntityRemoved(Entity entity);
    void notifyComponentAdded(Entity entity, ComponentType componentType);
    void notifyComponentRemoved(Entity entity, ComponentType componentType);
};

// Component pool interface
class IComponentPool {
public:
    virtual ~IComponentPool() = default;
    virtual void removeEntity(Entity entity) = 0;
    virtual void* getComponent(Entity entity) = 0;
    virtual const void* getComponent(Entity entity) const = 0;
    virtual Signature getSignature() const = 0;
};

// Component pool implementation
template<typename T>
class ComponentPool : public IComponentPool {
public:
    ComponentPool() {
        m_signature.set(ComponentRegistry::GetComponentType<T>());
    }

    void addComponent(Entity entity, T component) {
        if (m_components.find(entity) == m_components.end()) {
            m_components[entity] = component;
        }
    }

    T& getComponent(Entity entity) {
        return m_components.at(entity);
    }

    const T& getComponent(Entity entity) const {
        return m_components.at(entity);
    }

    void removeComponent(Entity entity) {
        m_components.erase(entity);
    }

    void* getComponent(Entity entity) override {
        return &m_components[entity];
    }

    const void* getComponent(Entity entity) const override {
        return &m_components.at(entity);
    }

    Signature getSignature() const override {
        return m_signature;
    }

    size_t size() const {
        return m_components.size();
    }

private:
    std::unordered_map<Entity, T> m_components;
    Signature m_signature;
};

// Entity implementation
inline void Entity::destroy() {
    if (isValid()) {
        m_manager->destroyEntity(*this);
        m_id = INVALID_ENTITY;
        m_manager = nullptr;
    }
}

// ECS Manager template implementations
template<typename T>
void ECSManager::registerComponent() {
    ComponentType type = ComponentRegistry::GetComponentType<T>();
    if (m_componentPools.find(type) == m_componentPools.end()) {
        m_componentPools[type] = std::make_unique<ComponentPool<T>>();
    }
}

template<typename T>
bool ECSManager::hasComponent(Entity entity) const {
    ComponentType type = ComponentRegistry::GetComponentType<T>();
    return m_entitySignatures[static_cast<size_t>(entity)].test(type);
}

template<typename T, typename... Args>
T& ECSManager::addComponent(Entity entity, Args&&... args) {
    ComponentType type = ComponentRegistry::GetComponentType<T>();
    
    // Ensure component is registered
    if (m_componentPools.find(type) == m_componentPools.end()) {
        registerComponent<T>();
    }

    // Add component to pool
    ComponentPool<T>* pool = static_cast<ComponentPool<T>*>(m_componentPools[type].get());
    T component = T(std::forward<Args>(args)...);
    pool->addComponent(entity, component);

    // Update entity signature
    m_entitySignatures[static_cast<size_t>(entity)].set(type);

    // Notify systems
    notifyComponentAdded(entity, type);

    return pool->getComponent(entity);
}

template<typename T>
T& ECSManager::getComponent(Entity entity) const {
    ComponentType type = ComponentRegistry::GetComponentType<T>();
    ComponentPool<T>* pool = static_cast<ComponentPool<T>*>(m_componentPools[type].get());
    return pool->getComponent(entity);
}

template<typename T>
void ECSManager::removeComponent(Entity entity) {
    ComponentType type = ComponentRegistry::GetComponentType<T>();
    
    if (m_componentPools.find(type) != m_componentPools.end()) {
        ComponentPool<T>* pool = static_cast<ComponentPool<T>*>(m_componentPools[type].get());
        pool->removeComponent(entity);

        // Update entity signature
        m_entitySignatures[static_cast<size_t>(entity)].reset(type);

        // Notify systems
        notifyComponentRemoved(entity, type);
    }
}


} // namespace VortexEngine