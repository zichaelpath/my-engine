#pragma once

#include <string>
#include <vector>

class Game;

// The scripting host (Lesson 66, Arc 5): one embedded Lua state,
// owned and driven through a small C++ face. Scripts are a game's
// own code — written by the game's author, living in the
// project's scripts/ folder — so the host opens Lua's standard
// libraries in full, the way the game's C++ would be trusted.
//
// Errors are values here: a script that fails to compile or run
// logs its message and returns false; it never takes the engine
// down with it. That discipline is what lets Lesson 70 call
// scripts from inside the frame loop safely.
//
// The header forward-declares lua_State so includers do not pull
// in the Lua headers; the binding layer (Lesson 67) includes them
// itself where it needs them.
struct lua_State;

class ScriptHost
{
public:
    ScriptHost();
    ~ScriptHost();
    ScriptHost(const ScriptHost&) = delete;
    ScriptHost& operator=(const ScriptHost&) = delete;

    bool valid() const { return m_state != nullptr; }

    // Runs a chunk of Lua source. chunkName names it in error
    // messages ("string", a file name, ...). Returns false — with
    // the error logged — on any compile or runtime failure.
    bool runString(const std::string& code,
                   const std::string& chunkName = "chunk");

    // Runs a .lua file; a missing file is a false, not a crash.
    bool runFile(const std::string& path);

    // Runs a chunk expected to `return` a number (tests, tools,
    // configuration reads). False when the chunk fails or the
    // value is not numeric.
    bool evalNumber(const std::string& code, double& out);

    lua_State* state() { return m_state; }

    // --- The bridge (Lesson 67) --------------------------------------
    // A script MODULE is a .lua file that returns a table of
    // functions (the Lua module idiom):
    //     local M = {}
    //     function M.onUpdate(self, dt) ... end
    //     return M
    // An INSTANCE binds a module to one entity: the instance owns
    // a state table (`self` in the callbacks) that carries the
    // entity's name in __name and whatever fields the script
    // stores on it — per-entity script memory with no C++ schema.
    // installEntityApi registers the engine API v1 (an `entity`
    // table of functions taking `self`, plus global log() and
    // time()) bound to one Game; entities are addressed by name.
    void installEntityApi(Game& game);
    bool loadModule(const std::string& path, int& moduleId);
    bool loadModuleFromString(const std::string& code,
                              const std::string& chunkName, int& moduleId);
    bool createInstance(int moduleId, const std::string& entityName,
                        int& instanceId);
    // Calls the instance's onUpdate(self, dt) when it defines one.
    // A script error logs and returns false; the engine survives.
    bool callUpdate(int instanceId, float dt);

private:
    struct Instance
    {
        int moduleRef = 0; // Lua registry refs
        int stateRef = 0;
        std::string entityName;
    };
    bool loadModuleFromStack(int& moduleId); // chunk already loaded
    lua_State* m_state = nullptr;
    std::vector<int> m_moduleRefs;
    std::vector<Instance> m_instances;
};
