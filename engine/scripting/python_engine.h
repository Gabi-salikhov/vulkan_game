#pragma once

#include <Python.h>

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../ecs/ecs_manager.h"

namespace VortexEngine {

// Forward declarations
// Entity is already defined in ecs_manager.h
class SceneManager;

// Python engine
class PythonEngine {
   public:
    PythonEngine();
    ~PythonEngine();

    // Python engine lifecycle
    bool initialize();
    void shutdown();

    // Script execution
    bool executeScript(const std::string& scriptPath);
    bool executeScriptString(const std::string& script);
    bool executeFunction(const std::string& moduleName, const std::string& functionName);

    // Module management
    bool loadModule(const std::string& moduleName);
    bool unloadModule(const std::string& moduleName);
    bool isModuleLoaded(const std::string& moduleName) const;
    std::vector<std::string> getLoadedModules() const;

    // Entity scripting
    bool createEntityScript(const std::string& scriptPath, Entity entity);
    bool attachScriptToEntity(const std::string& scriptPath, Entity entity);
    bool detachScriptFromEntity(Entity entity);
    bool hasScript(Entity entity) const;
    std::string getEntityScript(Entity entity) const;

    // Script callbacks
    using ScriptUpdateCallback = std::function<void(Entity, float)>;
    using ScriptStartCallback = std::function<void(Entity)>;
    using ScriptStopCallback = std::function<void(Entity)>;

    void registerUpdateCallback(ScriptUpdateCallback callback);
    void registerStartCallback(ScriptStartCallback callback);
    void registerStopCallback(ScriptStopCallback callback);

    // Script variables
    bool setVariable(const std::string& name, const std::string& value);
    bool setVariable(const std::string& name, int value);
    bool setVariable(const std::string& name, float value);
    bool setVariable(const std::string& name, double value);
    bool setVariable(const std::string& name, bool value);
    bool setVariable(const std::string& name, PyObject* value);

    bool getVariable(const std::string& name, std::string& value) const;
    bool getVariable(const std::string& name, int& value) const;
    bool getVariable(const std::string& name, float& value) const;
    bool getVariable(const std::string& name, double& value) const;
    bool getVariable(const std::string& name, bool& value) const;
    PyObject* getVariable(const std::string& name) const;

    // Script functions
    bool callFunction(const std::string& functionName);
    bool callFunction(const std::string& functionName, PyObject* args);
    bool callFunction(const std::string& functionName, PyObject* args, PyObject* kwargs);

    // Script error handling
    std::string getLastError() const { return m_lastError; }
    void clearLastError() { m_lastError.clear(); }

    // Script debugging
    void enableDebugMode(bool enable);
    bool isDebugModeEnabled() const { return m_debugMode; }
    void setScriptDirectory(const std::string& directory) { m_scriptDirectory = directory; }
    const std::string& getScriptDirectory() const { return m_scriptDirectory; }

    // Script hot-reload
    void enableHotReload(bool enable);
    bool isHotReloadEnabled() const { return m_hotReload; }
    void checkForScriptUpdates();

    // Python object management
    PyObject* createPyObject(const std::string& type);
    void destroyPyObject(PyObject* obj);
    PyObject* borrowReference(PyObject* obj);
    void releaseReference(PyObject* obj);

    // Type conversion helpers
    template <typename T>
    T* pythonToCpp(PyObject* obj) const;

    template <typename T>
    PyObject* cppToPython(const T& value) const;

    // Script environment
    void setECSManager(ECSManager* ecsManager) { m_ecsManager = ecsManager; }
    void setSceneManager(SceneManager* sceneManager) { m_sceneManager = sceneManager; }
    ECSManager* getECSManager() const { return m_ecsManager; }
    SceneManager* getSceneManager() const { return m_sceneManager; }

    // Debug information
    void printPythonInfo() const;
    bool isInitialized() const { return m_initialized; }
    std::string getPythonVersion() const { return m_pythonVersion; }

   private:
    // Python interpreter
    PyObject* m_mainModule = nullptr;
    PyObject* m_mainDict = nullptr;
    PyObject* m_builtinsModule = nullptr;

    // Script storage
    std::unordered_map<std::string, PyObject*> m_loadedModules;
    std::unordered_map<Entity, std::string> m_entityScripts;
    std::unordered_map<Entity, PyObject*> m_entityScriptObjects;

    // Configuration
    bool m_initialized = false;
    bool m_debugMode = false;
    bool m_hotReload = false;
    std::string m_scriptDirectory = "scripts";
    std::string m_pythonVersion;

    // Error handling
    std::string m_lastError;

    // Subsystem integration
    ECSManager* m_ecsManager = nullptr;
    SceneManager* m_sceneManager = nullptr;

    // Callbacks
    std::vector<ScriptUpdateCallback> m_updateCallbacks;
    std::vector<ScriptStartCallback> m_startCallbacks;
    std::vector<ScriptStopCallback> m_stopCallbacks;

    // Internal methods
    bool initializePython();
    void shutdownPython();
    bool initializePythonPath();
    bool initializeEngineModule();
    void cleanupModules();
    void cleanupEntityScripts();
    void updateEntityScripts(float deltaTime);
    void notifyScriptStart(Entity entity);
    void notifyScriptStop(Entity entity);
    void handleScriptError();
    std::string pythonObjectToString(PyObject* obj) const;
    PyObject* createEngineModule();
    PyObject* createEntityClass();
    PyObject* createTransformClass();
    PyObject* createCameraClass();
    PyObject* createLightClass();
    PyObject* createSceneManagerClass();
    PyObject* createECSManagerClass();

    // Script file watching
    struct ScriptWatcher;
    std::unique_ptr<ScriptWatcher> m_scriptWatcher;

    void startScriptWatcher();
    void stopScriptWatcher();
    void updateScriptWatchTimes();
};

// Python type definitions
struct PythonType {
    const char* name;
    PyTypeObject* type;
    PyObject* (*newFunc)(PyTypeObject*, PyObject*, PyObject*);
    int (*initFunc)(PyObject*, PyObject*, PyObject*);
    void (*deallocFunc)(PyObject*);
    PyMethodDef* methods;
    PyMemberDef* members;
};

// Python exception handling
class PythonException {
   public:
    PythonException();
    ~PythonException();

    bool hasError() const { return m_hasError; }
    std::string getMessage() const { return m_message; }
    void clear();

   private:
    bool m_hasError = false;
    std::string m_message;
};

// Python context manager
class PythonContext {
   public:
    PythonContext(PythonEngine* engine);
    ~PythonContext();

    PythonContext(const PythonContext&) = delete;
    PythonContext& operator=(const PythonContext&) = delete;

    PythonContext(PythonContext&& other) noexcept;
    PythonContext& operator=(PythonContext&& other) noexcept;

    // Context management
    bool isValid() const { return m_engine != nullptr; }
    PythonEngine* getEngine() const { return m_engine; }
    PyObject* getCurrentContext() const { return m_context; }

    // Variable access
    bool setVariable(const std::string& name, PyObject* value);
    PyObject* getVariable(const std::string& name) const;

    // Function calls
    bool callFunction(const std::string& functionName, PyObject* args = nullptr,
                      PyObject* kwargs = nullptr);

   private:
    PythonEngine* m_engine;
    PyObject* m_context;
    bool m_ownsContext;
};

// Python script helper
class PythonScript {
   public:
    PythonScript(PythonEngine* engine, const std::string& scriptPath);
    ~PythonScript();

    // Script lifecycle
    bool load();
    bool unload();
    bool reload();
    bool isLoaded() const { return m_loaded; }

    // Script execution
    bool execute();
    bool executeFunction(const std::string& functionName, PyObject* args = nullptr,
                         PyObject* kwargs = nullptr);

    // Script properties
    const std::string& getPath() const { return m_path; }
    const std::string& getName() const { return m_name; }
    PyObject* getModule() const { return m_module; }

    // Script variables
    bool setVariable(const std::string& name, PyObject* value);
    PyObject* getVariable(const std::string& name) const;

   private:
    PythonEngine* m_engine;
    std::string m_path;
    std::string m_name;
    PyObject* m_module = nullptr;
    bool m_loaded = false;

    // Internal methods
    bool loadModule();
    void unloadModule();
    std::string extractScriptName(const std::string& path) const;
};

}  // namespace VortexEngine